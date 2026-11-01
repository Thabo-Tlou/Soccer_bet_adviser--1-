#include "UserWidgets.h"
#include "UserTheme.h"
#include <QEnterEvent>
#include <QFontMetrics>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>

static QPolygonF starPolygon(const QPointF& c, double r) {
    QPolygonF poly;
    for (int i = 0; i < 10; ++i) {
        const double a = -M_PI / 2 + i * M_PI / 5, rr = (i % 2 == 0) ? r : r * 0.45;
        poly << QPointF(c.x() + rr * std::cos(a), c.y() + rr * std::sin(a));
    }
    return poly;
}

QFrame* makeColumn(QWidget* page, QVBoxLayout*& inner) {
    auto* outer = new QHBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto* col = new QFrame; col->setObjectName("Col"); col->setMaximumWidth(920); col->setMinimumWidth(640);
    col->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    inner = new QVBoxLayout(col); inner->setContentsMargins(0, 0, 0, 0); inner->setSpacing(0);
    outer->addStretch(1); outer->addWidget(col, 100); outer->addStretch(1);
    return col;
}

// ====================================================================== TextTabs

TextTabs::TextTabs(bool dark, QWidget* parent) : QWidget(parent), m_dark(dark) {
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(dark ? 52 : 44);
}

void TextTabs::setTabs(const QStringList& t) { m_tabs = t; if (m_cur >= t.size()) m_cur = 0; updateGeometry(); update(); }
void TextTabs::setLabel(int i, const QString& t) { if (i >= 0 && i < m_tabs.size() && m_tabs[i] != t) { m_tabs[i] = t; updateGeometry(); update(); } }
void TextTabs::setCurrent(int i, bool notify) {
    if (i < 0 || i >= m_tabs.size() || i == m_cur) return;
    m_cur = i; update();
    if (notify) emit changed(i);
}

QRect TextTabs::tabRect(int i) const {
    QFontMetrics fm(UT::font(13, true));
    int x = m_dark ? 0 : 8;
    for (int k = 0; k < m_tabs.size(); ++k) {
        const int w = fm.horizontalAdvance(m_tabs[k]) + 32;
        if (k == i) return QRect(x, 0, w, height());
        x += w;
    }
    return {};
}

QSize TextTabs::sizeHint() const {
    QFontMetrics fm(UT::font(13, true));
    int w = m_dark ? 0 : 16;
    for (const auto& t : m_tabs) w += fm.horizontalAdvance(t) + 32;
    return {w, height()};
}

int TextTabs::indexAt(int x) const {
    for (int i = 0; i < m_tabs.size(); ++i) if (tabRect(i).contains(x, 1)) return i;
    return -1;
}

void TextTabs::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QFontMetrics fm(UT::font(13, true));
    for (int i = 0; i < m_tabs.size(); ++i) {
        const QRect r = tabRect(i);
        const bool act = i == m_cur, hov = i == m_hover;
        QColor c = m_dark ? (act || hov ? UT::white : QColor("#9b9b9b")) : (act || hov ? UT::black : QColor("#777777"));
        p.setFont(UT::font(13, true));
        p.setPen(c);
        p.drawText(r, Qt::AlignCenter, m_tabs[i]);
        if (act) {
            const int tw = fm.horizontalAdvance(m_tabs[i]);
            p.fillRect(QRect(r.center().x() - tw / 2 - 2, height() - 3, tw + 4, 3), m_dark ? UT::greenOnDark : UT::green);
        }
    }
}

void TextTabs::mousePressEvent(QMouseEvent* e) {
    const int i = indexAt(e->position().toPoint().x());
    if (i >= 0) setCurrent(i, true);
}
void TextTabs::mouseMoveEvent(QMouseEvent* e) {
    const int i = indexAt(e->position().toPoint().x());
    if (i != m_hover) { m_hover = i; update(); }
}
void TextTabs::leaveEvent(QEvent*) { m_hover = -1; update(); }

