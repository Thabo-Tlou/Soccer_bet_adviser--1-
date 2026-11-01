#include "SettingsPage.h"
#include "Theme.h"
#include <QFormLayout>
#include <QScrollArea>

static QLineEdit* secretField() { auto* e = new QLineEdit; e->setEchoMode(QLineEdit::Password); e->setPlaceholderText("not set"); return e; }

SettingsPage::SettingsPage(AppController* c, QWidget* parent) : QWidget(parent), m_c(c) {
    setObjectName("Page");
    auto* outer = new QVBoxLayout(this); outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea; scroll->setWidgetResizable(true); outer->addWidget(scroll);
    auto* page = new QWidget; page->setObjectName("Page"); scroll->setWidget(page);
    auto* v = new QVBoxLayout(page); v->setContentsMargins(28, 24, 28, 24); v->setSpacing(14);
    v->addWidget(makeLabel("Settings", "h1"));
    v->addWidget(makeLabel("API credentials, refresh behaviour and the explicit rules the reasoning engine applies.", "muted"));
    m_banner = new Banner; v->addWidget(m_banner);

    auto form = [](Card* card) { auto* f = new QFormLayout; f->setLabelAlignment(Qt::AlignLeft); f->setHorizontalSpacing(18); f->setVerticalSpacing(10); card->body()->addLayout(f); return f; };

    auto* api = new Card("Data providers");
    auto* f1 = form(api);
    m_fdKey = secretField(); m_oddsKey = secretField();
    m_comp = new QLineEdit; m_sport = new QLineEdit; m_regions = new QLineEdit;
    f1->addRow("football-data.org key", m_fdKey);
    f1->addRow("The Odds API key", m_oddsKey);
    f1->addRow("Competition code", m_comp);
    f1->addRow("Odds sport key", m_sport);
    f1->addRow("Odds regions", m_regions);
    auto* n = makeLabel("Keys are never written into source files. Environment variables FOOTBALL_DATA_API_KEY and ODDS_API_KEY take priority; "
                        "keys entered here are stored in your user profile (QSettings). Check each provider's terms of use and free-tier limits "
                        "(football-data.org free: ~10 requests/min, limited competitions, odds are a paid add-on; The Odds API free: monthly credit quota).", "dim");
    n->setWordWrap(true); api->body()->addWidget(n);
    auto* test = makeButton("Test football-data.org connection"); api->body()->addWidget(test, 0, Qt::AlignLeft);
    v->addWidget(api);

    auto* rf = new Card("Refresh");
    auto* f2 = form(rf);
    m_auto = new QCheckBox("Automatic refresh"); m_live = new QSpinBox; m_fix = new QSpinBox; m_odds = new QSpinBox;
    m_live->setRange(30, 600); m_live->setSuffix(" s"); m_fix->setRange(1, 120); m_fix->setSuffix(" min"); m_odds->setRange(5, 360); m_odds->setSuffix(" min");
    f2->addRow("", m_auto); f2->addRow("Live scores (only while a match is live)", m_live);
    f2->addRow("Fixtures", m_fix); f2->addRow("Odds (watch your monthly quota)", m_odds);
    v->addWidget(rf);

    auto* rk = new Card("Reasoning rules");
    auto* f3 = form(rk);
    m_rf = new QDoubleSpinBox; m_rf->setRange(0, 1); m_rf->setSingleStep(0.1); m_rf->setDecimals(2);
    m_ev = new QDoubleSpinBox; m_ev->setRange(0, 1); m_ev->setSingleStep(0.01); m_ev->setDecimals(3);
    m_edge = new QDoubleSpinBox; m_edge->setRange(0, 1); m_edge->setSingleStep(0.01); m_edge->setDecimals(3);
    m_dis = new QDoubleSpinBox; m_dis->setRange(0.01, 1); m_dis->setSingleStep(0.01); m_dis->setDecimals(3);
    m_shrink = new QDoubleSpinBox; m_shrink->setRange(0, 1); m_shrink->setSingleStep(0.1); m_shrink->setDecimals(2);
    m_age = new QSpinBox; m_age->setRange(1, 1440); m_age->setSuffix(" min");
    m_minMatches = new QSpinBox; m_minMatches->setRange(1, 50);
    f3->addRow("Random Forest weight in blend (0 = Poisson only)", m_rf);
    f3->addRow("Minimum expected value per unit", m_ev);
    f3->addRow("Minimum model edge over market (probability)", m_edge);
    f3->addRow("Maximum RF/Poisson disagreement", m_dis);
    f3->addRow("Shrink model toward market (1 = no shrink)", m_shrink);
    f3->addRow("Odds count as stale after", m_age);
    f3->addRow("Minimum matches of history per team", m_minMatches);
    auto* rn = makeLabel("Stricter thresholds produce fewer 'possible value' results. Defaults were chosen conservatively because the models have not been shown to beat bookmaker probabilities.", "dim");
    rn->setWordWrap(true); rk->body()->addWidget(rn);
    v->addWidget(rk);

    auto* env = new Card("Environment");
    m_info = makeLabel("", "mono"); m_info->setWordWrap(true); m_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    env->body()->addWidget(m_info);
    v->addWidget(env);

    for (QWidget* w : std::initializer_list<QWidget*>{m_live, m_fix, m_odds, m_rf, m_ev, m_edge, m_dis, m_shrink, m_age, m_minMatches, m_comp, m_sport, m_regions}) w->setMaximumWidth(260);
    for (QWidget* w : std::initializer_list<QWidget*>{m_fdKey, m_oddsKey}) w->setMaximumWidth(520);
    auto* save = makeButton("Save settings", true);
    v->addWidget(save, 0, Qt::AlignLeft);
    v->addStretch();

    connect(save, &QPushButton::clicked, this, [this]() { m_c->applySettings(collect()); m_banner->show(Banner::Success, "Settings saved."); load(); });
    connect(test, &QPushButton::clicked, this, [this, test]() {
        m_c->applySettings(collect()); test->setEnabled(false);
        m_c->live()->testFootballConnection([this, test](bool ok, const QString& msg) {
            test->setEnabled(true); m_banner->show(ok ? Banner::Success : Banner::Error, "football-data.org: " + msg);
        });
    });
    load();
}

