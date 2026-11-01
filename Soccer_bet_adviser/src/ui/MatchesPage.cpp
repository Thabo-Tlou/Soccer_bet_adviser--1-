#include "MatchesPage.h"
#include "Formatting.h"
#include "Theme.h"
#include <QHeaderView>
#include <QSplitter>
#include <cmath>

static QTableWidgetItem* cell(const QString& t, Qt::Alignment a = Qt::AlignLeft | Qt::AlignVCenter, const QColor& fg = QColor()) {
    auto* i = new QTableWidgetItem(t);
    i->setTextAlignment(a);
    if (fg.isValid()) i->setForeground(fg);
    return i;
}

MatchesPage::MatchesPage(AppController* c, QWidget* parent) : QWidget(parent), m_c(c) {
    setObjectName("Page");
    auto* v = new QVBoxLayout(this); v->setContentsMargins(28, 24, 28, 24); v->setSpacing(14);
    v->addWidget(makeLabel("Live & upcoming matches", "h1"));
    v->addWidget(makeLabel("Scores, fixtures and bookmaker odds from your configured providers. Data is always labelled with its source and age.", "muted"));

    m_banner = new Banner;
    v->addWidget(m_banner);

    auto* bar = new Card;
    auto* br = new QHBoxLayout; br->setSpacing(10);
    m_statusFilter = new QComboBox; m_statusFilter->addItems({"All statuses", "Live", "Upcoming", "Finished"});
    m_teamFilter = new QLineEdit; m_teamFilter->setPlaceholderText("Filter by team or competition..."); m_teamFilter->setClearButtonEnabled(true);
    m_dateOn = new QCheckBox("Date range");
    m_from = new QDateEdit(QDate::currentDate().addDays(-3)); m_to = new QDateEdit(QDate::currentDate().addDays(14));
    for (auto* d : {m_from, m_to}) { d->setCalendarPopup(true); d->setDisplayFormat("dd MMM yyyy"); d->setEnabled(false); }
    m_auto = new QCheckBox("Auto-refresh"); m_auto->setChecked(c->settings().autoRefresh);
    m_updated = makeLabel("", "dim");
    auto* refresh = makeButton("Refresh now", true);
    br->addWidget(m_statusFilter); br->addWidget(m_teamFilter, 1); br->addWidget(m_dateOn); br->addWidget(m_from); br->addWidget(m_to);
    br->addSpacing(8); br->addWidget(m_auto); br->addWidget(refresh);
    bar->body()->addLayout(br);
    bar->body()->addWidget(m_updated);
    v->addWidget(bar);

    auto* split = new QSplitter(Qt::Horizontal);
    split->setChildrenCollapsible(false);
    m_table = new QTableWidget(0, 10);
    m_table->setHorizontalHeaderLabels({"Status", "Kickoff", "Competition", "Home", "Score", "Away", "1", "X", "2", "Data age"});
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(false); m_table->setAlternatingRowColors(true);
    auto* h = m_table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::ResizeToContents);
    h->setSectionResizeMode(3, QHeaderView::Stretch); h->setSectionResizeMode(5, QHeaderView::Stretch);
    split->addWidget(m_table);

    auto* detail = new Card("Match detail");
    m_detailTitle = makeLabel("Select a match", "h2"); m_detailTitle->setWordWrap(true);
    detail->body()->addWidget(m_detailTitle);
    detail->body()->addWidget(makeLabel("ODDS COMPARISON", "caption"));
    m_books = new QTableWidget(0, 5);
    m_books->setHorizontalHeaderLabels({"Bookmaker", "1", "X", "2", "Updated"});
    m_books->verticalHeader()->hide(); m_books->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_books->setShowGrid(false); m_books->setSelectionMode(QAbstractItemView::NoSelection);
    m_books->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_books->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    detail->body()->addWidget(m_books, 1);
    m_detailNote = makeLabel("", "dim"); m_detailNote->setWordWrap(true);
    detail->body()->addWidget(m_detailNote);
    m_analyse = makeButton("Analyse this match", true);
    m_analyse->setEnabled(false);
    detail->body()->addWidget(m_analyse);
    detail->setMinimumWidth(360);
    split->addWidget(detail);
    split->setStretchFactor(0, 3); split->setStretchFactor(1, 1);
    v->addWidget(split, 1);

    auto rebuildSlot = [this]() { rebuild(); };
    connect(m_statusFilter, &QComboBox::currentIndexChanged, this, rebuildSlot);
    connect(m_teamFilter, &QLineEdit::textChanged, this, rebuildSlot);
    connect(m_dateOn, &QCheckBox::toggled, this, [this](bool on) { m_from->setEnabled(on); m_to->setEnabled(on); rebuild(); });
    connect(m_from, &QDateEdit::dateChanged, this, rebuildSlot);
    connect(m_to, &QDateEdit::dateChanged, this, rebuildSlot);
    connect(m_auto, &QCheckBox::toggled, this, [this](bool on) { auto s = m_c->settings(); s.autoRefresh = on; m_c->applySettings(s); });
    connect(refresh, &QPushButton::clicked, this, [this]() { m_c->live()->refreshAll(true); });
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() { showDetail(); });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int r, int) { if (r >= 0 && r < static_cast<int>(m_rows.size())) emit openAnalysis(m_rows[r]); });
    connect(m_analyse, &QPushButton::clicked, this, [this]() { const int r = m_table->currentRow(); if (r >= 0 && r < static_cast<int>(m_rows.size())) emit openAnalysis(m_rows[r]); });
    connect(c->live(), &LiveDataService::fixturesChanged, this, [this]() { rebuild(); });
    connect(c->live(), &LiveDataService::statusChanged, this, [this]() { updateStatus(); });
    connect(c, &AppController::settingsChanged, this, [this]() { m_auto->setChecked(m_c->settings().autoRefresh); });
    rebuild();
    updateStatus();
}