// ====================================================================== DateStrip

DateStrip::DateStrip(QWidget* parent) : QWidget(parent), m_sel(QDate::currentDate()) {
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(58);
}

void DateStrip::setDate(const QDate& d, bool notify) {
    if (!d.isValid() || d == m_sel) return;
    m_sel = d; update();
    if (notify) emit dateChanged(d);
}

int DateStrip::cellAt(const QPoint& pt) const {
    const int arrow = 44, W = width();
    if (pt.x() < arrow) return -1;
    if (pt.x() >= W - arrow) return 7;
    const double cw = double(W - 2 * arrow) / 7.0;
    return std::min(6, int((pt.x() - arrow) / cw));
}

void DateStrip::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int arrow = 44, W = width(), H = height();
    const double cw = double(W - 2 * arrow) / 7.0;
    p.fillRect(rect(), UT::white);
    p.setPen(UT::line); p.drawLine(0, H - 1, W, H - 1);

    // arrows
    for (int side = 0; side < 2; ++side) {
        const bool hov = m_hover == (side == 0 ? -1 : 7);
        p.setPen(QPen(hov ? UT::black : QColor("#8c8c8c"), 1.8, Qt::SolidLine, Qt::RoundCap));
        const double cx = side == 0 ? arrow / 2.0 : W - arrow / 2.0, cy = H / 2.0, d = side == 0 ? -1 : 1;
        p.drawLine(QPointF(cx - 3 * d, cy - 6), QPointF(cx + 3 * d, cy));
        p.drawLine(QPointF(cx + 3 * d, cy), QPointF(cx - 3 * d, cy + 6));
    }
    const QDate today = QDate::currentDate();
    for (int i = 0; i < 7; ++i) {
        const QDate d = m_sel.addDays(i - 3);
        const QRectF cell(arrow + i * cw, 0, cw, H - 1);
        const bool sel = i == 3;
        if (i == m_hover && !sel) p.fillRect(cell, UT::hover);
        const QString top = d == today ? QStringLiteral("TODAY") : QLocale::c().toString(d, "ddd").toUpper();
        const QString bottom = QLocale::c().toString(d, "d MMM").toUpper();
        p.setFont(UT::font(11, true));
        p.setPen(sel ? UT::black : QColor("#9a9a9a"));
        p.drawText(QRectF(cell.x(), 11, cell.width(), 14), Qt::AlignCenter, top);
        p.setFont(UT::font(13, sel));
        p.setPen(sel ? UT::black : QColor("#666666"));
        p.drawText(QRectF(cell.x(), 27, cell.width(), 18), Qt::AlignCenter, bottom);
        if (sel) p.fillRect(QRectF(cell.x() + cell.width() * 0.2, H - 4, cell.width() * 0.6, 3), UT::green);
    }
}

void DateStrip::mousePressEvent(QMouseEvent* e) {
    const int c = cellAt(e->position().toPoint());
    if (c == -1) setDate(m_sel.addDays(-1), true);
    else if (c == 7) setDate(m_sel.addDays(1), true);
    else if (c >= 0) setDate(m_sel.addDays(c - 3), true);
}
void DateStrip::mouseMoveEvent(QMouseEvent* e) {
    const int c = cellAt(e->position().toPoint());
    if (c != m_hover) { m_hover = c; update(); }
}
void DateStrip::leaveEvent(QEvent*) { m_hover = -2; update(); }

// ====================================================================== SectionHeader

SectionHeader::SectionHeader(const QString& title, const QString& right, QWidget* parent) : QWidget(parent), m_title(title), m_right(right) {
    setFixedHeight(34);
}

