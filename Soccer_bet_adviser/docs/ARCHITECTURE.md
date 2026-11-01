# Architecture

```
src/core   Qt-free library (unit-tested): data, features, models, reasoning, metrics, TF-IDF, SQLite
src/api    Qt Network: HTTP client (timeout, cache, rate limits), football-data.org, The Odds API, live-data service
src/ui     Qt Widgets: theme, custom widgets, pages, controller (async work)
tests      core tests (no Qt) and API parser tests
ml_training  Python training, ONNX export and reference-data export
```

## Phase 4 - models
`ml_training/train_model.py` trains the Random Forest (exported to ONNX) and a Poisson goal model.
`ml_training/export_cpp_assets.py` exports the Poisson model to `models/poisson_model.json` and writes reference
features/probabilities (`fixtures/parity_reference.csv`) used by the C++ parity tests.
ONNX Runtime is initialised in `OnnxInference::load()` (`src/core/OnnxInference.cpp`): it creates the session, checks the
input width (58 features) and locates the `probabilities` output. `PredictionService::initialise()` calls it once at start-up.
`FeatureEngineer` is a line-by-line port of the Python feature pipeline; the test `feature_parity_with_python` proves the
two agree on 600 matches x 58 features.

The tuned ensemble in `epl_decision_config.json` gives the Random Forest weight 0.0 (the Poisson model validated slightly
better), so the original Python probabilities were Poisson-only. The app blends both (default 0.5, editable in Settings);
on the held-out window RF 1.0149, Poisson 1.0159 and blend 1.0116 log loss are all very close to each other.

## Phase 5 - data
`HttpClient` adds a 12 s timeout, an on-disk response cache (keys never enter cache file names), a stale-cache fallback
(flagged to the UI), per-host request spacing and HTTP 429 back-off. `LiveDataService` merges fixtures and odds, maps provider
team names to dataset names (`TeamMapper`) and reports Fresh / Stale / Error / Not configured per source.
Credentials: environment variables `FOOTBALL_DATA_API_KEY`, `ODDS_API_KEY`, or the Settings page. Never in source.

## Phase 6 - reasoning engine (`ReasoningEngine`)
1. **Validate**: identity, timing (match not started), probability validity, input completeness, history depth, odds freshness.
2. **Probabilities**: RF, Poisson and their blend.
3. **Market**: implied probabilities with the margin removed (proportional method).
4. **Uncertainty**: RF/Poisson disagreement, completeness, history, held-out quality versus the bookmaker.
5. **Decide** (all thresholds in `RiskConfig` / Settings):
   - *Possible value* only if EV >= 5 %, edge >= 3 pts, a market-shrunk EV is still positive, and no reliability warning applies.
   - *Insufficient evidence* if an edge exists but data/agreement/freshness checks fail, or evidence is invalid.
   - *No recommendation* if nothing clears the thresholds.
6. **Explain**: supporting factors, counter-arguments, reasons for withholding and every check, built from the live numbers.

## Phase 7 - knowledge search
`TfIdfIndex` chunks source and docs (40 lines, 10 overlap), tokenises identifiers (camelCase / snake_case), applies light
stemming, sublinear TF-IDF with cosine similarity, a concept-coverage boost and small synonym expansion. It excludes build
folders, `.git`, virtual environments, binaries, secrets and key files, masks key-like strings, and rebuilds when files change.

## Phase 8 - evaluation
`PredictionService::runBacktest()` evaluates RF, Poisson, blend, Elo, base-rate and bookmaker probabilities on the final 15 %
of matches by date (the models were fitted on earlier matches only): accuracy, log loss, Brier score, confusion matrices,
calibration/ECE, and flat-stake hypothetical betting with a standard error. `PredictionStore` keeps the original prediction
and timestamp (never overwritten), settles results from the API and scores prospective performance.

## Phase 10 - testing
`ctest` runs `sba_tests` (core) and `sba_api_tests`. See README for what is and is not covered.