void MatchesPage::updateStatus() {
    const auto fx = m_c->live()->fixturesStatus(), od = m_c->live()->oddsStatus();
    const int maxAge = std::max(5, m_c->settings().fixturesRefreshMin * 3);
    QString line = QString("Fixtures: %1 (%2)   |   Odds: %3 (%4)")
        .arg(fx.name, fx.lastEpoch ? "updated " + Fmt::ago(fx.lastEpoch) : "no data yet", od.name, od.lastEpoch ? "updated " + Fmt::ago(od.lastEpoch) : "no data yet");
    if (fx.remaining >= 0) line += QString("   |   requests left this minute: %1").arg(fx.remaining);
    m_updated->setText(line);

    if (fx.state == SourceStatus::NotConfigured)
        m_banner->show(Banner::Info, "No football-data.org API key is configured, so fixtures cannot be refreshed. Add a free key in Settings (or set FOOTBALL_DATA_API_KEY). "
                                     + QString(m_rows.empty() ? "" : "Showing previously cached fixtures (last update " + Fmt::ago(fx.lastEpoch) + "). ")
                                     + "The Analysis page still works without any API.");
    else if (fx.state == SourceStatus::Error)
        m_banner->show(Banner::Error, "Fixtures unavailable: " + fx.message + (m_rows.empty() ? "" : "  Showing the last cached data - it may be out of date."));
    else if (fx.state == SourceStatus::Stale || m_c->live()->isStale(fx, maxAge))
        m_banner->show(Banner::Warning, "Fixture data is stale (last update " + Fmt::ago(fx.lastEpoch) + "). " + fx.message);
    else if (od.state == SourceStatus::Error)
        m_banner->show(Banner::Warning, "Odds unavailable: " + od.message + "  Rows without odds show a dash - old odds are never presented as current.");
    else if (od.state == SourceStatus::NotConfigured)
        m_banner->show(Banner::Info, "Odds provider not configured. You can still analyse any match with manually entered odds.");
    else m_banner->show(Banner::Success, "Data is fresh. Free-tier providers may delay live scores - check the data age column.");
}

