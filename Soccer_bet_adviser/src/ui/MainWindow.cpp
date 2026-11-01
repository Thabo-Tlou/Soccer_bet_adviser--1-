#include "MainWindow.h"
#include "DashboardPage.h"
#include "Formatting.h"
#include "MatchesPage.h"
#include "PerformancePage.h"
#include "SearchPage.h"
#include "SettingsPage.h"
#include "Theme.h"
#include <QElapsedTimer>
#include <QKeySequence>
#include <QLineEdit>
#include <QShortcut>

static const char* kPageNames[] = {"Dashboard", "Live & upcoming", "Match analysis", "Performance", "Knowledge search", "Settings"};

MainWindow::MainWindow(AppController* c, QWidget* parent) : QMainWindow(parent), m_c(c) {
    setWindowTitle("Soccer Bet Adviser");
    resize(1360, 860); setMinimumSize(1080, 700);
    auto* central = new QWidget; setCentralWidget(central);
    auto* grid = new QVBoxLayout(central); grid->setContentsMargins(0, 0, 0, 0); grid->setSpacing(0);

    // ---------------- top bar
    auto* top = new QWidget; top->setObjectName("TopBar"); top->setFixedHeight(46);
    auto* tl = new QHBoxLayout(top); tl->setContentsMargins(16, 0, 16, 0); tl->setSpacing(10);
    auto* logo = new QLabel; logo->setText(QString("<span style='color:%1; font-size:13pt; font-weight:700'>&#9917;</span>").arg(Theme::hex(Theme::primary)));
    auto* brand = new QLabel("Soccer Bet Adviser"); brand->setStyleSheet("font-weight:600;");
    auto* sep = new QLabel("/"); sep->setStyleSheet(QString("color:%1;").arg(Theme::hex(Theme::dim)));
    m_crumb = new QLabel; m_crumb->setStyleSheet(QString("color:%1;").arg(Theme::hex(Theme::muted)));
    m_modelChip = new Badge("Model: loading", Badge::Neutral); m_apiChip = new Badge("API: -", Badge::Neutral);
    tl->addWidget(logo); tl->addWidget(brand); tl->addWidget(sep); tl->addWidget(m_crumb); tl->addStretch();
    tl->addWidget(m_modelChip, 0, Qt::AlignVCenter); tl->addWidget(m_apiChip, 0, Qt::AlignVCenter);
    auto* userView = makeButton("User view");
    userView->setToolTip("Switch to the user side of the app");
    tl->addWidget(userView, 0, Qt::AlignVCenter);
    connect(userView, &QPushButton::clicked, this, [this]() { emit userViewRequested(); });
    grid->addWidget(top);

    // ---------------- body
    auto* body = new QHBoxLayout; body->setContentsMargins(0, 0, 0, 0); body->setSpacing(0);
    auto* bar = new QWidget; bar->setObjectName("ActivityBar"); bar->setFixedWidth(52);
    auto* bl = new QVBoxLayout(bar); bl->setContentsMargins(0, 6, 0, 6); bl->setSpacing(0);
    m_nav = new QButtonGroup(this); m_nav->setExclusive(true);
    const NavButton::Icon icons[] = {NavButton::Dashboard, NavButton::Matches, NavButton::Analysis, NavButton::Performance, NavButton::Search, NavButton::Settings};
    for (int i = 0; i < 6; ++i) {
        auto* b = new NavButton(icons[i], QString("%1  (Ctrl+%2)").arg(kPageNames[i]).arg(i + 1));
        m_nav->addButton(b, i);
        if (i == 5) bl->addStretch();
        bl->addWidget(b);
    }
    body->addWidget(bar);

    m_stack = new QStackedWidget;
    auto* dash = new DashboardPage(c); auto* matches = new MatchesPage(c); m_analysis = new AnalysisPage(c);
    auto* perf = new PerformancePage(c); auto* search = new SearchPage(c); auto* settings = new SettingsPage(c);
    for (QWidget* w : std::initializer_list<QWidget*>{dash, matches, m_analysis, perf, search, settings}) m_stack->addWidget(w);
    body->addWidget(m_stack, 1);
    grid->addLayout(body, 1);

    // ---------------- status bar
    auto* sb = new QWidget; sb->setObjectName("StatusBar"); sb->setFixedHeight(24);
    auto* sl = new QHBoxLayout(sb); sl->setContentsMargins(12, 0, 12, 0);
    m_statusText = new QLabel("Starting..."); m_statusRight = new QLabel(QString("v%1   |   Estimates only - not financial advice").arg(SBA_VERSION));
    sl->addWidget(m_statusText); sl->addStretch(); sl->addWidget(m_statusRight);
    grid->addWidget(sb);

    connect(m_nav, &QButtonGroup::idClicked, this, [this](int id) { goTo(id); });
    for (int i = 0; i < 6; ++i) { auto* s = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(i + 1)), this); connect(s, &QShortcut::activated, this, [this, i]() { goTo(i); }); }
    connect(dash, &DashboardPage::gotoPage, this, &MainWindow::goTo);
    auto open = [this](const sba::Fixture& f) { goTo(2); m_analysis->loadFixture(f); };
    connect(dash, &DashboardPage::openAnalysis, this, open);
    connect(matches, &MatchesPage::openAnalysis, this, open);
    connect(c, &AppController::initialised, this, [this]() { refreshChips(); });
    connect(c, &AppController::backtestFinished, this, [this]() { refreshChips(); });
    connect(c, &AppController::backtestStarted, this, [this]() { m_statusText->setText("Running held-out backtest..."); });
    connect(c->live(), &LiveDataService::statusChanged, this, [this]() { refreshChips(); });
    goTo(0);
    refreshChips();
}

