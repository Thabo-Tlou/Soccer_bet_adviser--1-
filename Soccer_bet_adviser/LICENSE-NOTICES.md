# Third-party notices

| Component | Use | License |
|---|---|---|
| Qt 6 (Widgets, Network, Concurrent) | user interface, HTTP | LGPL v3 / GPL / commercial - check your Qt edition, link dynamically and ship the notices |
| ONNX Runtime | Random Forest inference | MIT |
| nlohmann/json | JSON parsing | MIT |
| SQLite | prediction history | Public domain |
| scikit-learn, skl2onnx, pandas, numpy | model training (Python, not shipped) | BSD-3-Clause |

**Data.** `ml_training/data/EPL_merged_2017-2027.csv` is the historical match/odds dataset that was supplied with the
project. Confirm that its source permits your intended use before distributing it.

**APIs.** football-data.org and The Odds API are third-party services with their own terms, quotas and licences.
Check that your intended usage is permitted. The application runs without either (manual odds on the Analysis page).
