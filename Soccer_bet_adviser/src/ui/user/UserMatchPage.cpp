#include "UserMatchPage.h"
#include "UserTheme.h"
#include "ui/Formatting.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <cmath>
#include <functional>

static QLabel* lab(const QString& t, int px = 13, bool bold = false, const QString& color = "#0b0b0b") {
    auto* l = new QLabel(t);
    l->setWordWrap(true);
    l->setStyleSheet(QString("color:%1; font-size:%2px; font-weight:%3; background:transparent;").arg(color).arg(px).arg(bold ? 600 : 400));
    return l;
}
static QLabel* caption(const QString& t) {
    auto* l = lab(t.toUpper(), 11, true, "#8c8c8c");
    l->setContentsMargins(0, 6, 0, 0);
    return l;
}
static QFrame* rule() { auto* f = new QFrame; f->setObjectName("Rule"); return f; }

// ---------------------------------------------------------------------------- scoreboard header (black)

class MatchHeader : public QWidget {
public:
    std::function<void()> onStar;
    explicit MatchHeader(QWidget* p = nullptr) : QWidget(p) { setFixedHeight(156); setMouseTracking(true); }
    void set(const MatchItem& m, bool fav) { m_item = m; m_fav = fav; update(); }
protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (e->position().x() > width() - 56 && e->position().y() < 56 && onStar) onStar();
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int W = width(), H = height();
        p.fillRect(rect(), UT::black);
        const auto& f = m_item.fx;
        const bool live = f.isLive(), fin = f.isFinished();
        const int cx = W / 2, by = 62;
        paintTeamBadge(p, QPointF(W * 0.2, by), 28, m_item.home(), true);
        paintTeamBadge(p, QPointF(W * 0.8, by), 28, m_item.away(), true);
        p.setFont(UT::font(16, true)); p.setPen(UT::white);
        QFontMetrics fm(p.font());
        const int nw = int(W * 0.34);
        p.drawText(QRect(int(W * 0.2) - nw / 2, by + 38, nw, 24), Qt::AlignCenter, fm.elidedText(m_item.home(), Qt::ElideRight, nw));
        p.drawText(QRect(int(W * 0.8) - nw / 2, by + 38, nw, 24), Qt::AlignCenter, fm.elidedText(m_item.away(), Qt::ElideRight, nw));

        QString big, sub;
        if (m_item.hasScore() && (live || fin)) {
            big = QString("%1 - %2").arg(f.homeGoals).arg(f.awayGoals);
            sub = live ? (f.status == "PAUSED" ? "Half-time" : (f.minute >= 0 ? QString("%1'").arg(f.minute) : "Live")) : "Full time";
        } else if (m_item.manual) {
            big = "vs"; sub = QLocale::c().toString(m_item.day, "ddd d MMM");
        } else if (f.status == "POSTPONED" || f.status == "CANCELLED" || f.status == "SUSPENDED") {
            big = "-"; sub = QString::fromStdString(f.status).left(1) + QString::fromStdString(f.status).mid(1).toLower();
        } else {
            big = MatchRow::statusText(m_item);
            sub = QLocale::c().toString(m_item.day, "ddd d MMM");
        }
        p.setFont(UT::font(big.size() > 5 ? 34 : 36, true));
        p.setPen(live ? UT::greenOnDark : UT::white);
        p.drawText(QRect(cx - 110, by - 24, 220, 48), Qt::AlignCenter, big);
        p.setFont(UT::font(13, live));
        p.setPen(live ? UT::greenOnDark : QColor("#a8a8a8"));
        p.drawText(QRect(cx - 110, by + 26, 220, 20), Qt::AlignCenter, sub);

        const QPointF sc(W - 30, 28);
        static const auto star = [](const QPointF& c, double r) {
            QPolygonF poly;
            for (int i = 0; i < 10; ++i) { const double a = -M_PI / 2 + i * M_PI / 5, rr = (i % 2 == 0) ? r : r * 0.45; poly << QPointF(c.x() + rr * std::cos(a), c.y() + rr * std::sin(a)); }
            return poly;
        };
        p.setBrush(m_fav ? QBrush(UT::greenOnDark) : Qt::NoBrush);
        p.setPen(QPen(m_fav ? UT::greenOnDark : QColor("#7a7a7a"), 1.5));
        p.drawPolygon(star(sc, 10));
        (void)H;
    }
private:
    MatchItem m_item;
    bool m_fav = false;
};

