#pragma once
// User-side look: black and white, with green used sparingly for live state, underlines and highlights.
// Layout and rhythm follow livescore.com (date strip, grouped match rows, text tabs with underline).
#include <QColor>
#include <QFont>
#include <QString>

namespace UT {

inline const QColor black{"#0b0b0b"};
inline const QColor white{"#ffffff"};
inline const QColor page{"#f2f2f2"};       // area outside the centre column
inline const QColor band{"#f6f6f6"};       // group headers
inline const QColor line{"#e6e6e6"};       // separators
inline const QColor hover{"#f4f9f5"};
inline const QColor muted{"#8c8c8c"};
inline const QColor soft{"#5c5c5c"};
inline const QColor green{"#16a34a"};      // on white
inline const QColor greenOnDark{"#2ecc71"};// on black
inline const QColor greenTint{"#e9f6ee"};

QFont font(int px, bool bold = false);
QString hex(const QColor& c);
QString stylesheet();

}  // namespace UT
