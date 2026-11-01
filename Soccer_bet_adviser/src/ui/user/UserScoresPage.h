#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include "UserWidgets.h"

// Home screen: date strip, filters and the match list grouped by competition (livescore style).
class UserScoresPage : public QWidget {
    Q_OBJECT
public:
    UserScoresPage(UserDataHub* hub, QWidget* parent = nullptr);
    void showLatest();
signals:
    void openMatch(const MatchItem& m);
private:
    void rebuild();
    void updateNotice();
    UserDataHub* m_hub;
    DateStrip* m_dates;
    TextTabs* m_filter;
    QLineEdit* m_search;
    QLabel* m_notice;
    QScrollArea* m_scroll;
    QWidget* m_list = nullptr;
    QTimer m_debounce;
    bool m_userNavigated = false;
};