void SectionHeader::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), UT::band);
    p.setPen(UT::line);
    p.drawLine(0, 0, width(), 0);
    p.drawLine(0, height() - 1, width(), height() - 1);
    p.setFont(UT::font(13, true));
    p.setPen(UT::black);
    p.drawText(QRect(16, 0, width() - 100, height()), Qt::AlignVCenter | Qt::AlignLeft, m_title);
    if (!m_right.isEmpty()) {
        p.setFont(UT::font(12));
        p.setPen(UT::muted);
        p.drawText(QRect(0, 0, width() - 16, height()), Qt::AlignVCenter | Qt::AlignRight, m_right);
    }
}

// ====================================================================== MatchRow

MatchRow::MatchRow(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(52);
}

QString MatchRow::statusText(const MatchItem& m) {
    const auto& f = m.fx;
    if (f.isLive()) return f.status == "PAUSED" ? QStringLiteral("HT") : (f.minute >= 0 ? QString("%1'").arg(f.minute) : QStringLiteral("LIVE"));
    if (f.isFinished()) return QStringLiteral("FT");
    if (f.status == "POSTPONED") return QStringLiteral("PP");
    if (f.status == "CANCELLED") return QStringLiteral("CAN");
    if (f.status == "SUSPENDED") return QStringLiteral("SUSP");
    if (f.kickoffEpoch > 0 && !m.fromDataset) return QDateTime::fromSecsSinceEpoch(f.kickoffEpoch).toLocalTime().toString("HH:mm");
    return QStringLiteral("-");
}

void MatchRow::setItem(const MatchItem& m, const PickInfo* pick, bool fav) {
    m_item = m; m_fav = fav;
    m_hasPick = pick && pick->ok && pick->a;
    m_value = m_hasPick && pick->a->reasoning.decision == sba::Decision::PossibleValue;
    m_best = m_hasPick ? pick->best : 0;
    m_p = m_hasPick ? pick->p : 0;
    update();
}

void MatchRow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = width(), H = height();
    p.fillRect(rect(), m_hover ? UT::hover : UT::white);
    p.setPen(UT::line); p.drawLine(0, H - 1, W, H - 1);
    const auto& f = m_item.fx;
    const bool live = f.isLive(), fin = f.isFinished();
    if (live) p.fillRect(0, 0, 3, H, UT::green);

    const int statusW = 72, starW = 44, pickW = 176;
    p.setFont(UT::font(12, live));
    p.setPen(live ? UT::green : (fin ? UT::muted : UT::black));
    p.drawText(QRect(0, 0, statusW, H), Qt::AlignCenter, statusText(m_item));

    const int midL = statusW, midR = std::max(midL + 300, W - pickW - starW), cx = (midL + midR) / 2, boxW = 76;
    // score
    if (m_item.hasScore() && (live || fin)) {
        p.setFont(UT::font(15, true));
        p.setPen(live ? UT::green : UT::black);
        p.drawText(QRect(cx - boxW / 2, 0, boxW, H), Qt::AlignCenter, QString("%1 - %2").arg(f.homeGoals).arg(f.awayGoals));
    } else {
        p.setFont(UT::font(14));
        p.setPen(UT::muted);
        p.drawText(QRect(cx - boxW / 2, 0, boxW, H), Qt::AlignCenter, QStringLiteral("-"));
    }
    // names
    p.setFont(UT::font(14));
    p.setPen(UT::black);
    QFontMetrics fm(p.font());
    const QRect hr(midL + 4, 0, cx - boxW / 2 - 14 - (midL + 4), H), ar(cx + boxW / 2 + 14, 0, midR - 4 - (cx + boxW / 2 + 14), H);
    p.drawText(hr, Qt::AlignVCenter | Qt::AlignRight, fm.elidedText(m_item.home(), Qt::ElideRight, hr.width()));
    p.drawText(ar, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(m_item.away(), Qt::ElideRight, ar.width()));

    // pick
    int x = W - starW;
    if (m_hasPick) {
        p.setFont(UT::font(13, true));
        p.setPen(UT::black);
        p.drawText(QRect(x - 46, 0, 42, H), Qt::AlignVCenter | Qt::AlignRight, pctText(m_p));
        const QRect chip(x - 46 - 30, (H - 24) / 2, 24, 24);
        p.setPen(Qt::NoPen); p.setBrush(UT::black);
        p.drawRoundedRect(chip, 4, 4);
        p.setPen(UT::white); p.setFont(UT::font(13, true));
        p.drawText(chip, Qt::AlignCenter, outcomeSymbol(m_best));
        if (m_value) {
            const QRect tag(chip.left() - 8 - 52, (H - 22) / 2, 52, 22);
            p.setPen(QPen(UT::green, 1.2)); p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(tag, 11, 11);
            p.setFont(UT::font(10, true)); p.setPen(UT::green);
            p.drawText(tag, Qt::AlignCenter, QStringLiteral("VALUE"));
        }
    }
    // star
    const QPointF sc(W - starW / 2.0 - 2, H / 2.0);
    p.setBrush(m_fav ? QBrush(UT::green) : Qt::NoBrush);
    p.setPen(QPen(m_fav ? UT::green : (m_overStar ? QColor("#555555") : QColor("#bdbdbd")), 1.4));
    p.drawPolygon(starPolygon(sc, 8.5));
}