// ---------------------------------------------------------------------------- stat comparison bar

class StatBar : public QWidget {
public:
    StatBar(const QString& name, double h, double a, QWidget* p = nullptr) : QWidget(p), m_name(name), m_h(h), m_a(a) { setFixedHeight(46); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int W = width();
        const bool hb = m_h > m_a, ab = m_a > m_h;
        p.setFont(UT::font(14, true));
        p.setPen(UT::black);
        p.drawText(QRect(0, 0, 60, 24), Qt::AlignLeft | Qt::AlignVCenter, QString::number(int(m_h)));
        p.drawText(QRect(W - 60, 0, 60, 24), Qt::AlignRight | Qt::AlignVCenter, QString::number(int(m_a)));
        p.setFont(UT::font(12)); p.setPen(UT::soft);
        p.drawText(QRect(0, 0, W, 24), Qt::AlignCenter, m_name);
        const double tot = std::max(1.0, m_h + m_a), half = (W - 12) / 2.0;
        const QRectF l(0, 30, half, 6), r(half + 12, 30, half, 6);
        p.setPen(Qt::NoPen); p.setBrush(QColor("#ececec"));
        p.drawRoundedRect(l, 3, 3); p.drawRoundedRect(r, 3, 3);
        const double lw = half * (m_h / tot) * 1.0, rw = half * (m_a / tot);
        p.setBrush(hb ? UT::green : QColor("#bdbdbd"));
        p.drawRoundedRect(QRectF(l.right() - lw, l.y(), lw, 6), 3, 3);
        p.setBrush(ab ? UT::green : QColor("#bdbdbd"));
        p.drawRoundedRect(QRectF(r.x(), r.y(), rw, 6), 3, 3);
    }
private:
    QString m_name; double m_h, m_a;
};

// ---------------------------------------------------------------------------- page

UserMatchPage::UserMatchPage(UserDataHub* hub, QWidget* parent) : QWidget(parent), m_hub(hub) {
    QVBoxLayout* v = nullptr;
    makeColumn(this, v);

    auto* top = new QWidget; top->setFixedHeight(42);
    auto* tl = new QHBoxLayout(top); tl->setContentsMargins(10, 0, 16, 0);
    auto* back = new QPushButton("< Back"); back->setProperty("ubtn", "link"); back->setCursor(Qt::PointingHandCursor);
    m_context = new QLabel; m_context->setStyleSheet("color:#8c8c8c; font-size:12px;");
    tl->addWidget(back); tl->addStretch(1); tl->addWidget(m_context);
    v->addWidget(top);

    m_header = new MatchHeader; v->addWidget(m_header);
    m_header->onStar = [this]() { if (m_hasItem) { m_hub->toggleFav(m_item.key()); } };

    m_tabs = new TextTabs(false);
    v->addWidget(m_tabs);
    v->addWidget(rule());
    m_scroll = new QScrollArea; m_scroll->setWidgetResizable(true); m_scroll->setFrameShape(QFrame::NoFrame);
    v->addWidget(m_scroll, 1);

    connect(back, &QPushButton::clicked, this, [this]() { emit backRequested(); });
    connect(m_tabs, &TextTabs::changed, this, [this](int i) { const auto n = tabNames(); if (i >= 0 && i < n.size()) m_tabName = n[i]; buildBody(); });
    connect(hub, &UserDataHub::pickReady, this, [this]() { if (m_hasItem && m_tabName == "Prediction") buildBody(); });
    connect(hub, &UserDataHub::favsChanged, this, [this]() { if (m_hasItem) m_header->set(m_item, m_hub->isFav(m_item.key())); });
    connect(hub, &UserDataHub::dataChanged, this, [this]() { applyUpdate(); });
}

QStringList UserMatchPage::tabNames() const {
    if (m_hasItem && m_item.fx.isFinished()) {
        QStringList t;
        if (m_hub->findHist(m_item)) t << "Stats";
        t << "H2H";
        return t;
    }
    return {"Prediction", "Odds", "H2H"};
}

void UserMatchPage::showMatch(const MatchItem& m) {
    m_item = m; m_hasItem = true;
    m_context->setText(m.competition() + "  |  " + QLocale::c().toString(m.day, "ddd d MMM yyyy"));
    m_header->set(m, m_hub->isFav(m.key()));
    const QStringList names = tabNames();
    m_tabs->setTabs(names);
    m_tabs->setCurrent(0);
    m_tabName = names.first();
    m_hub->requestPick(m_item);
    buildBody();
}

