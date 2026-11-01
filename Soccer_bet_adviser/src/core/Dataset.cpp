#include "Dataset.h"
#include "Csv.h"
#include "DateUtil.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>

namespace sba {

static std::string trim(std::string s) {
    const char* ws = " \t\r\n";
    s.erase(0, s.find_first_not_of(ws) == std::string::npos ? s.size() : s.find_first_not_of(ws));
    const size_t e = s.find_last_not_of(ws);
    s.erase(e == std::string::npos ? 0 : e + 1);
    return s;
}

bool Dataset::load(const std::string& csvPath) {
    m_matches.clear(); m_teams.clear(); m_seasonStart.clear(); m_dropped = 0; m_error.clear();

    CsvTable t;
    if (!loadCsv(csvPath, t, m_error)) return false;

    const int cDate = t.col("Date"), cHome = t.col("HomeTeam"), cAway = t.col("AwayTeam"),
              cHg = t.col("FTHG"), cAg = t.col("FTAG"), cRes = t.col("FTR");
    if (cDate < 0 || cHome < 0 || cAway < 0 || cHg < 0 || cAg < 0 || cRes < 0) {
        m_error = "Dataset is missing one of: Date, HomeTeam, AwayTeam, FTHG, FTAG, FTR";
        return false;
    }
    const int cSeason = t.col("SeasonStartYear");
    const int cTime = t.col("Time");
    const char* statCols[2][3] = {{"HS", "HST", "HC"}, {"AS", "AST", "AC"}};
    int cStat[2][3];
    for (int s = 0; s < 2; ++s) for (int k = 0; k < 3; ++k) cStat[s][k] = t.col(statCols[s][k]);
    const int cOdds[3] = {t.col("B365H"), t.col("B365D"), t.col("B365A")};

    auto at = [&](const std::vector<std::string>& r, int c) -> std::string {
        return (c >= 0 && static_cast<size_t>(c) < r.size()) ? r[c] : std::string();
    };

    std::vector<HistMatch> rows;
    for (const auto& r : t.rows) {
        HistMatch m;
        if (!parseIsoDate(at(r, cDate), m.date)) { ++m_dropped; continue; }
        m.home = trim(at(r, cHome)); m.away = trim(at(r, cAway));
        const double hg = parseDouble(at(r, cHg)), ag = parseDouble(at(r, cAg));
        std::string res = trim(at(r, cRes));
        for (char& ch : res) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (m.home.empty() || m.away.empty() || m.home == m.away || std::isnan(hg) || std::isnan(ag) ||
            hg < 0 || ag < 0 || res.size() != 1 || (res[0] != 'H' && res[0] != 'D' && res[0] != 'A')) {
            ++m_dropped; continue;
        }
        m.fthg = static_cast<int>(hg); m.ftag = static_cast<int>(ag); m.ftr = res[0];
        const char actual = m.fthg > m.ftag ? 'H' : (m.fthg < m.ftag ? 'A' : 'D');
        if (actual != m.ftr) { ++m_dropped; continue; }  // inconsistent result

        {
            const std::string ts = trim(at(r, cTime));
            int hh = 0, mm = 0;
            if (ts.size() >= 4 && std::sscanf(ts.c_str(), "%d:%d", &hh, &mm) == 2 && hh >= 0 && hh < 24 && mm >= 0 && mm < 60) m.timeMin = hh * 60 + mm;
        }
        const double sy = cSeason >= 0 ? parseDouble(at(r, cSeason)) : kNaN;
        m.season = std::isnan(sy) ? seasonStartYearFor(m.date) : static_cast<int>(sy);
        for (int s = 0; s < 2; ++s) for (int k = 0; k < 3; ++k) m.stat[s][k] = parseDouble(at(r, cStat[s][k]));
        for (int k = 0; k < 3; ++k) m.odds[k] = parseDouble(at(r, cOdds[k]));
        rows.push_back(std::move(m));
    }

    // Stable sort by date, then drop duplicate (date, home, away) keeping the last one.
    std::stable_sort(rows.begin(), rows.end(), [](const HistMatch& a, const HistMatch& b) { return a.date < b.date; });
    std::unordered_map<std::string, size_t> lastIdx;
    for (size_t i = 0; i < rows.size(); ++i)
        lastIdx[std::to_string(rows[i].date) + "|" + rows[i].home + "|" + rows[i].away] = i;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (lastIdx[std::to_string(rows[i].date) + "|" + rows[i].home + "|" + rows[i].away] == i) m_matches.push_back(rows[i]);
        else ++m_dropped;
    }
    if (m_matches.size() < 300) { m_error = "Too few valid matches in dataset (" + std::to_string(m_matches.size()) + ")"; return false; }

    std::set<std::string> names;
    for (const auto& m : m_matches) {
        names.insert(m.home); names.insert(m.away);
        auto it = m_seasonStart.find(m.season);
        if (it == m_seasonStart.end() || m.date < it->second) m_seasonStart[m.season] = m.date;
    }
    m_teams.assign(names.begin(), names.end());
    return true;
}

int Dataset::seasonStart(int season, int fallback) const {
    auto it = m_seasonStart.find(season);
    return it == m_seasonStart.end() ? fallback : it->second;
}

int Dataset::testStartDate() const {
    std::vector<int> dates;
    for (const auto& m : m_matches) if (dates.empty() || dates.back() != m.date) dates.push_back(m.date);
    if (dates.empty()) return 0;
    return dates[static_cast<size_t>(std::floor(dates.size() * (1.0 - 0.15)))];
}

}  // namespace sba