void MatchRow::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    if (e->position().x() >= width() - 44) emit favToggled(); else emit clicked();
}
void MatchRow::mouseMoveEvent(QMouseEvent* e) {
    const bool s = e->position().x() >= width() - 44;
    if (s != m_overStar) { m_overStar = s; update(); }
}
void MatchRow::enterEvent(QEnterEvent*) { m_hover = true; update(); }
void MatchRow::leaveEvent(QEvent*) { m_hover = false; m_overStar = false; update(); }

// ====================================================================== ProbRows

ProbRows::ProbRows(QWidget* parent) : QWidget(parent) { setMinimumHeight(150); }

void ProbRows::set(const QString labels[3], const sba::Probs3& model, bool hasMarket, const sba::Probs3& market) {
    for (int i = 0; i < 3; ++i) m_labels[i] = labels[i];
    m_model = model; m_market = market; m_hasMarket = hasMarket;
    update();
}

void ProbRows::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = width(), labelW = 170, rightW = m_hasMarket ? 150 : 70, barL = labelW + 8, barR = W - rightW;
    int best = 0;
    for (int i = 1; i < 3; ++i) if (m_model[i] > m_model[best]) best = i;
    p.setFont(UT::font(10, true)); p.setPen(UT::muted);
    p.drawText(QRect(W - 112, 0, 60, 20), Qt::AlignVCenter | Qt::AlignRight, QStringLiteral("MODEL"));
    if (m_hasMarket) p.drawText(QRect(W - 54, 0, 54, 20), Qt::AlignVCenter | Qt::AlignRight, QStringLiteral("MARKET"));
    for (int i = 0; i < 3; ++i) {
        const int y = 24 + i * 40;
        p.setFont(UT::font(14, i == best)); p.setPen(UT::black);
        QFontMetrics fm(p.font());
        p.drawText(QRect(0, y, labelW, 32), Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(m_labels[i], Qt::ElideRight, labelW));
        const QRectF track(barL, y + 11, std::max(10, barR - barL), 10);
        p.setPen(Qt::NoPen); p.setBrush(QColor("#ececec")); p.drawRoundedRect(track, 5, 5);
        p.setBrush(i == best ? UT::green : QColor("#bdbdbd"));
        p.drawRoundedRect(QRectF(track.x(), track.y(), std::max(6.0, track.width() * m_model[i]), track.height()), 5, 5);
        p.setFont(UT::font(15, true)); p.setPen(UT::black);
        p.drawText(QRect(W - 112, y, 60, 32), Qt::AlignVCenter | Qt::AlignRight, pctText(m_model[i]));
        if (m_hasMarket) {
            p.setFont(UT::font(13)); p.setPen(UT::muted);
            p.drawText(QRect(W - 54, y, 54, 32), Qt::AlignVCenter | Qt::AlignRight, pctText(m_market[i]));
        }
    }
}

