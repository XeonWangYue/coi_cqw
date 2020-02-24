#include "encode.h"
#include "ui_encode.h"

int FRAME_HEIGHT = 950;//定义帧高度
int FRAME_WIDTH = 1600;//定义帧宽度
int RECTSIZE = 150;//定义编码点尺寸
int LOCATERECTSIZE = 200;//定义定位点尺寸
int WHITEFRAMESRATE = 2;//关键帧的间隔，即此张关键帧到下一张关键帧的帧数（含此张关键帧）

cv::Size FRAME(1600, 950);//视频尺寸
cv::Size RSFRAME(1400, 750);//修剪后的尺寸

std::vector<cv::Point>codePointLocation =
{
    cv::Point(400, 250),cv::Point(400, 500),cv::Point(600, 250) ,cv::Point(600, 500),
    cv::Point(800, 250),cv::Point(800, 500),cv::Point(1000, 250),cv::Point(1000, 500)
};//定义编码点坐标

cv::Point locatPointTopLeft = cv::Point(100, 100);//定义定位点坐标-左上
cv::Size NEEDLESS(100, 100);//修剪尺寸

cv::Point locatPointBottomRight = cv::Point(1300, 650);//定义定位点坐标-右下

cv::Scalar BLACK = cv::Scalar(0, 0, 0);//黑色
cv::Scalar WHITE = cv::Scalar(255, 255, 255);//白色


Encode::Encode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Encode)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    ui->probarEncode->hide();
    ui->probarEncode->setValue(0);
    ui->labelEncodeWarning->setText("");
    ui->labelEncodeInputPath->setBuddy(ui->lineEditInput);
    ui->labelOutputPath->setBuddy(ui->lineEditOutput);
    connect(this,SIGNAL(prvalue(int)),this,SLOT(prchange(int)));
    connect(this,SIGNAL(encodeFin()),this,SLOT(encodeFinish()));
}

Encode::~Encode()
{
    delete ui;
}

void Encode::on_btnEncodeCancel_clicked()
{
    this->close();
}

void Encode::Openfile(void){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this,"打开文件",location,"二进制文件(*)");
    if(path.isEmpty())
        return;
    this->openPath=path;
    this->ui->lineEditInput->setText(path);
    binaryInput();
}

void Encode::Outputfile(void){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择路径",location);
    if(path.isEmpty())
        return;
    this->outputPath=path;
    this->ui->lineEditOutput->setText(path);
}


void Encode::on_btnEncodeInput_clicked()
{
    Openfile();
}

void Encode::on_btnEncodeOutput_clicked()
{
    Outputfile();
}

void Encode::on_btnEncodeEn_clicked()
{
    encodeStart();
}


void Encode::encodeStart(void){
    if(openPath.isEmpty()||outputPath.isEmpty()){
        ui->labelEncodeWarning->setText("路径不能为空！");
        return;
    }
    else {
        ui->btnEncodeEn->setDisabled(true);
        ui->btnEncodeInput->setDisabled(true);
        ui->btnEncodeOutput->setDisabled(true);
        ui->labelEncodeWarning->setText("转码中...");
    }
    ui->probarEncode->show();
    QtConcurrent::run([this](){
        videoInit();
    });


    return;
}

void Encode::binaryInput(void)
{
    char* pathC;
    if(fileReadin.size()!=0)
        fileReadin.clear();
    QByteArray pathQ=openPath.toLatin1();
    pathC=pathQ.data();
    FILE* f = fopen(pathC, "rb");//打开文件
    char ch;
    fseek(f, 0, SEEK_END); //定位到文件末
    int nFileLen = ftell(f);//文件长度
    fseek(f,0,SEEK_SET);//定位到文件头
    for(int i=0;i<nFileLen;i++)
    {
        ch=fgetc(f);
//        qDebug()<<ch<<"   "<<i;
        fileReadin.push_back(ch);//将字符存入c末尾
    }
//    for(int i=nFileLen-1;i>nFileLen-33;i--)
//        qDebug()<<fileReadin[i];
    fclose(f);//关闭文件
}


void Encode::videoInit(void) {
    int fourcc = cv::VideoWriter::fourcc('a','v','c','1');//文件编码格式h.264
    int fs=fileReadin.size();//读取字符数量确定视频长度，即关键帧帧数
    QString fileName=outputPath+"/vid_output.mp4";//输出路径和文件名
    vid_out=cv::VideoWriter(fileName.toStdString(),fourcc, 24, FRAME, 0);//视频输出流
    videoFrame=std::vector<cv::Mat>(fs+1);//缓存视频帧
    videoFrame[0]=cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, WHITE);//0位置存储空白帧，后面会多次调用

    //Release 2.0 新增，前置空白帧
    for(int j=0;j<24;j++)
        vid_out<<videoFrame[0];


    for(int j=1;j<=fs;j++){
        emit prvalue(j);//qt信号槽，不用管

        char c=fileReadin[j-1];//读取一个字符
        videoFrame[j]=cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, WHITE);//初始化关键帧

        //定位图像打印
        cv::rectangle(videoFrame[j], locatPointTopLeft, cv::Point(locatPointTopLeft.x + LOCATERECTSIZE, locatPointTopLeft.y + LOCATERECTSIZE), BLACK, cv::FILLED);
        cv::rectangle(videoFrame[j], locatPointBottomRight, cv::Point(locatPointBottomRight.x + LOCATERECTSIZE, locatPointBottomRight.y + LOCATERECTSIZE), BLACK, cv::FILLED);

        //信息图像打印，此位为1则打印，0不打印，保存在帧缓存中
        for (int i = 0; i < 8; i++) {
            if (c & 0x01) {
                cv::rectangle(videoFrame[j], codePointLocation[i], cv::Point(codePointLocation[i].x + RECTSIZE, codePointLocation[i].y + RECTSIZE), BLACK, cv::FILLED);
            }
            c >>= 1;
        }

        //写入视频中，可写入多次
        vid_out<<videoFrame[j];
        vid_out<<videoFrame[j];

        //写入空白帧
        for(int d=0;d<WHITEFRAMESRATE;d++)
            vid_out<<videoFrame[0];
    }

    //释放视频，可忽略
    vid_out.release();

    emit this->encodeFin();//qt信号槽，不用管

}


void Encode::prchange(int j){
    static int fs=fileReadin.size();
    ui->probarEncode->setValue(j*100/fs);
}

void Encode::encodeFinish(){
    ui->labelEncodeWarning->setText("转码成功！！！");
    ui->btnEncodeCancel->setText("返回");
}
