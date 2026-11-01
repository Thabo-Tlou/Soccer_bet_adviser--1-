#pragma once
// Owns the long-lived services and exposes asynchronous operations to the pages.
#include <QObject>
#include <functional>
#include <memory>
#include "api/AppSettings.h"
#include "api/LiveDataService.h"
#include "core/PredictionService.h"
#include "core/PredictionStore.h"
#include "core/TfIdfIndex.h"

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject* parent = nullptr);

    void start();                                   // loads data/models asynchronously
    bool ready() const { return m_ready; }
    QString initError() const { return m_initError; }

    sba::PredictionService& service() { return m_service; }
    sba::PredictionStore& store() { return m_store; }
    LiveDataService* live() { return m_live; }
    const AppSettings& settings() const { return m_settings; }
    void applySettings(const AppSettings& s);

    void analyse(const sba::AnalysisRequest& req, std::function<void(bool, const sba::Analysis&, const QString&)> done);
    void runBacktest();
    bool backtestRunning() const { return m_backtestRunning; }
    const sba::BacktestReport& backtest() const { return m_backtest; }

    void rebuildIndex();
    bool indexBuilding() const { return m_indexBuilding; }
    std::shared_ptr<sba::TfIdfIndex> index() const { return m_index; }
    QString indexRoot() const { return m_indexRoot; }
    QString dbPath() const { return m_dbPath; }
    QString resourcesRoot() const { return QString::fromStdString(m_paths.root); }
    void notifyPredictionsChanged() { emit predictionsChanged(); }

signals:
    void initialised(bool ok);
    void backtestStarted();
    void backtestFinished();
    void indexChanged();
    void settingsChanged();
    void predictionsChanged();

private:
    sba::PredictionService m_service;
    sba::PredictionStore m_store;
    LiveDataService* m_live = nullptr;
    AppSettings m_settings;
    sba::ResourcePaths m_paths;
    sba::BacktestReport m_backtest;
    std::shared_ptr<sba::TfIdfIndex> m_index = std::make_shared<sba::TfIdfIndex>();
    QString m_indexRoot, m_dbPath, m_initError;
    bool m_ready = false, m_backtestRunning = false, m_indexBuilding = false;
};
