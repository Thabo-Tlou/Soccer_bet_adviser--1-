#include "Widgets.h"
#include "Theme.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------- helpers
QLabel* makeLabel(const QString& text, const char* role, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setProperty("role", role);
    l->setWordWrap(false);
    return l;
}

QPushButton* makeButton(const QString& text, bool primary, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    if (primary) b->setProperty("primary", true);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QFrame* makeDivider(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("Divider");
    f->setFixedHeight(1);
    return f;
}

// ---------------------------------------------------------------- Card
Card::Card(const QString& title, QWidget* parent) : QFrame(parent) {
    setObjectName("Card");
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 14, 16, 16);
    outer->setSpacing(10);
    m_header = new QHBoxLayout;
    m_title = makeLabel(title, "h2");
    m_header->addWidget(m_title);
    m_header->addStretch();
    outer->addLayout(m_header);
    m_body = new QVBoxLayout;
    m_body->setSpacing(8);
    outer->addLayout(m_body, 1);
    m_title->setVisible(!title.isEmpty());
}

void Card::setTitle(const QString& t) { m_title->setText(t); m_title->setVisible(!t.isEmpty()); }

// ---------------------------------------------------------------- Badge
Badge::Badge(const QString& text, Kind k, QWidget* parent) : QLabel(text, parent) {
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setFixedHeight(22);
    setKind(k);
}

void Badge::setKind(Kind k) {
    QColor fg, bg;
    switch (k) {
        case Success: fg = Theme::tint; bg = Theme::withAlpha(Theme::primary, 70); break;
        case Warning: fg = QColor("#fcd34d"); bg = Theme::withAlpha(Theme::warning, 55); break;
        case Danger:  fg = QColor("#fca5a5"); bg = Theme::withAlpha(Theme::danger, 70); break;
        case Info:    fg = Theme::tint; bg = Theme::withAlpha(Theme::deeper, 255); break;
        default:      fg = Theme::muted; bg = Theme::withAlpha(Theme::dim, 55); break;
    }
    setStyleSheet(QString("QLabel { color: %1; background: rgba(%2,%3,%4,%5); border-radius: 9px; padding: 2px 9px; font-size: 8.5pt; font-weight: 600; }")
                      .arg(Theme::hex(fg)).arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha() / 255.0, 0, 'f', 2));
}

// ---------------------------------------------------------------- StatTile
StatTile::StatTile(const QString& caption, QWidget* parent) : Card(QString(), parent) {
    body()->setSpacing(2);
    body()->addWidget(makeLabel(caption.toUpper(), "caption"));
    m_value = makeLabel("-", "kpi");
    m_sub = makeLabel("", "dim");
    body()->addWidget(m_value);
    body()->addWidget(m_sub);
}

void StatTile::set(const QString& value, const QString& sub) { m_value->setText(value); m_sub->setText(sub); }

// ---------------------------------------------------------------- Banner
Banner::Banner(QWidget* parent) : QFrame(parent) {
    setObjectName("Banner");
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(12, 8, 12, 8);
    l->setSpacing(10);
    m_dot = new QLabel(QString::fromUtf8("\xE2\x97\x8F"));
    m_text = new QLabel;
    m_text->setWordWrap(true);
    l->addWidget(m_dot, 0, Qt::AlignTop);
    l->addWidget(m_text, 1);
    hide();
}

void Banner::show(Kind k, const QString& text) {
    QColor c = Theme::primary, bg = Theme::withAlpha(Theme::primary, 28);
    if (k == Warning) { c = Theme::warning; bg = Theme::withAlpha(Theme::warning, 28); }
    if (k == Error) { c = Theme::danger; bg = Theme::withAlpha(Theme::danger, 34); }
    if (k == Info) { c = Theme::muted; bg = Theme::withAlpha(Theme::dim, 30); }
    setStyleSheet(QString("QFrame#Banner { background: rgba(%1,%2,%3,%4); border: 1px solid %5; border-radius: 6px; } QLabel { background: transparent; }")
                      .arg(bg.red()).arg(bg.green()).arg(bg.blue()).arg(bg.alpha() / 255.0, 0, 'f', 2).arg(Theme::hex(Theme::withAlpha(c, 120))));
    m_dot->setStyleSheet(QString("color: %1;").arg(Theme::hex(c)));
    m_text->setText(text);
    setVisible(!text.isEmpty());
}

