#include "UserWindow.h"
#include "UserTheme.h"
#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

UserWindow::UserWindow(AppController* c, QWidget* parent) : QMainWindow(parent), m_c(c) {
    setWindowTitle("Soccer Bet Adviser");
    resize(1180, 820);
    setMinimumSize(800, 600);
    m_hub = new UserDataHub(c, this);

    auto* root = new QWidget; root->setObjectName("UserRoot");
    setCentralWidget(root);
    auto* rv = new QVBoxLayout(root); rv->setContentsMargins(0, 0, 0, 0); rv->setSpacing(0);

    // ---------------------------------------------------------------- header (black)
    auto* header = new QFrame; header->setObjectName("UHeader"); header->setFixedHeight(52);
    header->setStyleSheet("QFrame#UHeader { background: #0b0b0b; }");
    auto* hl = new QHBoxLayout(header); hl->setContentsMargins(20, 0, 16, 0); hl->setSpacing(24);
    auto* brand = new QLabel("<span style='color:#ffffff'>Soccer Bet</span> <span style='color:#2ecc71'>Adviser</span>");
    brand->setStyleSheet("font-size:18px; font-weight:700; background:transparent;");
    m_nav = new TextTabs(true);
    m_nav->setTabs({"Scores", "Predictions", "Table", "My picks"});
    auto* admin = new QPushButton("Admin"); admin->setProperty("ubtn", "darkghost"); admin->setCursor(Qt::PointingHandCursor);
    admin->setToolTip("Open the admin panel");
    hl->addWidget(brand); hl->addWidget(m_nav); hl->addStretch(1); hl->addWidget(admin);
    rv->addWidget(header);

    // ---------------------------------------------------------------- pages
    m_stack = new QStackedWidget;
    m_scores = new UserScoresPage(m_hub);
    m_predictions = new UserPredictionsPage(m_hub);
    m_table = new UserTablePage(m_hub);
    m_picks = new UserPicksPage(m_hub);
    m_match = new UserMatchPage(m_hub);
    for (QWidget* w : std::initializer_list<QWidget*>{m_scores, m_predictions, m_table, m_picks, m_match}) m_stack->addWidget(w);
    rv->addWidget(m_stack, 1);

    // ---------------------------------------------------------------- footer
    auto* foot = new QLabel("Estimates only, not financial advice.   18+   Please gamble responsibly.");
    foot->setAlignment(Qt::AlignCenter); foot->setFixedHeight(28);
    foot->setStyleSheet("color:#8c8c8c; font-size:11px; background:#ffffff; border-top:1px solid #e6e6e6;");
    rv->addWidget(foot);

    connect(m_nav, &TextTabs::changed, this, [this](int i) { goTab(i); });
    connect(admin, &QPushButton::clicked, this, [this]() { emit adminRequested(); });
    connect(m_scores, &UserScoresPage::openMatch, this, [this](const MatchItem& m) { openMatch(m); });
    connect(m_predictions, &UserPredictionsPage::openMatch, this, [this](const MatchItem& m) { openMatch(m); });
    connect(m_match, &UserMatchPage::backRequested, this, [this]() { m_stack->setCurrentIndex(m_prev); });
    for (int i = 0; i < 4; ++i) {
        auto* s = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(i + 1)), this);
        connect(s, &QShortcut::activated, this, [this, i]() { m_nav->setCurrent(i); goTab(i); });
    }
    auto* esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(esc, &QShortcut::activated, this, [this]() { if (m_stack->currentWidget() == m_match) m_stack->setCurrentIndex(m_prev); });
}

void UserWindow::goTab(int i) {
    m_stack->setCurrentIndex(i);
    if (i == 3) m_picks->refresh();
}

void UserWindow::openMatch(const MatchItem& m) {
    if (m_stack->currentWidget() != m_match) m_prev = m_stack->currentIndex();
    m_match->showMatch(m);
    m_stack->setCurrentWidget(m_match);
}

void UserWindow::captureScreens(const QString& dir) {
    QDir().mkpath(dir);
    auto wait = [](int ms) { QElapsedTimer t; t.start(); while (t.elapsed() < ms) QApplication::processEvents(QEventLoop::AllEvents, 40); };
    auto shot = [this, dir, &wait](const QString& n) { wait(250); grab().save(dir + "/" + n + ".png"); };
    shot("scores");
    // first finished match of the shown day, then an upcoming one
    auto day = m_hub->matchesOn(m_hub->defaultDate());
    if (!day.empty()) {
        m_match->showMatch(day.front()); m_stack->setCurrentWidget(m_match); shot("match_finished_h2h");
    }
    MatchItem up;
    for (const auto& m : m_hub->upcoming(14)) { up = m; break; }
    if (up.fx.home.empty()) {
        up.manual = true; up.day = QDate::currentDate().addDays(1);
        up.fx.home = up.fx.homeDataset = "Arsenal"; up.fx.away = up.fx.awayDataset = "Chelsea";
        up.fx.competition = "Premier League"; up.fx.status = "TIMED";
        sba::BookOdds b; b.bookmaker = "Your odds"; b.odds[0] = 1.95; b.odds[1] = 3.6; b.odds[2] = 4.1; b.updatedEpoch = QDateTime::currentSecsSinceEpoch();
        up.fx.books.push_back(b);
    }
    m_match->showMatch(up); m_stack->setCurrentWidget(m_match);
    wait(2500); shot("match_prediction");
    m_nav->setCurrent(1); m_stack->setCurrentIndex(1); wait(1500); shot("predictions");
    m_nav->setCurrent(2); m_stack->setCurrentIndex(2); shot("table");
    m_nav->setCurrent(3); goTab(3); shot("picks");
}