void UserMatchPage::applyUpdate() {
    if (!m_hasItem || m_item.manual || !isVisible()) return;
    for (const auto& n : m_hub->matchesOn(m_item.day)) {
        if (n.key() != m_item.key()) continue;
        const auto& a = m_item.fx; const auto& b = n.fx;
        const bool changed = a.status != b.status || a.homeGoals != b.homeGoals || a.awayGoals != b.awayGoals || a.minute != b.minute ||
                             a.books.size() != b.books.size() || a.oddsFetchedEpoch != b.oddsFetchedEpoch;
        if (!changed) return;
        m_item = n;
        m_header->set(m_item, m_hub->isFav(m_item.key()));
        m_hub->requestPick(m_item);
        buildBody();
        return;
    }
}

void UserMatchPage::buildBody() {
    if (!m_hasItem) return;
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(20, 14, 20, 24);
    v->setSpacing(10);
    if (m_tabName == "Odds") buildOdds(v);
    else if (m_tabName == "H2H") buildH2H(v);
    else if (m_tabName == "Stats") buildStats(v);
    else buildPrediction(v);
    v->addStretch(1);
    m_scroll->setWidget(w);
}

// ---------------------------------------------------------------------------- Prediction

void UserMatchPage::buildPrediction(QVBoxLayout* v) {
    auto* c = m_hub->controller();
    auto note = [&](const QString& t) { v->addSpacing(24); auto* l = lab(t, 14, false, "#5c5c5c"); l->setAlignment(Qt::AlignCenter); v->addWidget(l); };
    if (m_item.fx.isFinished()) { note("This match is finished. Predictions are only made before kick-off."); return; }
    if (m_item.fx.isLive()) { note("This match is in play. Predictions are only made before kick-off."); return; }
    if (!m_item.canPredict()) { note("No prediction for this match: one of the teams is not in the historical data."); return; }
    if (!c->ready()) { note(c->initError().isEmpty() ? "Loading the models..." : "Models unavailable: " + c->initError()); return; }
    const PickInfo* pi = m_hub->pick(m_item);
    if (!pi) { note("Working out the prediction..."); return; }
    if (!pi->ok || !pi->a) { note("No prediction: " + pi->error); return; }

    const auto& r = pi->a->reasoning;
    const auto& e = pi->a->evidence;

    v->addWidget(caption("Our pick"));
    auto* head = new QHBoxLayout;
    head->addWidget(lab(outcomeText(m_item, pi->best), 22, true), 1);
    head->addWidget(lab(pctText(pi->p), 22, true, UT::hex(UT::green)));
    v->addLayout(head);
    v->addWidget(lab("Most likely result according to the combined model. It is a probability, not a promise.", 12, false, "#8c8c8c"));

    auto* pr = new ProbRows;
    const QString labels[3] = {m_item.home(), "Draw", m_item.away()};
    pr->set(labels, r.model, r.hasMarket, r.market);
    v->addWidget(pr);

    // verdict
    QString title, accent;
    switch (r.decision) {
        case sba::Decision::PossibleValue: title = "Value spotted"; accent = UT::hex(UT::green); break;
        case sba::Decision::InsufficientEvidence: title = "Not enough to call"; accent = "#8c8c8c"; break;
        default: title = "No clear value"; accent = "#0b0b0b"; break;
    }
    auto* box = new QFrame;
    box->setStyleSheet(QString("QFrame#Verdict { background:#f6f6f6; border:0; border-left:3px solid %1; }").arg(accent));
    box->setObjectName("Verdict");
    auto* bv = new QVBoxLayout(box); bv->setContentsMargins(14, 10, 14, 12); bv->setSpacing(4);
    bv->addWidget(lab(title, 15, true, r.decision == sba::Decision::PossibleValue ? UT::hex(UT::green) : "#0b0b0b"));
    const QString headline = QString::fromStdString(r.headline);
    if (headline.compare("No recommendation", Qt::CaseInsensitive) != 0 && headline.compare(title, Qt::CaseInsensitive) != 0)
        bv->addWidget(lab(headline, 13, false, "#2b2b2b"));
    if (r.hasMarket && r.candidate >= 0)
        bv->addWidget(lab(QString("%1:  model edge %2 pts  |  expected value %3  |  bookmaker margin %4")
                              .arg(outcomeText(m_item, r.candidate), Fmt::signedPct(r.edge[static_cast<size_t>(r.candidate)]).remove('%'),
                                   Fmt::signedPct(r.ev[static_cast<size_t>(r.candidate)]), Fmt::pct(r.overround)), 12, false, "#5c5c5c"));
    else
        bv->addWidget(lab("No bookmaker odds yet, so value cannot be checked. Odds appear here once a price is available.", 12, false, "#5c5c5c"));
    v->addWidget(box);

    if (std::isfinite(e.lambdaHome) && std::isfinite(e.lambdaAway)) {
        auto* row = new QHBoxLayout;
        row->addWidget(lab("Expected goals", 13, false, "#5c5c5c"), 1);
        row->addWidget(lab(QString("%1  -  %2").arg(e.lambdaHome, 0, 'f', 2).arg(e.lambdaAway, 0, 'f', 2), 14, true));
        v->addLayout(row);
    }

    auto bullets = [&](const QString& heading, const std::vector<std::string>& items, int max) {
        if (items.empty()) return;
        v->addWidget(caption(heading));
        int n = 0;
        for (const auto& s : items) { if (n++ >= max) break; v->addWidget(lab(QString::fromUtf8("\u2022  ") + QString::fromStdString(s), 13, false, "#2b2b2b")); }
    };
    if (r.decision != sba::Decision::PossibleValue) bullets("Why no value call", r.withheld, 2);
    bullets("What points this way", r.supporting, 3);
    bullets("What points against", r.counter, 3);

    v->addSpacing(6);
    auto* actions = new QHBoxLayout;
    auto* save = new QPushButton("Save to My picks"); save->setProperty("ubtn", "solid"); save->setCursor(Qt::PointingHandCursor);
    auto* msg = lab("", 12, false, "#5c5c5c"); msg->setWordWrap(false);
    actions->addWidget(save); actions->addWidget(msg, 1);
    v->addLayout(actions);
    connect(save, &QPushButton::clicked, this, [this, save, msg]() {
        QString text;
        const bool ok = m_hub->savePick(m_item, &text);
        msg->setText(text);
        if (ok) { save->setText("Saved"); save->setEnabled(false); }
    });
    v->addWidget(lab("Estimates only, not financial advice. Models were about as accurate as bookmaker odds in testing, not better. 18+, gamble responsibly.", 11, false, "#8c8c8c"));
}

