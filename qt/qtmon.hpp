#ifndef QTMON_H
#define QTMON_H

#include <QApplication>
#include <QDebug>
#include <QtGui>

class MainWindow : public QWidget{ Q_OBJECT
public:
  explicit MainWindow();

signals:
  void startCall(QString);

private:
  QLineEdit *sip_url;
  QPushButton *callBtn;

private slots:
  void clickBtn();
};

#endif