// ---------------------------------------------------------------- ProbBar
ProbBar::ProbBar(QWidget* parent) : QWidget(parent) { setMinimumHeight(24); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); }

void ProbBar::setProbs(const sba::Probs3& p, bool valid) { m_p = p; m_valid = valid; update(); }
void ProbBar::setMarket(const sba::Probs3& m, bool valid) { m_m = m; m_hasMarket = valid; update(); }

void ProbBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, m_hasMarket ? -6.5 : -0.5);
    QPainterPath clip; clip.addRoundedRect(r, 5, 5);
    if (!m_valid) {
        p.setPen(Theme::border); p.setBrush(Theme::bg3); p.drawPath(clip);
        p.setPen(Theme::dim); p.drawText(r, Qt::AlignCenter, "no model output");
        return;
    }
    p.setClipPath(clip);
    const QColor cols[3] = {Theme::homeColor, Theme::drawColor, Theme::awayColor};
    double x = r.left();
    p.setFont(Theme::uiFont(8, QFont::DemiBold));
    for (int i = 0; i < 3; ++i) {
        const double w = r.width() * m_p[i];
        p.fillRect(QRectF(x, r.top(), w, r.height()), cols[i]);
        if (w > 34) {
            p.setPen(i == 2 ? QColor("#052e16") : Qt::white);
            p.drawText(QRectF(x, r.top(), w, r.height()), Qt::AlignCenter, QString("%1%").arg(m_p[i] * 100, 0, 'f', 0));
        }
        x += w;
    }
    p.setClipping(false);
    if (m_hasMarket) {
        p.setPen(QPen(Theme::tint, 2));
        double c = 0;
        for (int i = 0; i < 2; ++i) {
            c += m_m[i];
            const double mx = r.left() + r.width() * c;
            p.drawLine(QPointF(mx, r.bottom() + 1), QPointF(mx, r.bottom() + 5));
        }
    }
}

// ---------------------------------------------------------------- OutcomeBar
OutcomeBar::OutcomeBar(const QString& label, const QColor& color, QWidget* parent) : QWidget(parent), m_label(label), m_color(color) {
    setMinimumHeight(34);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void OutcomeBar::setValues(double model, double market) { m_model = model; m_market = market; update(); }

void OutcomeBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(Theme::uiFont(9, QFont::DemiBold));
    const int labelW = 78;
    p.setPen(Theme::muted);
    p.drawText(QRect(0, 0, labelW, height()), Qt::AlignVCenter | Qt::AlignLeft, m_label);
    const QRectF track(labelW, height() / 2.0 - 7, width() - labelW - 60, 14);
    p.setPen(Qt::NoPen); p.setBrush(Theme::bg1); p.drawRoundedRect(track, 7, 7);
    const double v = std::clamp(m_model, 0.0, 1.0);
    if (v > 0) { p.setBrush(m_color); p.drawRoundedRect(QRectF(track.left(), track.top(), std::max(14.0, track.width() * v), track.height()), 7, 7); }
    if (std::isfinite(m_market)) {
        const double mx = track.left() + track.width() * std::clamp(m_market, 0.0, 1.0);
        p.setPen(QPen(Theme::text, 2)); p.drawLine(QPointF(mx, track.top() - 4), QPointF(mx, track.bottom() + 4));
    }
    p.setPen(Theme::text);
    p.drawText(QRect(width() - 56, 0, 56, height()), Qt::AlignVCenter | Qt::AlignRight, QString("%1%").arg(m_model * 100, 0, 'f', 1));
}

