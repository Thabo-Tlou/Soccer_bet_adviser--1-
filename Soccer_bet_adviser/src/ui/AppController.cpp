#include "AppController.h"
#include "Async.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

AppController::AppController(QObject* parent) : QObject(parent) {
    m_settings = AppSettings::load();
    m_live = new LiveDataService(&m_store, this);
    connect(m_live, &LiveDataService::fixturesChanged, this, &AppController::predictionsChanged);
}

void AppController::start() {
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + "/soccer_bet_adviser.db";
    if (!m_store.open(m_dbPath.toStdString()))
        m_initError = "Prediction history database could not be opened: " + QString::fromStdString(m_store.error());

    // Project folder searched by the knowledge base: the source tree if present, else the installed docs.
    const QString src = QString::fromUtf8(SBA_SOURCE_DIR);
    m_indexRoot = QDir(src).exists() ? src : QCoreApplication::applicationDirPath() + "/docs";

    const std::string exeDir = QCoreApplication::applicationDirPath().toStdString();
    runAsync(this, [this, exeDir]() {
        m_paths = sba::locateResources(exeDir, SBA_SOURCE_DIR);
        return m_service.initialise(m_paths);
    }, [this](bool ok) {
        m_ready = ok;
        if (ok) {
            m_service.setRfWeight(m_settings.rfWeight);
            m_service.setRiskConfig(m_settings.risk);
            m_live->setSettings(m_settings);
            m_live->setDatasetTeams(m_service.dataset().teams());
            m_live->start();
            runBacktest();
        } else {
            m_live->setSettings(m_settings);
            const auto& msgs = m_service.status().messages;
            m_initError = msgs.empty() ? "Initialisation failed" : QString::fromStdString(msgs.front());
        }
        emit initialised(ok);
        rebuildIndex();
    });
}

void AppController::applySettings(const AppSettings& s) {
    m_settings = s;
    m_settings.save();
    m_service.setRfWeight(s.rfWeight);
    m_service.setRiskConfig(s.risk);
    m_live->setSettings(s);
    emit settingsChanged();
}

void AppController::analyse(const sba::AnalysisRequest& req, std::function<void(bool, const sba::Analysis&, const QString&)> done) {
    if (!m_ready) { done(false, {}, "Models are not loaded yet."); return; }
    struct Out { bool ok; sba::Analysis a; QString err; };
    runAsync(this, [this, req]() {
        Out o; std::string err;
        o.ok = m_service.analyse(req, o.a, err);
        o.err = QString::fromStdString(err);
        return o;
    }, [done](const Out& o) { done(o.ok, o.a, o.err); });
}

void AppController::runBacktest() {
    if (!m_ready || m_backtestRunning) return;
    m_backtestRunning = true;
    emit backtestStarted();
    runAsync(this, [this]() { return m_service.runBacktest(); }, [this](const sba::BacktestReport& r) {
        m_backtestRunning = false;
        m_backtest = r;
        if (r.ok) m_service.setModelQuality(r.quality);
        emit backtestFinished();
    });
}

void AppController::rebuildIndex() {
    if (m_indexBuilding) return;
    m_indexBuilding = true;
    const std::string root = m_indexRoot.toStdString();
    runAsync(this, [root]() {
        auto idx = std::make_shared<sba::TfIdfIndex>();
        idx->build(root);
        return idx;
    }, [this](std::shared_ptr<sba::TfIdfIndex> idx) {
        m_index = idx;
        m_indexBuilding = false;
        emit indexChanged();
    });
}
