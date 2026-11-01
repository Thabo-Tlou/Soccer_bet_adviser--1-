# Soccer Bet Adviser

A desktop application (C++17 / Qt 6) that estimates home / draw / away probabilities for English Premier League matches with a
Random Forest (ONNX Runtime) and a Poisson goals model, compares them with bookmaker odds, and explains - or withholds - a
"possible value" assessment using explicit rules. It tracks its own predictions so you can see how good they really are.

> **Honest status.** On the held-out test window the models are about as accurate as bookmaker odds, not better
> (blend log loss 1.0116 vs bookmaker 1.0099, 522 matches). The app says so on screen. Nothing here guarantees winnings.
> Estimates only - not financial advice.

## Two sides, one app
The app opens on the **user side** (livescore-style: black and white, green for live state and highlights). The **Admin** button
(top right) switches to the admin panel; **User view** (top right of the admin panel) switches back.

| User screen | What it does |
|---|---|
| Scores | 7-day date strip, All / Live / Finished / Upcoming / Favourites, search, matches grouped by competition, the model's pick (1 / X / 2 and %) and a VALUE tag on upcoming rows, star to follow a match |
| Match | scoreboard header; Prediction (probabilities vs market, verdict, reasons, save), Odds (bookmaker comparison), H2H (form + previous meetings), Stats (finished matches) |
| Predictions | next 14 days of fixtures with the model's pick, "Value only" filter, and a form to predict any two teams (optional odds) |
| Table | league table for any season in the data, with form |
| My picks | saved predictions, marked correct / missed once the result is known |

Without API keys the user side shows recorded results from the dataset (up to its last date); add keys in the admin Settings for live scores, fixtures and odds.
Screenshots: `docs/screenshots/user_*.png` (they include demo fixtures). Dev aid: `SoccerBetAdviser --user-screenshots <folder>`.

## Build and run (Windows, Visual Studio 2022 + Qt 6)

Prerequisites: Qt 6 (MSVC 2022 64-bit kit, with Widgets/Network/Concurrent), Visual Studio 2022 Build Tools, CMake >= 3.21.

```bat
build_windows.bat C:\Qt\6.8.0\msvc2022_64
```
or manually:
```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2022_64"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target deploy      # copies Qt DLLs next to the exe (windeployqt)
build\Release\SoccerBetAdviser.exe
```
**Qt Creator:** open `CMakeLists.txt`, pick your MSVC kit, build, run.

The first configure downloads (needs internet): ONNX Runtime 1.20.1, nlohmann/json, and the SQLite amalgamation.
To use a local ONNX Runtime instead: `-DONNXRUNTIME_ROOT=C:/path/onnxruntime-win-x64-1.20.1`.
Without ONNX Runtime the app still runs on the Poisson model and says so.

Linux/macOS also work (`cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt>`); use `-DSBA_BUILD_UI=OFF` to build only the core + tests without Qt.

## First run
1. Start the app. The **Analysis** page works immediately: pick two teams, a date and (optionally) decimal odds.
2. For live data add free API keys in **Settings** (or set `FOOTBALL_DATA_API_KEY` / `ODDS_API_KEY`):
   football-data.org (fixtures/scores) and The Odds API (bookmaker odds). Check their free-tier limits and terms.
3. **Save** predictions from the Analysis page; **Performance** scores them against real results (button: *Update results from API*).

## Screens
| Page | Purpose |
|---|---|
| Dashboard | model/data/API health, upcoming fixtures with model bars |
| Live & upcoming | filters, odds comparison across bookmakers, data-age and stale-data warnings, auto/manual refresh |
| Match analysis | probabilities vs market, edge/EV, model comparison, reasoning, checks, save prediction |
| Performance | held-out backtest, calibration, confusion matrix, hypothetical betting, prospective tracking |
| Knowledge search | TF-IDF search of source/docs with file and line ranges (try "Where is ONNX initialized?") |
| Settings | API keys, refresh intervals, reasoning thresholds, RF/Poisson blend |

## Tests (what is verified)
`ctest` runs 100+ checks: C++ features identical to the Python pipeline; ONNX Runtime and Poisson outputs match Python
(max gap 3e-6); reasoning rules (value / insufficient evidence / no recommendation / stale odds / started match / bad data);
SQLite persistence, SQL-injection-safe storage, corrupt-file handling; TF-IDF search and exclusions; API parsers against
malformed responses; secret redaction.
**Not covered by automated tests:** live HTTP calls (need your keys), the Windows installer/deployment, long-running UI soak tests.
Verify on a clean Windows machine without Qt Creator after running the `deploy` target.

## Retraining
```
cd ml_training
python train_model.py            # retrain + export ONNX
python export_cpp_assets.py      # export Poisson JSON + parity fixtures, then rebuild
```
See `docs/ARCHITECTURE.md` and `LICENSE-NOTICES.md`.
