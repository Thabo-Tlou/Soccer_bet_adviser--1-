#pragma once
#include <QDateTime>
#include <QString>
#include <cmath>

namespace Fmt {
inline QString pct(double v, int d = 1) { return std::isfinite(v) ? QString("%1%").arg(v * 100.0, 0, 'f', d) : QStringLiteral("-"); }
inline QString signedPct(double v, int d = 1) { return std::isfinite(v) ? QString("%1%2%").arg(v >= 0 ? "+" : "").arg(v * 100.0, 0, 'f', d) : QStringLiteral("-"); }
inline QString num(double v, int d = 2) { return std::isfinite(v) ? QString::number(v, 'f', d) : QStringLiteral("-"); }
inline QString kickoff(qint64 epoch) { return epoch > 0 ? QDateTime::fromSecsSinceEpoch(epoch).toLocalTime().toString("ddd d MMM  HH:mm") : QStringLiteral("-"); }
inline QString ago(qint64 epoch) {
    if (epoch <= 0) return QStringLiteral("never");
    const qint64 s = QDateTime::currentSecsSinceEpoch() - epoch;
    if (s < 60) return QStringLiteral("just now");
    if (s < 3600) return QString("%1 min ago").arg(s / 60);
    if (s < 86400) return QString("%1 h ago").arg(s / 3600);
    return QString("%1 d ago").arg(s / 86400);
}
inline QString html(const QString& s) { return s.toHtmlEscaped(); }
}  // namespace Fmt