void MatchesPage::rebuild() {
    m_rows.clear();
    const QString q = m_teamFilter->text().trimmed().toLower();
    const int sf = m_statusFilter->currentIndex();
    const qint64 from = m_from->date().startOfDay().toSecsSinceEpoch();
    const qint64 to = m_to->date().addDays(1).startOfDay().toSecsSinceEpoch();
    for (const auto& f : m_c->live()->fixtures()) {
        if (sf == 1 && !f.isLive()) continue;
        if (sf == 2 && !f.isUpcoming()) continue;
        if (sf == 3 && !f.isFinished()) continue;
        if (m_dateOn->isChecked() && (f.kickoffEpoch < from || f.kickoffEpoch >= to)) continue;
        if (!q.isEmpty()) {
            const QString hay = QString::fromStdString(f.home + " " + f.away + " " + f.competition).toLower();
            if (!hay.contains(q)) continue;
        }
        m_rows.push_back(f);
    }
    std::sort(m_rows.begin(), m_rows.end(), [](const auto& a, const auto& b) {
        const int ra = a.isLive() ? 0 : (a.isUpcoming() ? 1 : 2), rb = b.isLive() ? 0 : (b.isUpcoming() ? 1 : 2);
        if (ra != rb) return ra < rb;
        return ra == 2 ? a.kickoffEpoch > b.kickoffEpoch : a.kickoffEpoch < b.kickoffEpoch;
    });
    m_table->setRowCount(static_cast<int>(m_rows.size()));
    const int maxAge = std::max(5, m_c->settings().fixturesRefreshMin * 3);
    for (int r = 0; r < static_cast<int>(m_rows.size()); ++r) {
        const auto& f = m_rows[r];
        QString st = QString::fromStdString(f.status); QColor stc = Theme::muted;
        if (f.isLive()) { st = f.minute >= 0 ? QString("LIVE %1'").arg(f.minute) : "LIVE"; stc = Theme::primary; }
        else if (f.isUpcoming()) st = "Upcoming";
        else if (f.isFinished()) st = "Full time";
        else if (st == "POSTPONED" || st == "CANCELLED" || st == "SUSPENDED") stc = Theme::danger;
        m_table->setItem(r, 0, cell(st, Qt::AlignLeft | Qt::AlignVCenter, stc));
        m_table->setItem(r, 1, cell(Fmt::kickoff(f.kickoffEpoch)));
        m_table->setItem(r, 2, cell(QString::fromStdString(f.competition), Qt::AlignLeft | Qt::AlignVCenter, Theme::muted));
        m_table->setItem(r, 3, cell(QString::fromStdString(f.homeDataset.empty() ? f.home : f.homeDataset), Qt::AlignRight | Qt::AlignVCenter));
        m_table->item(r, 3)->setToolTip(QString::fromStdString(f.home));
        const bool hasScore = f.homeGoals >= 0 && f.awayGoals >= 0;
        m_table->setItem(r, 4, cell(hasScore ? QString("%1 - %2").arg(f.homeGoals).arg(f.awayGoals) : "vs", Qt::AlignCenter, hasScore ? Theme::text : Theme::dim));
        m_table->setItem(r, 5, cell(QString::fromStdString(f.awayDataset.empty() ? f.away : f.awayDataset)));
        m_table->item(r, 5)->setToolTip(QString::fromStdString(f.away));
        double best[3]; std::string names[3];
        const bool have = f.bestOdds(best, names);
        const bool oddsStale = have && (f.oddsFetchedEpoch <= 0 || QDateTime::currentSecsSinceEpoch() - f.oddsFetchedEpoch > m_c->settings().risk.maxOddsAgeMinutes * 60LL);
        for (int k = 0; k < 3; ++k) {
            auto* it = cell(have ? Fmt::num(best[k]) : "-", Qt::AlignCenter, oddsStale ? Theme::warning : Theme::text);
            if (have) it->setToolTip(QString("Best price: %1%2").arg(QString::fromStdString(names[k]), oddsStale ? "\nSTALE - refresh before relying on it" : ""));
            m_table->setItem(r, 6 + k, it);
        }
        const bool old = f.updatedEpoch > 0 && QDateTime::currentSecsSinceEpoch() - f.updatedEpoch > maxAge * 60LL;
        m_table->setItem(r, 9, cell(Fmt::ago(f.updatedEpoch) + (old ? "  (stale)" : ""), Qt::AlignLeft | Qt::AlignVCenter, old ? Theme::warning : Theme::dim));
        m_table->setRowHeight(r, 38);
    }
    if (m_rows.empty()) m_table->setRowCount(0);
    showDetail();
    updateStatus();
}

void MatchesPage::showDetail() {
    const int r = m_table->currentRow();
    m_books->setRowCount(0);
    if (r < 0 || r >= static_cast<int>(m_rows.size())) { m_detailTitle->setText("Select a match"); m_detailNote->setText(""); m_analyse->setEnabled(false); return; }
    const auto& f = m_rows[r];
    m_detailTitle->setText(QString::fromStdString(f.home + " vs " + f.away));
    m_analyse->setEnabled(!f.homeDataset.empty() && !f.awayDataset.empty() && m_c->ready());
    QString note;
    if (f.homeDataset.empty() || f.awayDataset.empty()) note = "One of these teams is not in the historical dataset, so no model analysis is available.";
    if (f.books.empty()) {
        note += (note.isEmpty() ? "" : "\n") + QString("No bookmaker odds for this fixture. They appear when The Odds API covers the match; you can also type odds on the Analysis page.");
        m_detailNote->setText(note); return;
    }
    double best[3]; f.bestOdds(best);
    m_books->setRowCount(static_cast<int>(f.books.size()));
    for (int i = 0; i < static_cast<int>(f.books.size()); ++i) {
        const auto& b = f.books[i];
        m_books->setItem(i, 0, cell(QString::fromStdString(b.bookmaker)));
        for (int k = 0; k < 3; ++k) {
            const bool isBest = std::fabs(b.odds[k] - best[k]) < 1e-9;
            m_books->setItem(i, 1 + k, cell(Fmt::num(b.odds[k]), Qt::AlignCenter, isBest ? Theme::primary : Theme::text));
            if (isBest) { auto f2 = m_books->item(i, 1 + k)->font(); f2.setBold(true); m_books->item(i, 1 + k)->setFont(f2); }
        }
        m_books->setItem(i, 4, cell(Fmt::ago(b.updatedEpoch), Qt::AlignLeft | Qt::AlignVCenter, Theme::dim));
        m_books->setRowHeight(i, 30);
    }
    note += QString("Odds fetched %1. Highlighted prices are the best available for each outcome.").arg(Fmt::ago(f.oddsFetchedEpoch));
    m_detailNote->setText(note);
}
