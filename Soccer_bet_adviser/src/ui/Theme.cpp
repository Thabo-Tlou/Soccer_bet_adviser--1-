#include "Theme.h"

namespace Theme {

QString hex(const QColor& c) { return c.name(QColor::HexRgb); }
QColor withAlpha(const QColor& c, int a) { QColor x = c; x.setAlpha(a); return x; }

QFont uiFont(int pt, QFont::Weight w) {
    QFont f("Segoe UI");
    f.setStyleHint(QFont::SansSerif);
    f.setPointSize(pt);
    f.setWeight(w);
    return f;
}

QFont monoFont(int pt) {
    QFont f("Cascadia Mono");
    f.setStyleHint(QFont::Monospace);
    f.setFamilies({"Cascadia Mono", "Consolas", "DejaVu Sans Mono", "monospace"});
    f.setPointSize(pt);
    return f;
}

QString stylesheet() {
    QString s = R"(
* { font-family: "Segoe UI", "Inter", "Noto Sans", sans-serif; font-size: 10pt; outline: 0; }
QMainWindow, QDialog { background: @bg0; }
QWidget { color: @text; }
QWidget#Page, QStackedWidget { background: @bg0; }
QWidget#TopBar { background: @bg1; border-bottom: 1px solid @border; }
QWidget#ActivityBar { background: @bg0; border-right: 1px solid @border; }
QWidget#StatusBar { background: @deeper; }
QWidget#StatusBar QLabel { color: @tint; font-size: 9pt; }
QToolTip { background: @bg3; color: @text; border: 1px solid @borderStrong; padding: 4px 6px; }

QFrame#Card { background: @bg2; border: 1px solid @border; border-radius: 8px; }
QFrame#Card QLabel { background: transparent; }
QFrame#Banner { border-radius: 6px; border: 1px solid @border; }
QFrame#Divider { background: @border; max-height: 1px; min-height: 1px; border: 0; }

QLabel[role="h1"] { font-size: 17pt; font-weight: 600; color: @text; }
QLabel[role="h2"] { font-size: 11pt; font-weight: 600; color: @text; }
QLabel[role="muted"] { color: @muted; }
QLabel[role="dim"] { color: @dim; font-size: 9pt; }
QLabel[role="kpi"] { font-size: 20pt; font-weight: 600; color: @text; }
QLabel[role="mono"] { font-family: "Cascadia Mono", "Consolas", "DejaVu Sans Mono", monospace; color: @muted; }
QLabel[role="caption"] { color: @dim; font-size: 8pt; font-weight: 600; letter-spacing: 1px; }

QPushButton { background: @bg3; border: 1px solid @borderStrong; border-radius: 6px; padding: 6px 14px; color: @text; }
QPushButton:hover { background: #22332b; border-color: @primary; }
QPushButton:pressed { background: @deep; }
QPushButton:disabled { color: @dim; background: @bg1; border-color: @border; }
QPushButton[primary="true"] { background: @primary; border: 1px solid @primary; color: white; font-weight: 600; }
QPushButton[primary="true"]:hover { background: @primaryHover; border-color: @primaryHover; }
QPushButton[primary="true"]:disabled { background: @deep; border-color: @deep; color: #7aa890; }
QPushButton[flat="true"] { background: transparent; border: 1px solid transparent; color: @muted; }
QPushButton[flat="true"]:hover { color: @text; background: @bg3; }

QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox, QDateEdit, QPlainTextEdit, QTextBrowser, QTextEdit {
    background: @bg1; border: 1px solid @borderStrong; border-radius: 6px; padding: 5px 8px; color: @text;
    selection-background-color: @deep; selection-color: @tint; }
QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus { border: 1px solid @primary; }
QLineEdit:disabled, QComboBox:disabled, QDoubleSpinBox:disabled { color: @dim; background: @bg0; }
QTextBrowser, QPlainTextEdit { border: 1px solid @border; }
QComboBox::drop-down { border: 0; width: 24px; }
QComboBox::down-arrow { image: url(:/icons/chevron-down.svg); width: 12px; height: 12px; }
QComboBox QAbstractItemView { background: @bg2; border: 1px solid @borderStrong; selection-background-color: @deep; selection-color: @tint; outline: 0; padding: 2px; }
QSpinBox::up-button, QSpinBox::down-button, QDoubleSpinBox::up-button, QDoubleSpinBox::down-button, QDateEdit::up-button, QDateEdit::down-button { width: 0; border: 0; }
QDateEdit::drop-down { border: 0; width: 22px; }
QDateEdit::down-arrow { image: url(:/icons/chevron-down.svg); width: 12px; height: 12px; }

QCheckBox { spacing: 8px; color: @text; }
QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid @borderStrong; border-radius: 4px; background: @bg1; }
QCheckBox::indicator:hover { border-color: @primary; }
QCheckBox::indicator:checked { background: @primary; border-color: @primary; image: url(:/icons/check.svg); }

QTableWidget, QTableView, QListWidget, QTreeWidget { background: @bg2; alternate-background-color: #111a16; border: 1px solid @border; border-radius: 8px;
    gridline-color: transparent; selection-background-color: @deep; selection-color: @tint; }
QTableWidget::item, QListWidget::item { padding: 6px 8px; border: 0; }
QTableWidget::item:hover, QListWidget::item:hover { background: @bg3; }
QTableWidget::item:selected, QListWidget::item:selected { background: @deep; color: @tint; }
QHeaderView { background: transparent; }
QHeaderView::section { background: @bg1; color: @muted; border: 0; border-bottom: 1px solid @border; padding: 7px 8px; font-size: 9pt; font-weight: 600; }
QTableCornerButton::section { background: @bg1; border: 0; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: @borderStrong; border-radius: 4px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: @dim; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:horizontal { background: @borderStrong; border-radius: 4px; min-width: 28px; }
QScrollBar::handle:horizontal:hover { background: @dim; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QScrollArea { border: 0; background: transparent; }
QSplitter::handle { background: @border; }
QProgressBar { background: @bg1; border: 1px solid @border; border-radius: 4px; height: 6px; text-align: center; color: transparent; }
QProgressBar::chunk { background: @primary; border-radius: 3px; }
QMenu { background: @bg2; border: 1px solid @borderStrong; padding: 4px; }
QMenu::item { padding: 6px 18px; border-radius: 4px; }
QMenu::item:selected { background: @deep; color: @tint; }
QSlider::groove:horizontal { height: 4px; background: @borderStrong; border-radius: 2px; }
QSlider::sub-page:horizontal { background: @primary; border-radius: 2px; }
QSlider::handle:horizontal { background: @tint; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }
)";
    const QPair<const char*, QColor> map[] = {
        {"@bg0", bg0}, {"@bg1", bg1}, {"@bg2", bg2}, {"@bg3", bg3}, {"@borderStrong", borderStrong}, {"@border", border},
        {"@text", text}, {"@muted", muted}, {"@dim", dim}, {"@primaryHover", primaryHover}, {"@primary", primary},
        {"@deeper", deeper}, {"@deep", deep}, {"@tint", tint}, {"@danger", danger}, {"@warning", warning}};
    for (const auto& m : map) s.replace(m.first, hex(m.second));
    return s;
}

}  // namespace Theme