void SettingsPage::load() {
    const auto& s = m_c->settings();
    m_fdKey->setText(s.footballDataKey); m_oddsKey->setText(s.oddsApiKey);
    m_comp->setText(s.competitionCode); m_sport->setText(s.oddsSportKey); m_regions->setText(s.oddsRegions);
    m_auto->setChecked(s.autoRefresh); m_live->setValue(s.liveRefreshSec); m_fix->setValue(s.fixturesRefreshMin); m_odds->setValue(s.oddsRefreshMin);
    m_rf->setValue(s.rfWeight); m_ev->setValue(s.risk.minExpectedValue); m_edge->setValue(s.risk.minEdge); m_dis->setValue(s.risk.maxDisagreement);
    m_shrink->setValue(s.risk.shrinkTowardMarket); m_age->setValue(s.risk.maxOddsAgeMinutes); m_minMatches->setValue(s.risk.minMatchesPlayed);
    m_info->setText(QString("football-data key: %1%2\nOdds API key:      %3%4\nResources:         %5\nHistory database:  %6\nKnowledge index:   %7")
                        .arg(AppSettings::mask(s.effectiveFootballKey()), qEnvironmentVariableIsSet("FOOTBALL_DATA_API_KEY") ? " (from environment)" : "",
                             AppSettings::mask(s.effectiveOddsKey()), qEnvironmentVariableIsSet("ODDS_API_KEY") ? " (from environment)" : "",
                             m_c->resourcesRoot().isEmpty() ? "not found" : m_c->resourcesRoot(), m_c->dbPath(), m_c->indexRoot()));
}

AppSettings SettingsPage::collect() const {
    AppSettings s = m_c->settings();
    s.footballDataKey = m_fdKey->text().trimmed(); s.oddsApiKey = m_oddsKey->text().trimmed();
    s.competitionCode = m_comp->text().trimmed().toUpper(); s.oddsSportKey = m_sport->text().trimmed(); s.oddsRegions = m_regions->text().trimmed();
    s.autoRefresh = m_auto->isChecked(); s.liveRefreshSec = m_live->value(); s.fixturesRefreshMin = m_fix->value(); s.oddsRefreshMin = m_odds->value();
    s.rfWeight = m_rf->value(); s.risk.minExpectedValue = m_ev->value(); s.risk.minEdge = m_edge->value(); s.risk.maxDisagreement = m_dis->value();
    s.risk.shrinkTowardMarket = m_shrink->value(); s.risk.maxOddsAgeMinutes = m_age->value(); s.risk.minMatchesPlayed = m_minMatches->value();
    return s;
}
