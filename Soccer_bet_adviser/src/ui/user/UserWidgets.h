#pragma once
// Custom-painted building blocks of the user side (all black / white, green for live and highlights).
#include <QWidget>
#include <vector>
#include "UserCommon.h"
#include "UserData.h"

class QFrame; class QVBoxLayout;
// White centre column on a grey page (max 920 px wide); returns the column and its layout.
QFrame* makeColumn(QWidget* page, QVBoxLayout*& inner);

// Text tabs with a green underline on the active one (also used, in dark mode, for the main navigation).
class TextTabs : public QWidget {
    Q_OBJECT
public:
    explicit TextTabs(bool dark = false, QWidget* parent = nullptr);
    void setTabs(const QStringList& t);
    void setLabel(int i, const QString& t);
    int current() const { return m_cur; }
    void setCurrent(int i, bool notify = false);
    QSize sizeHint() const override;
signals:
    void changed(int index);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
private:
    int indexAt(int x) const;
    QRect tabRect(int i) const;
    QStringList m_tabs;
    int m_cur = 0, m_hover = -1;
    bool m_dark;
};

// Seven-day strip centred on the selected day, like the livescore date selector.
class DateStrip : public QWidget {
    Q_OBJECT
public:
    explicit DateStrip(QWidget* parent = nullptr);
    QDate date() const { return m_sel; }
    void setDate(const QDate& d, bool notify = false);
    QSize sizeHint() const override { return {600, 58}; }
signals:
    void dateChanged(const QDate& d);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
private:
    int cellAt(const QPoint& p) const;   // -1 left arrow, 7 right arrow, 0..6 days, -2 none
    QDate m_sel;
    int m_hover = -2;
};

// Group header ("Premier League"), with an optional count on the right.
class SectionHeader : public QWidget {
    Q_OBJECT
public:
    explicit SectionHeader(const QString& title, const QString& right = QString(), QWidget* parent = nullptr);
    QSize sizeHint() const override { return {400, 34}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_title, m_right;
};

// One match line: status | home | score | away | pick | star.
class MatchRow : public QWidget {
    Q_OBJECT
public:
    explicit MatchRow(QWidget* parent = nullptr);
    void setItem(const MatchItem& m, const PickInfo* pick, bool fav);
    const MatchItem& item() const { return m_item; }
    QSize sizeHint() const override { return {600, 52}; }
    static QString statusText(const MatchItem& m);
signals:
    void clicked();
    void favToggled();
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
private:
    MatchItem m_item;
    bool m_hasPick = false, m_value = false, m_fav = false, m_hover = false, m_overStar = false;
    int m_best = 0;
    double m_p = 0;
};

// Home / Draw / Away probabilities as three bars, with the bookmaker view next to it when odds exist.
class ProbRows : public QWidget {
    Q_OBJECT
public:
    explicit ProbRows(QWidget* parent = nullptr);
    void set(const QString labels[3], const sba::Probs3& model, bool hasMarket, const sba::Probs3& market);
    QSize sizeHint() const override { return {560, 150}; }
    QSize minimumSizeHint() const override { return {420, 150}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_labels[3];
    sba::Probs3 m_model{}, m_market{};
    bool m_hasMarket = false;
};

// Last results as small squares (W green, D grey, L black); oldest first.
class FormChips : public QWidget {
    Q_OBJECT
public:
    explicit FormChips(QWidget* parent = nullptr);
    void setForm(const QString& f);
    QSize sizeHint() const override;
    static void paintChips(QPainter& p, const QString& form, int x, int y, int size, int gap);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_form;
};

// Round badge with the team's three-letter code (no club logos are bundled).
void paintTeamBadge(QPainter& p, const QPointF& centre, double radius, const QString& team, bool dark);

// League table.
class StandingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StandingsWidget(QWidget* parent = nullptr);
    void setRows(const std::vector<TableRow>& r);
    QSize sizeHint() const override { return {700, 32 + 42 * 20}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::vector<TableRow> m_rows;
};

// One saved pick in "My picks".
class PickRowWidget : public QWidget {
    Q_OBJECT
public:
    enum State { Pending, Won, Lost };
    explicit PickRowWidget(QWidget* parent = nullptr);
    void set(const QString& date, const QString& teams, const QString& pick, State s, const QString& result);
    QSize sizeHint() const override { return {600, 56}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_date, m_teams, m_pick, m_result;
    State m_state = Pending;
};