// ====================================================================== FormChips

FormChips::FormChips(QWidget* parent) : QWidget(parent) { setFixedHeight(26); }
void FormChips::setForm(const QString& f) { m_form = f; updateGeometry(); update(); }
QSize FormChips::sizeHint() const { return {std::max(1, int(m_form.size())) * 28, 26}; }

void FormChips::paintChips(QPainter& p, const QString& form, int x, int y, int size, int gap) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(UT::font(std::max(9, size * 5 / 9), true));
    for (int i = 0; i < form.size(); ++i) {
        const QRect r(x + i * (size + gap), y, size, size);
        const QChar c = form[i];
        p.setPen(Qt::NoPen);
        p.setBrush(c == 'W' ? UT::green : (c == 'D' ? QColor("#b4b4b4") : UT::black));
        p.drawRoundedRect(r, 3, 3);
        p.setPen(UT::white);
        p.drawText(r, Qt::AlignCenter, QString(c));
    }
    p.restore();
}

void FormChips::paintEvent(QPaintEvent*) {
    QPainter p(this);
    paintChips(p, m_form, 0, 1, 24, 4);
}

// ====================================================================== team badge

void paintTeamBadge(QPainter& p, const QPointF& c, double r, const QString& team, bool dark) {
    QStringList w = team.split(' ', Qt::SkipEmptyParts);
    QString code;
    if (w.size() >= 2) code = w[0].left(1) + w[1].left(2);
    else code = team.left(3);
    code = code.toUpper();
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(dark ? QColor("#454545") : QColor("#dcdcdc"), 1.4));
    p.setBrush(dark ? QColor("#161616") : UT::white);
    p.drawEllipse(c, r, r);
    p.setFont(UT::font(int(r * 0.62), true));
    p.setPen(dark ? UT::white : UT::black);
    p.drawText(QRectF(c.x() - r, c.y() - r, 2 * r, 2 * r), Qt::AlignCenter, code);
    p.restore();
}

// ====================================================================== StandingsWidget

StandingsWidget::StandingsWidget(QWidget* parent) : QWidget(parent) {}

void StandingsWidget::setRows(const std::vector<TableRow>& r) {
    m_rows = r;
    setFixedHeight(32 + 42 * static_cast<int>(std::max<size_t>(1, r.size())));
    update();
}

void StandingsWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = width();
    const int formW = 5 * 16 + 4 * 3, colW = 36, gdW = 48, ptsW = 52;
    const int xForm = W - 16 - formW, xPts = xForm - 14 - ptsW, xGd = xPts - gdW, xL = xGd - colW, xD = xL - colW, xW = xD - colW, xP = xW - colW;
    p.fillRect(rect(), UT::white);
    // header
    p.fillRect(0, 0, W, 32, UT::band);
    p.setPen(UT::line); p.drawLine(0, 31, W, 31);
    p.setFont(UT::font(11, true)); p.setPen(UT::muted);
    p.drawText(QRect(0, 0, 48, 32), Qt::AlignCenter, "#");
    p.drawText(QRect(56, 0, 200, 32), Qt::AlignVCenter | Qt::AlignLeft, "TEAM");
    const struct { int x, w; const char* t; } heads[] = {{xP, colW, "P"}, {xW, colW, "W"}, {xD, colW, "D"}, {xL, colW, "L"}, {xGd, gdW, "GD"}, {xPts, ptsW, "PTS"}};
    for (const auto& h : heads) p.drawText(QRect(h.x, 0, h.w, 32), Qt::AlignCenter, h.t);
    p.drawText(QRect(xForm, 0, formW, 32), Qt::AlignCenter, "FORM");

    const int n = static_cast<int>(m_rows.size());
    for (int i = 0; i < n; ++i) {
        const auto& r = m_rows[static_cast<size_t>(i)];
        const int y = 32 + i * 42;
        p.setPen(UT::line); p.drawLine(0, y + 41, W, y + 41);
        if (i < 4) p.fillRect(0, y, 4, 41, UT::green);
        else if (n >= 12 && i >= n - 3) p.fillRect(0, y, 4, 41, UT::black);
        p.setFont(UT::font(13)); p.setPen(UT::black);
        p.drawText(QRect(0, y, 48, 41), Qt::AlignCenter, QString::number(i + 1));
        p.setFont(UT::font(14));
        QFontMetrics fm(p.font());
        p.drawText(QRect(56, y, xP - 66, 41), Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(r.team, Qt::ElideRight, xP - 66));
        p.setFont(UT::font(13)); p.setPen(UT::black);
        const QString cells[] = {QString::number(r.played), QString::number(r.won), QString::number(r.drawn), QString::number(r.lost),
                                 (r.gd() > 0 ? "+" : "") + QString::number(r.gd())};
        const int xs[] = {xP, xW, xD, xL, xGd}, ws[] = {colW, colW, colW, colW, gdW};
        for (int k = 0; k < 5; ++k) p.drawText(QRect(xs[k], y, ws[k], 41), Qt::AlignCenter, cells[k]);
        p.setFont(UT::font(14, true));
        p.drawText(QRect(xPts, y, ptsW, 41), Qt::AlignCenter, QString::number(r.points));
        FormChips::paintChips(p, r.form, xForm + (formW - (int(r.form.size()) * 16 + std::max(0, int(r.form.size()) - 1) * 3)), y + 13, 16, 3);
    }
}

// ====================================================================== PickRowWidget

PickRowWidget::PickRowWidget(QWidget* parent) : QWidget(parent) { setFixedHeight(58); }

void PickRowWidget::set(const QString& date, const QString& teams, const QString& pick, State s, const QString& result) {
    m_date = date; m_teams = teams; m_pick = pick; m_state = s; m_result = result;
    update();
}

void PickRowWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = width(), H = height();
    p.fillRect(rect(), UT::white);
    p.setPen(UT::line); p.drawLine(0, H - 1, W, H - 1);
    p.setFont(UT::font(12)); p.setPen(UT::muted);
    p.drawText(QRect(16, 0, 70, H), Qt::AlignVCenter | Qt::AlignLeft, m_date);
    const int chipW = 78, textR = W - chipW - 28;
    p.setFont(UT::font(14, true)); p.setPen(UT::black);
    QFontMetrics fm(p.font());
    p.drawText(QRect(92, 8, textR - 92, 20), Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(m_teams, Qt::ElideRight, textR - 92));
    p.setFont(UT::font(12)); p.setPen(UT::soft);
    QFontMetrics fm2(p.font());
    p.drawText(QRect(92, 29, textR - 92, 18), Qt::AlignVCenter | Qt::AlignLeft, fm2.elidedText(m_pick, Qt::ElideRight, textR - 92));
    // state chip
    const QRect chip(W - 16 - chipW, (H - 24) / 2, chipW, 24);
    p.setFont(UT::font(11, true));
    if (m_state == Pending) { p.setPen(QPen(QColor("#c4c4c4"), 1)); p.setBrush(Qt::NoBrush); p.drawRoundedRect(chip, 12, 12); p.setPen(UT::muted); }
    else { p.setPen(Qt::NoPen); p.setBrush(m_state == Won ? UT::green : UT::black); p.drawRoundedRect(chip, 12, 12); p.setPen(UT::white); }
    p.drawText(chip, Qt::AlignCenter, m_state == Pending ? "PENDING" : (m_state == Won ? "CORRECT" : "MISSED"));
    if (!m_result.isEmpty()) {
        p.setFont(UT::font(12)); p.setPen(UT::muted);
        p.drawText(QRect(chip.left() - 150, 0, 140, H), Qt::AlignVCenter | Qt::AlignRight, m_result);
    }
}