void MainWindow::goTo(int i) {
    m_stack->setCurrentIndex(i);
    if (auto* b = m_nav->button(i)) b->setChecked(true);
    m_crumb->setText(kPageNames[i]);
}

void MainWindow::refreshChips() {
    const auto& st = m_c->service().status();
    if (!m_c->ready()) m_modelChip->set(m_c->initError().isEmpty() ? "Model: loading" : "Model: unavailable", m_c->initError().isEmpty() ? Badge::Neutral : Badge::Danger);
    else if (st.rf && st.poisson) m_modelChip->set("Model: RF + Poisson", Badge::Success);
    else m_modelChip->set(st.rf ? "Model: RF only" : "Model: Poisson only", Badge::Warning);

    const auto fx = m_c->live()->fixturesStatus();
    switch (fx.state) {
        case SourceStatus::Fresh: m_apiChip->set("API: live", Badge::Success); break;
        case SourceStatus::Stale: m_apiChip->set("API: stale", Badge::Warning); break;
        case SourceStatus::Error: m_apiChip->set("API: error", Badge::Danger); break;
        case SourceStatus::Loading: m_apiChip->set("API: refreshing", Badge::Info); break;
        case SourceStatus::NotConfigured: m_apiChip->set("API: not configured", Badge::Neutral); break;
        default: m_apiChip->set("API: idle", Badge::Neutral); break;
    }
    if (m_c->ready())
        m_statusText->setText(QString("%1 matches  |  %2 to %3  |  fixtures %4")
                                  .arg(st.matches).arg(QString::fromStdString(st.firstDate), QString::fromStdString(st.lastDate), fx.lastEpoch ? Fmt::ago(fx.lastEpoch) : "not loaded"));
    else m_statusText->setText(m_c->initError().isEmpty() ? "Loading models and data..." : m_c->initError());
}

#include <QApplication>
#include <QDir>
#include <QTimer>
void MainWindow::captureScreens(const QString& dir) {
    QDir().mkpath(dir);
    const char* names[] = {"dashboard", "matches", "analysis", "performance", "search", "settings"};
    auto shot = [this, dir](const QString& n) { QApplication::processEvents(); grab().save(dir + "/" + n + ".png"); };
    for (int i = 0; i < 6; ++i) {
        goTo(i);
        if (i == 2) {
            sba::Fixture f; f.homeDataset = "Arsenal"; f.awayDataset = "Chelsea"; f.home = "Arsenal"; f.away = "Chelsea";
            f.kickoffEpoch = QDateTime::currentSecsSinceEpoch() + 2 * 86400; f.status = "TIMED"; f.oddsFetchedEpoch = QDateTime::currentSecsSinceEpoch() - 300;
            sba::BookOdds b; b.bookmaker = "Bet365"; b.odds[0] = 1.95; b.odds[1] = 3.60; b.odds[2] = 4.10; f.books.push_back(b);
            m_analysis->loadFixture(f);
            QElapsedTimer t; t.start();
            while (t.elapsed() < 2500) { QApplication::processEvents(QEventLoop::AllEvents, 50); }
        }
        if (i == 4) {
            auto* page = qobject_cast<QWidget*>(m_stack->widget(4));
            if (auto* le = page->findChild<QLineEdit*>()) { le->setText("Where is ONNX initialized?"); emit le->returnPressed(); }
        }
        shot(names[i]);
    }
}