// ---------------------------------------------------------------------------- Odds

void UserMatchPage::buildOdds(QVBoxLayout* v) {
    const auto& f = m_item.fx;
    if (f.books.empty()) {
        v->addSpacing(24);
        auto* l = lab(f.isFinished() ? "Odds are not kept for finished matches." : "No bookmaker odds for this match yet.", 14, false, "#5c5c5c");
        l->setAlignment(Qt::AlignCenter); v->addWidget(l); return;
    }
    double best[3]; f.bestOdds(best);
    auto* g = new QGridLayout; g->setHorizontalSpacing(8); g->setVerticalSpacing(0);
    const QStringList heads{"Bookmaker", "1", "X", "2", "Updated"};
    for (int c = 0; c < 5; ++c) {
        auto* h = lab(heads[c].toUpper(), 11, true, "#8c8c8c");
        h->setAlignment(c == 0 ? Qt::AlignLeft : Qt::AlignCenter); h->setContentsMargins(0, 6, 0, 8);
        g->addWidget(h, 0, c);
    }
    int row = 1;
    for (const auto& b : f.books) {
        g->addWidget(rule(), row * 2 - 1 + 0, 0, 1, 5);
        auto* n = lab(QString::fromStdString(b.bookmaker), 14, true); n->setContentsMargins(0, 10, 0, 10);
        g->addWidget(n, row * 2, 0);
        for (int k = 0; k < 3; ++k) {
            const bool isBest = std::fabs(b.odds[k] - best[k]) < 1e-9;
            auto* o = lab(Fmt::num(b.odds[k]), 14, isBest, isBest ? UT::hex(UT::green) : "#0b0b0b");
            o->setAlignment(Qt::AlignCenter);
            g->addWidget(o, row * 2, 1 + k);
        }
        auto* u = lab(b.updatedEpoch ? Fmt::ago(b.updatedEpoch) : QString("-"), 12, false, "#8c8c8c"); u->setAlignment(Qt::AlignCenter);
        g->addWidget(u, row * 2, 4);
        ++row;
    }
    g->setColumnStretch(0, 3); for (int c = 1; c < 5; ++c) g->setColumnStretch(c, 1);
    v->addLayout(g);
    v->addWidget(lab("Green is the best price for that result.", 12, false, "#8c8c8c"));
    const int maxAge = m_hub->controller()->settings().risk.maxOddsAgeMinutes;
    if (!m_item.manual && f.oddsFetchedEpoch > 0 && QDateTime::currentSecsSinceEpoch() - f.oddsFetchedEpoch > maxAge * 60LL)
        v->addWidget(lab("These odds are more than " + QString::number(maxAge) + " minutes old and may have changed.", 12, true, "#0b0b0b"));
}

