#pragma once
#include <map>
#include <string>
#include <vector>
#include "Types.h"

namespace sba {

// One finished historical match, cleaned exactly like train_model.py does.
struct HistMatch {
    int date = 0;                 // days since epoch
    std::string home, away;
    int fthg = 0, ftag = 0;
    char ftr = 'D';               // H / D / A
    int season = 0;               // season start year
    int timeMin = -1;             // kickoff time as minutes after midnight (UK local), -1 = unknown
    double stat[2][3] = {{kNaN, kNaN, kNaN}, {kNaN, kNaN, kNaN}};  // [home|away][shots, on target, corners]
    double odds[3] = {kNaN, kNaN, kNaN};                            // Bet365 H/D/A (decimal)
    int target() const { return ftr == 'H' ? 0 : (ftr == 'D' ? 1 : 2); }
};

class Dataset {
public:
    // Loads and cleans the CSV. On failure returns false and sets error().
    bool load(const std::string& csvPath);

    const std::vector<HistMatch>& matches() const { return m_matches; }
    const std::vector<std::string>& teams() const { return m_teams; }   // sorted
    int seasonStart(int season, int fallback) const;
    int firstDate() const { return m_matches.empty() ? 0 : m_matches.front().date; }
    int lastDate() const { return m_matches.empty() ? 0 : m_matches.back().date; }
    int droppedRows() const { return m_dropped; }
    const std::string& error() const { return m_error; }

    // Date index where the chronological 15 % test window starts (same rule as training).
    int testStartDate() const;

private:
    std::vector<HistMatch> m_matches;
    std::vector<std::string> m_teams;
    std::map<int, int> m_seasonStart;
    int m_dropped = 0;
    std::string m_error;
};

}  // namespace sba
