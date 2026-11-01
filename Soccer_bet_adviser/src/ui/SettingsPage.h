#pragma once
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include "AppController.h"
#include "Widgets.h"

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(AppController* c, QWidget* parent = nullptr);
private:
    void load();
    AppSettings collect() const;
    AppController* m_c;
    QLineEdit *m_fdKey, *m_oddsKey, *m_comp, *m_sport, *m_regions;
    QSpinBox *m_live, *m_fix, *m_odds, *m_age, *m_minMatches;
    QDoubleSpinBox *m_ev, *m_edge, *m_dis, *m_shrink, *m_rf;
    QCheckBox* m_auto;
    QLabel* m_info;
    Banner* m_banner;
};