// ---------------------------------------------------------------------------- H2H

void UserMatchPage::buildH2H(QVBoxLayout* v) {
    if (!m_item.canPredict()) {
        v->addSpacing(24);
        auto* l = lab("No history for this match: one of the teams is not in the data.", 14, false, "#5c5c5c");
        l->setAlignment(Qt::AlignCenter); v->addWidget(l); return;
    }
    v->addWidget(caption("Form before this match"));
    for (const QString& t : {m_item.home(), m_item.away()}) {
        auto* row = new QHBoxLayout;
        row->addWidget(lab(t, 14, true), 1);
        auto* chips = new FormChips; chips->setForm(m_hub->form(t, m_item.day, 5));
        row->addWidget(chips);
        v->addLayout(row);
    }
    v->addSpacing(6);
    v->addWidget(caption("Previous meetings"));
    auto all = m_hub->h2h(m_item.home(), m_item.away(), 16);
    const int cut = UserDataHub::serialOf(m_item.day);
    std::vector<sba::HistMatch> list;
    for (const auto& h : all) if (h.date < cut && list.size() < 6) list.push_back(h);
    if (list.empty()) { v->addWidget(lab("No earlier meetings in the data.", 13, false, "#5c5c5c")); return; }
    int hw = 0, aw = 0, dr = 0;
    auto* g = new QGridLayout; g->setHorizontalSpacing(10); g->setVerticalSpacing(0);
    int row = 0;
    for (const auto& h : list) {
        const QString H = QString::fromStdString(h.home), A = QString::fromStdString(h.away);
        if (h.ftr == 'D') ++dr; else if ((h.ftr == 'H') == (H == m_item.home())) ++hw; else ++aw;
        g->addWidget(rule(), row * 2, 0, 1, 4);
        auto* d = lab(QLocale::c().toString(UserDataHub::dateOf(h.date), "d MMM yy"), 12, false, "#8c8c8c"); d->setContentsMargins(0, 11, 0, 11);
        auto* hn = lab(H, 14); hn->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        auto* sc = lab(QString("%1 - %2").arg(h.fthg).arg(h.ftag), 14, true); sc->setAlignment(Qt::AlignCenter);
        auto* an = lab(A, 14);
        g->addWidget(d, row * 2 + 1, 0); g->addWidget(hn, row * 2 + 1, 1); g->addWidget(sc, row * 2 + 1, 2); g->addWidget(an, row * 2 + 1, 3);
        ++row;
    }
    g->setColumnStretch(0, 2); g->setColumnStretch(1, 5); g->setColumnStretch(2, 2); g->setColumnStretch(3, 5);
    v->addLayout(g);
    v->addWidget(lab(QString("Last %1: %2 %3 wins, %4 draws, %5 %6 wins.").arg(list.size()).arg(m_item.home()).arg(hw).arg(dr).arg(m_item.away()).arg(aw), 12, false, "#5c5c5c"));
}

// ---------------------------------------------------------------------------- Stats

void UserMatchPage::buildStats(QVBoxLayout* v) {
    const sba::HistMatch* h = m_hub->findHist(m_item);
    if (!h) { v->addWidget(lab("No match statistics available.", 13, false, "#5c5c5c")); return; }
    const char* names[3] = {"Shots", "Shots on target", "Corners"};
    bool any = false;
    for (int k = 0; k < 3; ++k) {
        if (!std::isfinite(h->stat[0][k]) || !std::isfinite(h->stat[1][k])) continue;
        v->addWidget(new StatBar(names[k], h->stat[0][k], h->stat[1][k]));
        any = true;
    }
    if (!any) v->addWidget(lab("No match statistics available.", 13, false, "#5c5c5c"));
}
