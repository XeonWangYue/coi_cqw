#include "decode.h"
#include "ui_decode.h"
#include "adjust.h"


extern int FRAME_HEIGHT;
extern int FRAME_WIDTH;
extern int RECTSIZE;
extern int LOCATERECTSIZE;
extern int WHITEFRAMESRATE;

extern cv::Size FRAME;
extern cv::Size RSFRAME;

extern std::vector<cv::Point>codePointLocation;

extern cv::Point locatPointTopLeft;
extern cv::Size NEEDLESS;

extern cv::Point locatPointBottomRight;

extern cv::Scalar BLACK;
extern cv::Scalar WHITE;
constexpr double FPS_VIDEO = 24;
constexpr double FPS_PHONE = 60;
constexpr double DELAY = 0.030;

Decode::Decode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Decode)
{
    ui->setupUi(this);
    ui->btnDecodeDe->setDisabled(true);

}

Decode::~Decode()
{
    delete ui;
}
void Decode::on_btnDecodeDe_clicked()
{
//    ParaDecodeSet* pd=new ParaDecodeSet(this);
//    pd->show();

    adjust* ad=new adjust(this->Path,this);
    ad->show();

}

void Decode::Openfile(){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this,"打开文件",location,"视频文件(*.mp4)");
    if(path.isEmpty())
        return;
    this->Path=path;
    this->ui->lineEditDecodePath->setText(path);
    this->ui->btnDecodeDe->setEnabled(true);
}

void Decode::on_btnDecodeSelect_clicked()
{
    Openfile();
}

void Decode::on_btnDecodeCancel_clicked()
{
    this->close();
}

