#ifndef QTMON_H
#define QTMON_H

#include <QApplication>
#include <QDebug>
#include <QtGui>

class MainWindow : public QWidget{ Q_OBJECT
public:
  explicit MainWindow();

private:
  QLineEdit *sip_url;

};

#endif
