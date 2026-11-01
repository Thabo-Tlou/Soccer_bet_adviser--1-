#include "UserTheme.h"

namespace UT {

QString hex(const QColor& c) { return c.name(QColor::HexRgb); }

QFont font(int px, bool bold) {
    QFont f;
    f.setFamilies({"Segoe UI", "Roboto", "Inter", "Noto Sans", "DejaVu Sans"});
    f.setPixelSize(px);
    f.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    return f;
}

QString stylesheet() {
    QString s = R"(
* { font-family: "Segoe UI", "Roboto", "Inter", "Noto Sans", "DejaVu Sans", sans-serif; font-size: 13px; outline: 0; }
QMainWindow, QDialog { background: @page; }
QWidget { color: @black; }
QWidget#UserRoot, QStackedWidget { background: @page; }
QFrame#Col { background: @white; border-left: 1px solid @line; border-right: 1px solid @line; }
QFrame#Rule { background: @line; max-height: 1px; min-height: 1px; border: 0; }
QLabel { background: transparent; }
QToolTip { background: @black; color: @white; border: 0; padding: 4px 8px; }

QScrollArea { border: 0; background: transparent; }
QScrollArea > QWidget > QWidget { background: transparent; }
QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }
QScrollBar::handle:vertical { background: #cfcfcf; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #9a9a9a; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

QLineEdit, QComboBox, QDateEdit {
    background: @white; border: 1px solid #d6d6d6; border-radius: 4px; padding: 6px 9px; color: @black;
    selection-background-color: @black; selection-color: @white; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid @black; }
QComboBox::drop-down, QDateEdit::drop-down { border: 0; width: 22px; }
QComboBox::down-arrow, QDateEdit::down-arrow { image: url(:/icons/chevron-down-dark.svg); width: 12px; height: 12px; }
QComboBox QAbstractItemView { background: @white; border: 1px solid #d6d6d6; selection-background-color: @black; selection-color: @white; outline: 0; }
QCalendarWidget QWidget { background: @white; color: @black; }
QCalendarWidget QAbstractItemView:enabled { selection-background-color: @black; selection-color: @white; color: @black; }

QPushButton { background: @white; border: 1px solid #cfcfcf; border-radius: 4px; padding: 7px 16px; color: @black; font-weight: 600; }
QPushButton:hover { border-color: @black; }
QPushButton:disabled { color: #b5b5b5; border-color: #e3e3e3; }
QPushButton[ubtn="solid"] { background: @black; border: 1px solid @black; color: @white; }
QPushButton[ubtn="solid"]:hover { background: #262626; }
QPushButton[ubtn="solid"]:disabled { background: #d9d9d9; border-color: #d9d9d9; color: @white; }
QPushButton[ubtn="link"] { background: transparent; border: 0; padding: 4px 2px; color: @soft; font-weight: 600; }
QPushButton[ubtn="link"]:hover { color: @black; }
QPushButton[ubtn="darkghost"] { background: transparent; border: 1px solid #3a3a3a; color: #d8d8d8; padding: 5px 14px; }
QPushButton[ubtn="darkghost"]:hover { border-color: @greenOnDark; color: @white; }
)";
    const QPair<const char*, QColor> map[] = {
        {"@page", page}, {"@white", white}, {"@black", black}, {"@line", line}, {"@soft", soft}, {"@greenOnDark", greenOnDark}};
    for (const auto& m : map) s.replace(m.first, hex(m.second));
    return s;
}

}  // namespace UT