// ---------------------------------------------------------------- CalibrationChart
CalibrationChart::CalibrationChart(QWidget* parent) : QWidget(parent) { setMinimumSize(300, 260); }
void CalibrationChart::setBins(const std::vector<sba::CalibrationBin>& b) { m_bins = b; update(); }

void CalibrationChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF plot(44, 12, width() - 60, height() - 46);
    p.setPen(Theme::border);
    for (int i = 0; i <= 5; ++i) {
        const double t = i / 5.0;
        p.drawLine(QPointF(plot.left(), plot.bottom() - plot.height() * t), QPointF(plot.right(), plot.bottom() - plot.height() * t));
        p.drawLine(QPointF(plot.left() + plot.width() * t, plot.top()), QPointF(plot.left() + plot.width() * t, plot.bottom()));
    }
    p.setFont(Theme::uiFont(8)); p.setPen(Theme::dim);
    for (int i = 0; i <= 5; ++i) {
        const double t = i / 5.0;
        p.drawText(QRectF(0, plot.bottom() - plot.height() * t - 8, 38, 16), Qt::AlignRight | Qt::AlignVCenter, QString("%1%").arg(i * 20));
        p.drawText(QRectF(plot.left() + plot.width() * t - 20, plot.bottom() + 4, 40, 14), Qt::AlignCenter, QString("%1%").arg(i * 20));
    }
    p.drawText(QRectF(plot.left(), height() - 18, plot.width(), 16), Qt::AlignCenter, "predicted probability");
    p.setPen(QPen(Theme::dim, 1, Qt::DashLine));
    p.drawLine(plot.bottomLeft(), plot.topRight());   // perfect calibration

    int maxCount = 1;
    for (const auto& b : m_bins) maxCount = std::max(maxCount, b.count);
    QPolygonF line;
    for (const auto& b : m_bins) {
        if (b.count < 5) continue;
        line << QPointF(plot.left() + plot.width() * b.meanPredicted, plot.bottom() - plot.height() * b.observed);
    }
    p.setPen(QPen(Theme::primary, 2)); p.drawPolyline(line);
    for (const auto& b : m_bins) {
        if (b.count < 5) continue;
        const QPointF c(plot.left() + plot.width() * b.meanPredicted, plot.bottom() - plot.height() * b.observed);
        const double r = 3.5 + 6.0 * std::sqrt(static_cast<double>(b.count) / maxCount);
        p.setPen(QPen(Theme::bg2, 1.5)); p.setBrush(Theme::tint); p.drawEllipse(c, r, r);
    }
    p.save(); p.translate(12, plot.center().y()); p.rotate(-90); p.setPen(Theme::dim);
    p.drawText(QRectF(-80, -10, 160, 14), Qt::AlignCenter, "observed frequency"); p.restore();
}

// ---------------------------------------------------------------- ConfusionWidget
ConfusionWidget::ConfusionWidget(QWidget* parent) : QWidget(parent) { setMinimumSize(280, 220); }
void ConfusionWidget::setMatrix(const sba::ConfusionMatrix& m) { m_m = m; update(); }

void ConfusionWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const char* names[3] = {"Home", "Draw", "Away"};
    const double left = 96, top = 28, cw = (width() - left - 8) / 3.0, ch = (height() - top - 26) / 3.0;
    p.setFont(Theme::uiFont(8, QFont::DemiBold)); p.setPen(Theme::muted);
    for (int i = 0; i < 3; ++i) {
        p.drawText(QRectF(left + i * cw, 4, cw, 20), Qt::AlignCenter, QString("pred %1").arg(names[i]));
        p.drawText(QRectF(0, top + i * ch, left - 6, ch), Qt::AlignRight | Qt::AlignVCenter, QString("actual %1").arg(names[i]));
    }
    for (int a = 0; a < 3; ++a) {
        int rowSum = 0; for (int q = 0; q < 3; ++q) rowSum += m_m.m[a][q];
        for (int q = 0; q < 3; ++q) {
            const double share = rowSum ? static_cast<double>(m_m.m[a][q]) / rowSum : 0.0;
            QColor c = (a == q) ? Theme::primary : Theme::danger;
            c.setAlphaF(0.10 + 0.75 * share);
            const QRectF cell(left + q * cw + 2, top + a * ch + 2, cw - 4, ch - 4);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawRoundedRect(cell, 6, 6);
            p.setPen(Theme::text); p.setFont(Theme::uiFont(11, QFont::DemiBold));
            p.drawText(cell, Qt::AlignCenter, QString::number(m_m.m[a][q]));
            p.setFont(Theme::uiFont(8)); p.setPen(Theme::muted);
            p.drawText(cell.adjusted(0, 0, 0, -4), Qt::AlignHCenter | Qt::AlignBottom, QString("%1%").arg(share * 100, 0, 'f', 0));
        }
    }
}

// ---------------------------------------------------------------- NavButton
NavButton::NavButton(Icon icon, const QString& tip, QWidget* parent) : QPushButton(parent), m_icon(icon) {
    setCheckable(true);
    setToolTip(tip);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(52, 48);
    setFlat(true);
}

void NavButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool on = isChecked(), hov = underMouse();
    if (hov && !on) p.fillRect(rect(), Theme::bg1);
    if (on) {
        p.fillRect(QRect(0, 0, 2, height()), Theme::primary);
        p.fillRect(rect().adjusted(2, 0, 0, 0), Theme::withAlpha(Theme::primary, 22));
    }
    const QColor c = on ? Theme::tint : (hov ? Theme::text : Theme::dim);
    p.setPen(QPen(c, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.translate(width() / 2.0 - 11, height() / 2.0 - 11);   // 22x22 icon box
    switch (m_icon) {
        case Dashboard:
            p.drawRoundedRect(QRectF(2, 2, 8, 8), 2, 2); p.drawRoundedRect(QRectF(12, 2, 8, 5), 2, 2);
            p.drawRoundedRect(QRectF(2, 12, 8, 5), 2, 2); p.drawRoundedRect(QRectF(12, 9, 8, 11), 2, 2); break;
        case Matches:   // pitch
            p.drawRoundedRect(QRectF(2, 4, 18, 14), 2, 2); p.drawLine(QPointF(11, 4), QPointF(11, 18)); p.drawEllipse(QPointF(11, 11), 3, 3); break;
        case Analysis: {  // chart with spark
            p.drawLine(QPointF(3, 3), QPointF(3, 19)); p.drawLine(QPointF(3, 19), QPointF(19, 19));
            QPolygonF l; l << QPointF(6, 15) << QPointF(10, 10) << QPointF(13, 13) << QPointF(18, 5); p.drawPolyline(l); break; }
        case Performance:
            p.drawRect(QRectF(3, 11, 3.5, 8)); p.drawRect(QRectF(9.2, 6, 3.5, 13)); p.drawRect(QRectF(15.4, 2, 3.5, 17)); break;
        case Search:
            p.drawEllipse(QPointF(9.5, 9.5), 6, 6); p.drawLine(QPointF(14, 14), QPointF(19.5, 19.5)); break;
        case Settings:
            p.drawLine(QPointF(3, 6), QPointF(19, 6)); p.drawLine(QPointF(3, 11), QPointF(19, 11)); p.drawLine(QPointF(3, 16), QPointF(19, 16));
            p.setBrush(Theme::bg0); p.drawEllipse(QPointF(7, 6), 2.2, 2.2); p.drawEllipse(QPointF(14, 11), 2.2, 2.2); p.drawEllipse(QPointF(9, 16), 2.2, 2.2); break;
    }
}
