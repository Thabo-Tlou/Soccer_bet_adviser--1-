#pragma once
// Central theme: colors, fonts, stylesheet.
#include <QColor>
#include <QFont>
#include <QString>

namespace Theme {

// Black surfaces
inline const QColor bg0{"#000000"};
inline const QColor bg1{"#0a0a0a"};
inline const QColor bg2{"#111111"};
inline const QColor bg3{"#1a1a1a"};
inline const QColor border{"#262626"};
inline const QColor borderStrong{"#3a3a3a"};

// Text
inline const QColor text{"#ffffff"};
inline const QColor muted{"#bfbfbf"};
inline const QColor dim{"#7f7f7f"};

// Greens
inline const QColor primary{"#16a34a"};
inline const QColor primaryHover{"#22c55e"};
inline const QColor deep{"#14532d"};
inline const QColor deeper{"#064e3b"};
inline const QColor tint{"#dcfce7"};

// Semantic aliases kept green
inline const QColor danger{"#16a34a"};
inline const QColor warning{"#22c55e"};

// Outcome colors for probability visuals
inline const QColor homeColor{"#16a34a"};
inline const QColor drawColor{"#7f7f7f"};
inline const QColor awayColor{"#ffffff"};

QString hex(const QColor& c);
QString stylesheet();
QFont uiFont(int pointSize = 10, QFont::Weight w = QFont::Normal);
QFont monoFont(int pointSize = 10);
QColor withAlpha(const QColor& c, int alpha);

}  // namespace Theme