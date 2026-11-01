#include "LiveDataService.h"
#include <QDate>
#include <QDateTime>
#include <algorithm>
#include <cmath>
#include "core/TeamMapper.h"

LiveDataService::LiveDataService(sba::PredictionStore* store, QObject* parent) : QObject(parent), m_store(store) {
    m_fixStatus.name = "football-data.org";
    m_oddsStatus.name = "The Odds API";
    connect(&m_fixTimer, &QTimer::timeout, this, [this]() { if (m_settings.autoRefresh) refreshFixtures(false); });
    connect(&m_oddsTimer, &QTimer::timeout, this, [this]() { if (m_settings.autoRefresh) refreshOdds(false); });
    connect(&m_liveTimer, &QTimer::timeout, this, [this]() {
        if (!m_settings.autoRefresh) return;
        const bool anyLive = std::any_of(m_fixtures.begin(), m_fixtures.end(), [](const sba::Fixture& f) { return f.isLive(); });
        if (anyLive) refreshFixtures(true);
    });
}

void LiveDataService::setSettings(const AppSettings& s) {
    m_settings = s;
    m_fixTimer.setInterval(std::max(1, s.fixturesRefreshMin) * 60 * 1000);
    m_oddsTimer.setInterval(std::max(5, s.oddsRefreshMin) * 60 * 1000);
    m_liveTimer.setInterval(std::max(30, s.liveRefreshSec) * 1000);
    if (m_settings.effectiveFootballKey().isEmpty()) { m_fixStatus.state = SourceStatus::NotConfigured; m_fixStatus.message = "No API key - add one in Settings"; }
    if (m_settings.effectiveOddsKey().isEmpty()) { m_oddsStatus.state = SourceStatus::NotConfigured; m_oddsStatus.message = "No API key - odds can be entered manually"; }
    emit statusChanged();
}

void LiveDataService::start() {
    if (m_store && m_store->isOpen()) {            // show the last known fixtures immediately, flagged as cached
        m_fixtures = m_store->loadFixtures();
        remap();
        if (!m_fixtures.empty()) {
            qint64 newest = 0; for (const auto& f : m_fixtures) newest = std::max<qint64>(newest, f.updatedEpoch);
            m_fixStatus.lastEpoch = newest; m_fixStatus.state = SourceStatus::Stale; m_fixStatus.message = "Showing cached data until the next refresh";
            emit fixturesChanged();
        }
    }
    m_fixTimer.start(); m_oddsTimer.start(); m_liveTimer.start();
    refreshAll(false);
}

void LiveDataService::refreshAll(bool force) { refreshFixtures(force); refreshOdds(force); }

void LiveDataService::remap() {
    for (auto& f : m_fixtures) {
        f.homeDataset = sba::mapTeamName(f.home, m_teams);
        f.awayDataset = sba::mapTeamName(f.away, m_teams);
    }
}

bool LiveDataService::isStale(const SourceStatus& s, int maxAgeMin) const {
    if (s.lastEpoch <= 0) return true;
    return QDateTime::currentSecsSinceEpoch() - s.lastEpoch > maxAgeMin * 60LL;
}

void LiveDataService::refreshFixtures(bool force) {
    const QString key = m_settings.effectiveFootballKey();
    if (key.isEmpty()) { m_fixStatus.state = SourceStatus::NotConfigured; m_fixStatus.message = "No API key - add one in Settings"; emit statusChanged(); return; }
    m_fixStatus.state = SourceStatus::Loading; emit statusChanged();
    const QDate today = QDate::currentDate();
    m_fd.fetchMatches(key, m_settings.competitionCode, today.addDays(-3).toString(Qt::ISODate), today.addDays(14).toString(Qt::ISODate),
        [this](const FixturesResult& r) {
            if (!r.ok) {
                m_fixStatus.state = SourceStatus::Error; m_fixStatus.message = r.error;
                emit statusChanged(); return;
            }
            m_fixtures = r.fixtures;
            for (auto& f : m_fixtures) f.updatedEpoch = r.fetchedEpoch;
            remap();
            applyOdds();
            m_fixStatus.lastEpoch = r.fetchedEpoch; m_fixStatus.remaining = r.remaining;
            m_fixStatus.state = r.stale ? SourceStatus::Stale : SourceStatus::Fresh;
            m_fixStatus.message = r.stale ? (r.error.isEmpty() ? "Showing cached data" : r.error) : QString("Updated, %1 fixtures").arg(m_fixtures.size());
            emitAndStore();
        }, force);
}

