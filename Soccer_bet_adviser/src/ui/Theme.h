#pragma once
// Single source of truth for colours, fonts and the application style sheet.
// Palette = the original project colours (greens #064e3b/#14532d/#16a34a/#dcfce7,
// red #d93520, slate #0f172a-#e2e8f0) applied to a dark, IDE-style layout.
#include <QColor>
#include <QFont>
#include <QString>

namespace Theme {

inline const QColor bg0{"#0a0f0d"};        // window, activity bar
inline const QColor bg1{"#0f1613"};        // top bar, panels
inline const QColor bg2{"#131c18"};        // cards
inline const QColor bg3{"#1a2620"};        // inputs, hover
inline const QColor border{"#223029"};
inline const QColor borderStrong{"#2e4137"};
inline const QColor text{"#e6f0ea"};
inline const QColor muted{"#94a3b8"};      // slate-400
inline const QColor dim{"#64748b"};        // slate-500
inline const QColor primary{"#16a34a"};
inline const QColor primaryHover{"#15803d"};
inline const QColor deep{"#14532d"};
inline const QColor deeper{"#064e3b"};
inline const QColor tint{"#dcfce7"};
inline const QColor danger{"#d93520"};
inline const QColor warning{"#f59e0b"};

// Outcome colours used in all probability visuals.
inline const QColor homeColor{"#16a34a"};
inline const QColor drawColor{"#64748b"};
inline const QColor awayColor{"#dcfce7"};

QString hex(const QColor& c);
QString stylesheet();
QFont uiFont(int pointSize = 10, QFont::Weight w = QFont::Normal);
QFont monoFont(int pointSize = 10);
QColor withAlpha(const QColor& c, int alpha);

}  // namespace Theme
