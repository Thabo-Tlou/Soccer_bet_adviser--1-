#include "UserScoresPage.h"
#include "UserTheme.h"
#include "ui/Formatting.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QVBoxLayout>
#include <map>

UserScoresPage::UserScoresPage(UserDataHub* hub, QWidget* parent) : QWidget(parent), m_hub(hub) {
    QVBoxLayout* v = nullptr;
    makeColumn(this, v);

    m_dates = new DateStrip; v->addWidget(m_dates);

    auto* bar = new QWidget; bar->setFixedHeight(46);
    auto* bl = new QHBoxLayout(bar); bl->setContentsMargins(0, 0, 14, 0); bl->setSpacing(10);
    m_filter = new TextTabs(false); m_filter->setTabs({"All", "Live", "Finished", "Upcoming", "Favourites"});
    m_search = new QLineEdit; m_search->setPlaceholderText("Search team"); m_search->setClearButtonEnabled(true); m_search->setFixedWidth(180);
    bl->addWidget(m_filter); bl->addStretch(1); bl->addWidget(m_search);
    v->addWidget(bar);
    auto* rule = new QFrame; rule->setObjectName("Rule"); v->addWidget(rule);

    m_notice = new QLabel; m_notice->setWordWrap(true); m_notice->setContentsMargins(16, 8, 16, 8);
    m_notice->setStyleSheet("color:#5c5c5c; background:#f6f6f6; font-size:12px;");
    v->addWidget(m_notice);

    m_scroll = new QScrollArea; m_scroll->setWidgetResizable(true); m_scroll->setFrameShape(QFrame::NoFrame);
    v->addWidget(m_scroll, 1);

    m_debounce.setSingleShot(true); m_debounce.setInterval(60);
    connect(&m_debounce, &QTimer::timeout, this, [this]() { rebuild(); });

    connect(m_dates, &DateStrip::dateChanged, this, [this]() { m_userNavigated = true; rebuild(); });
    connect(m_filter, &TextTabs::changed, this, [this]() { rebuild(); });
    connect(m_search, &QLineEdit::textChanged, this, [this]() { m_debounce.start(); });
    connect(hub, &UserDataHub::dataChanged, this, [this]() {
        if (!m_userNavigated) m_dates->setDate(m_hub->defaultDate());
        m_debounce.start();
    });
    connect(hub, &UserDataHub::favsChanged, this, [this]() { m_debounce.start(); });
    connect(hub, &UserDataHub::pickReady, this, [this]() { m_debounce.start(); });
    connect(hub->controller()->live(), &LiveDataService::statusChanged, this, [this]() { updateNotice(); });

    m_dates->setDate(hub->defaultDate());
    rebuild();
}

void UserScoresPage::showLatest() {
    const QDate d = m_hub->lastResultDate();
    if (d.isValid()) { m_userNavigated = true; m_dates->setDate(d, true); }
}

void UserScoresPage::updateNotice() {
    const auto fx = m_hub->controller()->live()->fixturesStatus();
    QString t;
    if (m_hub->hasFixtures() && fx.state == SourceStatus::NotConfigured) {
        t = "Live updates are off. Showing the last saved scores" + (fx.lastEpoch ? " (updated " + Fmt::ago(fx.lastEpoch) + ")." : QString("."));
    } else if (fx.state == SourceStatus::NotConfigured || (!m_hub->hasFixtures() && fx.state != SourceStatus::Loading)) {
        const QDate last = m_hub->lastResultDate();
        t = "Live scores are not connected yet." + (last.isValid() ? QString(" Showing recorded results up to %1.").arg(QLocale::c().toString(last, "d MMM yyyy")) : QString());
    } else if (fx.state == SourceStatus::Error) {
        t = "Live scores are unavailable right now. Showing the last known data.";
    } else if (fx.state == SourceStatus::Stale) {
        t = "Scores may be out of date (last update " + Fmt::ago(fx.lastEpoch) + ").";
    }
    m_notice->setVisible(!t.isEmpty());
    m_notice->setText(t);
}

void UserScoresPage::rebuild() {
    updateNotice();
    const QDate day = m_dates->date();
    auto all = m_hub->matchesOn(day);
    const QString q = m_search->text().trimmed().toLower();
    int live = 0, fav = 0;
    for (const auto& m : all) { if (m.fx.isLive()) ++live; if (m_hub->isFav(m.key())) ++fav; }
    m_filter->setLabel(1, live ? QString("Live (%1)").arg(live) : QStringLiteral("Live"));
    m_filter->setLabel(4, fav ? QString("Favourites (%1)").arg(fav) : QStringLiteral("Favourites"));

    std::vector<MatchItem> shown;
    for (const auto& m : all) {
        switch (m_filter->current()) {
            case 1: if (!m.fx.isLive()) continue; break;
            case 2: if (!m.fx.isFinished()) continue; break;
            case 3: if (m.fx.isLive() || m.fx.isFinished()) continue; break;
            case 4: if (!m_hub->isFav(m.key())) continue; break;
            default: break;
        }
        if (!q.isEmpty() && !(m.home() + " " + m.away()).toLower().contains(q)) continue;
        shown.push_back(m);
    }

    auto* list = new QWidget;
    auto* lv = new QVBoxLayout(list); lv->setContentsMargins(0, 0, 0, 0); lv->setSpacing(0);
    if (shown.empty()) {
        auto* box = new QWidget; auto* bv = new QVBoxLayout(box); bv->setContentsMargins(24, 56, 24, 24); bv->setSpacing(10);
        QString msg = !m_hub->controller()->ready() ? (m_hub->controller()->initError().isEmpty() ? "Loading data..." : m_hub->controller()->initError()) : all.empty() ? "No matches on this day." : (m_filter->current() == 4 ? "Star a match to follow it here." : "Nothing matches this filter.");
        auto* l = new QLabel(msg); l->setAlignment(Qt::AlignCenter); l->setStyleSheet("color:#8c8c8c; font-size:14px;");
        bv->addWidget(l);
        const QDate last = m_hub->lastResultDate();
        if (all.empty() && last.isValid() && last != day) {
            auto* b = new QPushButton("Go to latest results"); b->setProperty("ubtn", "solid"); b->setCursor(Qt::PointingHandCursor);
            connect(b, &QPushButton::clicked, this, [this]() { showLatest(); });
            bv->addWidget(b, 0, Qt::AlignCenter);
        }
        lv->addWidget(box);
    } else {
        std::map<QString, std::vector<const MatchItem*>> groups;
        std::vector<QString> order;
        for (const auto& m : shown) {
            auto it = groups.find(m.competition());
            if (it == groups.end()) order.push_back(m.competition());
            groups[m.competition()].push_back(&m);
        }
        for (const auto& comp : order) {
            const auto& rows = groups[comp];
            lv->addWidget(new SectionHeader(comp, QString::number(rows.size())));
            for (const MatchItem* m : rows) {
                m_hub->requestPick(*m);
                auto* row = new MatchRow;
                row->setItem(*m, m_hub->pick(*m), m_hub->isFav(m->key()));
                const MatchItem copy = *m;
                connect(row, &MatchRow::clicked, this, [this, copy]() { emit openMatch(copy); });
                connect(row, &MatchRow::favToggled, this, [this, copy]() { m_hub->toggleFav(copy.key()); });
                lv->addWidget(row);
            }
        }
    }
    lv->addStretch(1);
    const int pos = m_scroll->verticalScrollBar()->value();
    m_scroll->setWidget(list);                // the scroll area deletes the previous list
    m_scroll->verticalScrollBar()->setValue(pos);
    m_list = list;
}
