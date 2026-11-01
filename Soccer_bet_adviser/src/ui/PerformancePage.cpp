#include "PerformancePage.h"
#include "Formatting.h"
#include "Theme.h"
#include "core/DateUtil.h"
#include <QDateTime>
#include <QHeaderView>
#include <QScrollArea>
#include <QStyle>

static QTableWidget* makeTable(const QStringList& headers) {
    auto* t = new QTableWidget(0, headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->verticalHeader()->hide(); t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setShowGrid(false); t->setSelectionMode(QAbstractItemView::NoSelection); t->setAlternatingRowColors(true);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return t;
}
static QTableWidgetItem* it(const QString& s, Qt::Alignment a = Qt::AlignCenter, const QColor& c = QColor()) {
    auto* i = new QTableWidgetItem(s); i->setTextAlignment(a | Qt::AlignVCenter); if (c.isValid()) i->setForeground(c); return i;
}

PerformancePage::PerformancePage(AppController* c, QWidget* parent) : QWidget(parent), m_c(c) {
    setObjectName("Page");
    auto* outer = new QVBoxLayout(this); outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea; scroll->setWidgetResizable(true); outer->addWidget(scroll);
    auto* page = new QWidget; page->setObjectName("Page"); scroll->setWidget(page);
    auto* v = new QVBoxLayout(page); v->setContentsMargins(28, 24, 28, 24); v->setSpacing(14);

    auto* top = new QHBoxLayout;
    auto* hv = new QVBoxLayout; hv->setSpacing(2);
    hv->addWidget(makeLabel("Performance & history", "h1"));
    hv->addWidget(makeLabel("Held-out backtest on matches the models never saw, plus your own saved predictions scored against real results.", "muted"));
    top->addLayout(hv, 1);
    m_run = makeButton("Re-run backtest", true); top->addWidget(m_run, 0, Qt::AlignTop);
    v->addLayout(top);
    m_banner = new Banner; v->addWidget(m_banner);

    auto* tiles = new QHBoxLayout; tiles->setSpacing(12);
    m_t1 = new StatTile("Held-out matches"); m_t2 = new StatTile("App model log loss");
    m_t3 = new StatTile("Bookmaker log loss"); m_t4 = new StatTile("Calibration error (ECE)");
    for (auto* t : {m_t1, m_t2, m_t3, m_t4}) tiles->addWidget(t, 1);
    v->addLayout(tiles);

    auto* mc = new Card("Probability quality - lower log loss and Brier score are better");
    m_models = makeTable({"Model", "Accuracy", "Log loss", "Brier", "Matches"});
    m_models->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_models->setFixedHeight(260);
    mc->body()->addWidget(m_models);
    mc->body()->addWidget(makeLabel("The bookmaker row is the bar to beat. A model that cannot beat it has no reliable edge, however good its accuracy looks.", "dim"));
    v->addWidget(mc);

    auto* row = new QHBoxLayout; row->setSpacing(12);
    auto* cc = new Card("Calibration (blend)");
    m_cal = new CalibrationChart; cc->body()->addWidget(m_cal);
    cc->body()->addWidget(makeLabel("Points on the dashed line mean predictions came true as often as claimed. Bubble size = number of predictions.", "dim"));
    static_cast<QLabel*>(cc->body()->itemAt(1)->widget())->setWordWrap(true);
    row->addWidget(cc, 1);
    auto* mx = new Card("Confusion matrix");
    auto* sw = new QHBoxLayout;
    m_aware = makeButton("Draw-aware rule"); m_plain = makeButton("Plain argmax");
    sw->addWidget(m_aware); sw->addWidget(m_plain); sw->addStretch();
    mx->body()->addLayout(sw);
    m_conf = new ConfusionWidget; mx->body()->addWidget(m_conf, 1);
    row->addWidget(mx, 1);
    v->addLayout(row);

    auto* bc = new Card("Hypothetical betting on the held-out window (1 unit flat stakes)");
    m_betting = makeTable({"Rule", "Bets", "Win rate", "ROI per unit", "Max drawdown", "Final bankroll (start 100)"});
    m_betting->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_betting->setFixedHeight(120);
    bc->body()->addWidget(m_betting);
    m_bettingNote = makeLabel("", "dim"); m_bettingNote->setWordWrap(true);
    bc->body()->addWidget(m_bettingNote);
    v->addWidget(bc);

    auto* hc = new Card("Prospective tracking - your saved predictions");
    m_settle = makeButton("Update results from API"); hc->headerRow()->addWidget(m_settle);
    m_prospective = makeLabel("", "muted"); m_prospective->setWordWrap(true);
    hc->body()->addWidget(m_prospective);
    m_history = makeTable({"Saved", "Match", "Kickoff", "H / D / A (model)", "Decision", "Odds taken", "Result"});
    m_history->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_history->setMinimumHeight(240);
    hc->body()->addWidget(m_history);
    v->addWidget(hc);
    v->addStretch();

    connect(m_run, &QPushButton::clicked, this, [this]() { m_c->runBacktest(); });
    connect(m_aware, &QPushButton::clicked, this, [this]() { m_showAware = true; showBacktest(); });
    connect(m_plain, &QPushButton::clicked, this, [this]() { m_showAware = false; showBacktest(); });
    connect(m_settle, &QPushButton::clicked, this, [this]() { updateResults(); });
    connect(c, &AppController::backtestStarted, this, [this]() { m_run->setEnabled(false); m_run->setText("Running..."); m_banner->show(Banner::Info, "Running the held-out backtest..."); });
    connect(c, &AppController::backtestFinished, this, [this]() { m_run->setEnabled(true); m_run->setText("Re-run backtest"); showBacktest(); });
    connect(c, &AppController::predictionsChanged, this, [this]() { showHistory(); });
    connect(c, &AppController::initialised, this, [this]() { showBacktest(); showHistory(); });
    showBacktest(); showHistory();
}

void PerformancePage::showBacktest() {
    const auto& b = m_c->backtest();
    m_aware->setProperty("primary", m_showAware); m_plain->setProperty("primary", !m_showAware);
    for (auto* x : {m_aware, m_plain}) { x->style()->unpolish(x); x->style()->polish(x); }
    if (!b.ok) {
        m_banner->show(m_c->ready() ? Banner::Info : Banner::Warning, b.error.empty() ? (m_c->ready() ? "Backtest has not finished yet." : "Models are not loaded.") : QString::fromStdString(b.error));
        m_models->setRowCount(0); m_betting->setRowCount(0); return;
    }
    m_banner->show(Banner::Info, QString("Chronological held-out window %1 to %2 (%3 matches). The models were fitted on earlier matches only. RF weight in the blend: %4.")
                                     .arg(QString::fromStdString(b.from), QString::fromStdString(b.to)).arg(b.matches).arg(b.rfWeight, 0, 'f', 2));
    m_t1->set(QString::number(b.matches), QString::fromStdString(b.from) + " - " + QString::fromStdString(b.to));
    m_t2->set(Fmt::num(b.quality.modelLogLoss, 4), "blend, matches with odds");
    m_t3->set(Fmt::num(b.quality.marketLogLoss, 4), b.quality.modelLogLoss >= b.quality.marketLogLoss ? "market is at least as good" : "model is slightly better");
    m_t4->set(Fmt::pct(b.ece, 2), "lower is better");

    m_models->setRowCount(static_cast<int>(b.models.size()));
    for (int i = 0; i < static_cast<int>(b.models.size()); ++i) {
        const auto& m = b.models[i];
        const bool market = QString::fromStdString(m.name).startsWith("Bookmaker");
        const QColor c = market ? Theme::tint : Theme::text;
        m_models->setItem(i, 0, it(QString::fromStdString(m.name), Qt::AlignLeft, c));
        m_models->setItem(i, 1, it(Fmt::pct(m.scores.accuracy), Qt::AlignCenter, c));
        m_models->setItem(i, 2, it(Fmt::num(m.scores.logLoss, 4), Qt::AlignCenter, c));
        m_models->setItem(i, 3, it(Fmt::num(m.scores.brier, 4), Qt::AlignCenter, c));
        m_models->setItem(i, 4, it(QString::number(m.scores.n), Qt::AlignCenter, Theme::dim));
        m_models->setRowHeight(i, 34);
    }
    m_cal->setBins(b.calibration);
    m_conf->setMatrix(m_showAware ? b.confusionDrawAware : b.confusionPlain);

    m_betting->setRowCount(static_cast<int>(b.betting.size()));
    for (int i = 0; i < static_cast<int>(b.betting.size()); ++i) {
        const auto& s = b.betting[i];
        m_betting->setItem(i, 0, it(QString::fromStdString(s.rule), Qt::AlignLeft));
        m_betting->setItem(i, 1, it(QString::number(s.bets)));
        m_betting->setItem(i, 2, it(s.bets ? Fmt::pct(static_cast<double>(s.wins) / s.bets, 0) : "-"));
        m_betting->setItem(i, 3, it(s.bets ? QString("%1  +/- %2").arg(Fmt::signedPct(s.roi), Fmt::pct(s.roiStdErr)) : "-", Qt::AlignCenter, s.roi >= 0 ? Theme::primary : Theme::danger));
        m_betting->setItem(i, 4, it(Fmt::num(s.maxDrawdown, 1) + " units"));
        m_betting->setItem(i, 5, it(Fmt::num(s.finalBankroll, 1)));
        m_betting->setRowHeight(i, 34);
    }
    const auto& s0 = b.betting.empty() ? sba::BettingSummary{} : b.betting.front();
    m_bettingNote->setText(QString("\"+/-\" is one standard error. With %1 bets the uncertainty is large, so a positive ROI here is not proof of an edge - "
                                   "it needs to survive prospective tracking below. Prices are Bet365 odds from the dataset; real prices move, vary by bookmaker and may be limited.").arg(s0.bets));
}

void PerformancePage::showHistory() {
    if (!m_c->store().isOpen()) { m_prospective->setText("History database is unavailable."); return; }
    const auto sum = m_c->store().summary();
    QString t = QString("%1 saved predictions, %2 settled.").arg(sum.total).arg(sum.settled);
    if (sum.settled >= 1) {
        t += QString(" Model: accuracy %1, log loss %2.").arg(Fmt::pct(sum.model.accuracy, 0), Fmt::num(sum.model.logLoss, 4));
        if (sum.market.n > 0) t += QString(" Market on the same matches: log loss %1.").arg(Fmt::num(sum.market.logLoss, 4));
        if (sum.hypotheticalBets > 0) t += QString(" Hypothetical value picks: %1 bets, %2 won, profit %3 units.").arg(sum.hypotheticalBets).arg(sum.wins).arg(Fmt::num(sum.profit, 2));
        if (sum.settled < 30) t += " (Too few results to draw conclusions.)";
    } else t += " Save predictions from the Analysis page; results are recorded after the matches finish.";
    m_prospective->setText(t);

    const auto all = m_c->store().all();
    m_history->setRowCount(static_cast<int>(all.size()));
    for (int i = 0; i < static_cast<int>(all.size()); ++i) {
        const auto& r = all[i];
        m_history->setItem(i, 0, it(QString::fromStdString(r.createdIso).left(16).replace('T', ' '), Qt::AlignLeft, Theme::dim));
        m_history->setItem(i, 1, it(QString::fromStdString(r.home + " vs " + r.away), Qt::AlignLeft));
        m_history->setItem(i, 2, it(QString::fromStdString(r.kickoffIso).left(10), Qt::AlignLeft, Theme::muted));
        m_history->setItem(i, 3, it(QString("%1 / %2 / %3").arg(r.finalP[0] * 100, 0, 'f', 0).arg(r.finalP[1] * 100, 0, 'f', 0).arg(r.finalP[2] * 100, 0, 'f', 0)));
        m_history->setItem(i, 4, it(QString::fromStdString(r.decision), Qt::AlignLeft, r.decision == sba::decisionLabel(sba::Decision::PossibleValue) ? Theme::primary : Theme::muted));
        m_history->setItem(i, 5, it(std::isfinite(r.odds[0]) ? QString("%1 / %2 / %3").arg(r.odds[0], 0, 'f', 2).arg(r.odds[1], 0, 'f', 2).arg(r.odds[2], 0, 'f', 2) : "-"));
        QString res = "pending"; QColor col = Theme::dim;
        if (r.actual >= 0) {
            const bool hit = sba::argmax(r.finalP) == r.actual;
            res = QString("%1 - %2").arg(sba::outcomeName(r.actual), hit ? "predicted" : "missed"); col = hit ? Theme::primary : Theme::danger;
        }
        m_history->setItem(i, 6, it(res, Qt::AlignLeft, col));
        m_history->setRowHeight(i, 32);
    }
}

void PerformancePage::updateResults() {
    m_settle->setEnabled(false);
    m_c->live()->fetchRecentResults([this](const std::vector<sba::Fixture>& list, const QString& err) {
        m_settle->setEnabled(true);
        if (list.empty() && !err.isEmpty()) { m_banner->show(Banner::Error, "Could not fetch results: " + err); return; }
        const int n = m_c->store().settleFromFixtures(list, QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString());
        m_banner->show(n ? Banner::Success : Banner::Info, QString("%1 prediction(s) settled. %2").arg(n).arg(err));
        showHistory();
    });
}
