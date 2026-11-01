#pragma once
// Reusable, custom-painted building blocks for the interface.
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include "core/Metrics.h"
#include "core/Types.h"

class Card : public QFrame {
    Q_OBJECT
public:
    explicit Card(const QString& title = QString(), QWidget* parent = nullptr);
    QVBoxLayout* body() const { return m_body; }
    QHBoxLayout* headerRow() const { return m_header; }
    void setTitle(const QString& t);
private:
    QVBoxLayout* m_body;
    QHBoxLayout* m_header;
    QLabel* m_title;
};

class Badge : public QLabel {
    Q_OBJECT
public:
    enum Kind { Neutral, Success, Warning, Danger, Info };
    explicit Badge(const QString& text = QString(), Kind k = Neutral, QWidget* parent = nullptr);
    void setKind(Kind k);
    void set(const QString& text, Kind k) { setText(text); setKind(k); }
};

class StatTile : public Card {
    Q_OBJECT
public:
    explicit StatTile(const QString& caption, QWidget* parent = nullptr);
    void set(const QString& value, const QString& sub = QString());
private:
    QLabel *m_value, *m_sub;
};

// Inline status strip ("data is stale", "API unavailable"...).
class Banner : public QFrame {
    Q_OBJECT
public:
    enum Kind { Info, Warning, Error, Success };
    explicit Banner(QWidget* parent = nullptr);
    void show(Kind k, const QString& text);
private:
    QLabel *m_dot, *m_text;
};

QFrame* makeDivider(QWidget* parent = nullptr);
QLabel* makeLabel(const QString& text, const char* role, QWidget* parent = nullptr);
QPushButton* makeButton(const QString& text, bool primary = false, QWidget* parent = nullptr);

// Stacked Home / Draw / Away bar with optional market ticks.
class ProbBar : public QWidget {
    Q_OBJECT
public:
    explicit ProbBar(QWidget* parent = nullptr);
    void setProbs(const sba::Probs3& p, bool valid = true);
    void setMarket(const sba::Probs3& m, bool valid);
    QSize sizeHint() const override { return {240, 26}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    sba::Probs3 m_p{}, m_m{};
    bool m_valid = false, m_hasMarket = false;
};

// One horizontal bar for a single outcome: model fill + market marker.
class OutcomeBar : public QWidget {
    Q_OBJECT
public:
    OutcomeBar(const QString& label, const QColor& color, QWidget* parent = nullptr);
    void setValues(double model, double market);   // NaN market = none
    QSize sizeHint() const override { return {300, 34}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString m_label; QColor m_color; double m_model = 0, m_market = std::nan("");
};

class CalibrationChart : public QWidget {
    Q_OBJECT
public:
    explicit CalibrationChart(QWidget* parent = nullptr);
    void setBins(const std::vector<sba::CalibrationBin>& b);
    QSize sizeHint() const override { return {360, 300}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::vector<sba::CalibrationBin> m_bins;
};

class ConfusionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConfusionWidget(QWidget* parent = nullptr);
    void setMatrix(const sba::ConfusionMatrix& m);
    QSize sizeHint() const override { return {320, 240}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    sba::ConfusionMatrix m_m;
};

// Left-hand activity bar button with a vector icon.
class NavButton : public QPushButton {
    Q_OBJECT
public:
    enum Icon { Dashboard, Matches, Analysis, Performance, Search, Settings };
    NavButton(Icon icon, const QString& tooltip, QWidget* parent = nullptr);
    QSize sizeHint() const override { return {52, 48}; }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    Icon m_icon;
};
