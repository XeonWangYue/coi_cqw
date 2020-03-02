#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->portBox->addItem(serial.portName());
            serial.close();
        }
    }
    ui->baudBox->addItem(QString::number(1200));
    ui->baudBox->addItem(QString::number(2400));
    ui->baudBox->addItem(QString::number(4800));
    ui->baudBox->addItem(QString::number(19200));
    ui->baudBox->addItem(QString::number(38400));
    ui->baudBox->addItem(QString::number(57600));
    ui->baudBox->addItem(QString::number(115200));
    ui->dataBox->addItem(QString::number(8));
    ui->dataBox->addItem(QString::number(7));
    ui->dataBox->addItem(QString::number(6));
    ui->dataBox->addItem(QString::number(5));
    ui->checkBox->addItem("None");
    ui->checkBox->addItem("even");
    ui->checkBox->addItem("odd");
    ui->stopBox->addItem("1");
     ui->stopBox->addItem("2");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_btnopen_clicked()
{
    if(ui->btnopen->text()==tr("打开串口"))
    {
     serial=new QSerialPort;
     //设置串口名
     serial->setPortName(ui->portBox->currentText());
     //打开串口
     serial->open(QIODevice::ReadWrite);
     //设置波特率
     serial->setBaudRate(QSerialPort::Baud115200);
     //设置数据为
     switch(ui->baudBox->currentIndex()){
     case 0:
         serial->setBaudRate(QSerialPort::Baud1200);
         break;
     case 1:
         serial->setBaudRate(QSerialPort::Baud2400);
         break;
     case 2:
         serial->setBaudRate(QSerialPort::Baud4800);
         break;
     case 3:
         serial->setBaudRate(QSerialPort::Baud9600);
         break;
     case 4:
         serial->setBaudRate(QSerialPort::Baud19200);
         break;
     case 5:
         serial->setBaudRate(QSerialPort::Baud38400);
         break;
     case 6:
         serial->setBaudRate(QSerialPort::Baud57600);
         break;
     case 7:
         serial->setBaudRate(QSerialPort::Baud115200);
         break;
     default:
         break;
     }
     switch(ui->dataBox->currentIndex())
     {
         case 0:
         serial->setDataBits(QSerialPort::Data8);
         break;
     case 1:
     serial->setDataBits(QSerialPort::Data7);
     break;
     case 2:
     serial->setDataBits(QSerialPort::Data6);
     break;
     case 3:
     serial->setDataBits(QSerialPort::Data5);
     break;
         default:
         break;
     }
     //设置校验位
     switch (ui->checkBox->currentIndex())
     {
         case 0:
         serial->setParity(QSerialPort::NoParity);
         break;
     case 1:
     serial->setParity(QSerialPort::EvenParity);
     break;
     case 2:
     serial->setParity(QSerialPort::OddParity);
     break;
         default:
         break;
     }
     //设置停止为
     switch(ui->stopBox->currentIndex())
     {
         case 0:
         serial->setStopBits(QSerialPort::OneStop);
         break;
         case 1:
         serial->setStopBits(QSerialPort::TwoStop);
         break;
         default:
         break;
     }
     //设置流控制
     serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

     //关闭设置菜单使能
     ui->portBox->setEnabled(false);
     ui->dataBox->setEnabled(false);
     ui->checkBox->setEnabled(false);
     ui->stopBox->setEnabled(false);
     ui->baudBox->setEnabled(false);
     ui->btnopen->setText("关闭串口");


     QObject::connect(serial,&QSerialPort::readyRead,this,&MainWindow::ReadData);
    }
    else
    {
    //关闭串口
    serial->clear();
    serial->close();
    serial->deleteLater();

    //恢复使能
    ui->portBox->setEnabled(true);
    ui->baudBox->setEnabled(true);
    ui->dataBox->setEnabled(true);
    ui->checkBox->setEnabled(true);
    ui->stopBox->setEnabled(true);
    ui->btnopen->setText("打开串口");
    }
}

void MainWindow::on_btnsend_clicked()
{
     serial->write(ui->txtWrite->toPlainText().toLatin1());
}

void MainWindow::ReadData()
{
    QByteArray buf;

    buf=serial->readAll();
    if(!buf.isEmpty())
        {
            QString str = buf;
            ui->txtRead->appendPlainText(str);
        }
    buf.clear();
}
