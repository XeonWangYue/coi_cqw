#ifndef ADJUST_H
#define ADJUST_H
#include <opencv2/opencv.hpp>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QImage>
#include <QValidator>
#include <QPixmapCache>
#include <QtConcurrent>
namespace Ui {
class adjust;
}

class adjust : public QWidget
{
    Q_OBJECT

public:
    explicit adjust(QWidget *parent = nullptr);
    explicit adjust(QString path, QWidget *parent=nullptr);
    void Decode();
    void Openfile();
    void Paintpic();
    QImage CV2QT(cv::Mat);
    ~adjust();

private slots:
    void on_pushButton_clicked();
    void on_btnThreshComf_clicked();
    void on_lineEditThresh_returnPressed();
    void on_btnDecode_clicked();
    void prchange(int);
    void textappend(char);
    void decodeFinish();
signals:
    void prvalue(int);
    void textout(char);
    void decodeFin();
private:
    Ui::adjust *ui;
    QString PATH;
    cv::Mat tempcv;

    cv::VideoCapture vid_in;
    double THRESH;
};


#endif // ADJUST_H
