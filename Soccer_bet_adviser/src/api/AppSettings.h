#pragma once
// Persistent user settings. API keys are NEVER stored in source code: they come from
// environment variables (preferred) or from the Settings page (stored per user via QSettings).
#include <QString>
#include "core/ReasoningEngine.h"

struct AppSettings {
    QString footballDataKey, oddsApiKey;
    QString competitionCode = "PL";          // football-data.org competition
    QString oddsSportKey = "soccer_epl";     // The Odds API sport key
    QString oddsRegions = "uk,eu";
    int liveRefreshSec = 60;                 // only while a match is live and auto-refresh is on
    int fixturesRefreshMin = 15;
    int oddsRefreshMin = 30;
    bool autoRefresh = true;
    double rfWeight = 0.5;
    sba::RiskConfig risk;

    static AppSettings load();
    void save() const;
    QString effectiveFootballKey() const;    // env var overrides stored value
    QString effectiveOddsKey() const;
    static QString mask(const QString& key);
};