void LiveDataService::refreshOdds(bool force) {
    const QString key = m_settings.effectiveOddsKey();
    if (key.isEmpty()) { m_oddsStatus.state = SourceStatus::NotConfigured; m_oddsStatus.message = "No API key - odds can be entered manually"; emit statusChanged(); return; }
    m_oddsStatus.state = SourceStatus::Loading; emit statusChanged();
    m_odds.fetchOdds(key, m_settings.oddsSportKey, m_settings.oddsRegions, [this](const OddsResult& r) {
        if (!r.ok) { m_oddsStatus.state = SourceStatus::Error; m_oddsStatus.message = r.error; emit statusChanged(); return; }
        m_oddsEvents = r.events;
        m_oddsFetchedEpoch = r.fetchedEpoch;
        m_oddsStatus.lastEpoch = r.fetchedEpoch; m_oddsStatus.remaining = r.remaining;
        m_oddsStatus.state = r.stale ? SourceStatus::Stale : SourceStatus::Fresh;
        m_oddsStatus.message = r.stale ? (r.error.isEmpty() ? "Showing cached odds" : r.error) : QString("Updated, %1 events").arg(m_oddsEvents.size());
        applyOdds();
        emitAndStore();
    }, force);
}

void LiveDataService::applyOdds() {
    for (auto& f : m_fixtures) {
        f.books.clear(); f.oddsFetchedEpoch = 0;
        if (f.homeDataset.empty() || f.awayDataset.empty()) continue;
        for (const auto& e : m_oddsEvents) {
            if (std::llabs(e.commenceEpoch - f.kickoffEpoch) > 36 * 3600) continue;
            if (sba::mapTeamName(e.home, m_teams) == f.homeDataset && sba::mapTeamName(e.away, m_teams) == f.awayDataset) {
                f.books = e.books; f.oddsFetchedEpoch = m_oddsFetchedEpoch; break;
            }
        }
    }
}

void LiveDataService::emitAndStore() {
    if (m_store && m_store->isOpen()) m_store->saveFixtures(m_fixtures);
    emit fixturesChanged();
    emit statusChanged();
}

void LiveDataService::fetchRecentResults(std::function<void(const std::vector<sba::Fixture>&, const QString&)> cb) {
    const QString key = m_settings.effectiveFootballKey();
    if (key.isEmpty()) { cb({}, "No football-data.org API key configured."); return; }
    const QDate today = QDate::currentDate();
    m_fd.fetchMatches(key, m_settings.competitionCode, today.addDays(-10).toString(Qt::ISODate), today.toString(Qt::ISODate),
        [this, cb](const FixturesResult& r) {
            if (!r.ok) { cb({}, r.error); return; }
            auto list = r.fixtures;
            for (auto& f : list) { f.homeDataset = sba::mapTeamName(f.home, m_teams); f.awayDataset = sba::mapTeamName(f.away, m_teams); }
            cb(list, r.stale ? "Results may be outdated (cached response)." : QString());
        }, true);
}

void LiveDataService::testFootballConnection(std::function<void(bool, const QString&)> cb) {
    const QString key = m_settings.effectiveFootballKey();
    const QDate today = QDate::currentDate();
    m_fd.fetchMatches(key, m_settings.competitionCode, today.toString(Qt::ISODate), today.addDays(1).toString(Qt::ISODate),
        [cb](const FixturesResult& r) { cb(r.ok && !r.stale, r.ok ? (r.stale ? "Using cached data: " + r.error : "Connection OK") : r.error); }, true);
}
