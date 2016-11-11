#include "qtmon.hpp"
#include <QRegExp>
#include <QDebug>

int main( int argc, char **argv ){
  QApplication a(argc, argv);
  MainWindow *ui = new MainWindow;

  ui->show();
  
  return a.exec();
}


MainWindow::MainWindow(){
  this->setWindowTitle("QtMon");
  this->setFixedSize(600,300);  

  QVBoxLayout *top = new QVBoxLayout;
  this->setLayout(top);

  QGroupBox   *gbox_conf = new QGroupBox("Server");
  QFormLayout *conf = new QFormLayout;
  gbox_conf->setLayout(conf);
  top->addWidget(gbox_conf);

  sip_url = new QLineEdit;
  conf->addRow("Endpoint:", sip_url);

  callBtn = new QPushButton("Start");
  conf->addRow("", callBtn);

  connect(callBtn, SIGNAL(clicked()), this, SLOT(clickBtn()));
}

void MainWindow::clickBtn(){
  QRegExp rx("^[^@]+@.+");
  if(rx.exactMatch(sip_url->text())){
    qDebug() << "Call" << sip_url->text();
    emit startCall(sip_url->text());
  } else {
    qDebug() << "Bad SIP URL" << sip_url->text();
  }
}
