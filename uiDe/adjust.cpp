#include "adjust.h"
#include "ui_adjust.h"


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
adjust::adjust(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::adjust)
{
    ui->setupUi(this);
}
adjust::adjust(QString path, QWidget *parent):
    QWidget(parent),
    THRESH(100),
    ui(new Ui::adjust){
    ui->setupUi(this);
    ui->btnThreshComf->setFocus();
    ui->btnThreshComf->setDefault(true);
    ui->progressBar->hide();
    ui->labelStatus->hide();
    ui->textBrowser->hide();
    connect(this,SIGNAL(decodeFin()),this,SLOT(decodeFinish()));
    connect(this,SIGNAL(prvalue(int)),this,SLOT(prchange(int)));
    connect(this,SIGNAL(textout(char)),this,SLOT(textappend(char)));
    this->PATH=path;
    this->ui->lineEditThresh->setText("100");
    vid_in.open(this->PATH.toStdString());

    QValidator *validator=new QIntValidator(0,255,ui->lineEditThresh);
    this->ui->lineEditThresh->setValidator(validator);


    vid_in.read(this->tempcv);
    cv::Mat display=tempcv;
    cv::cvtColor(tempcv, tempcv, cv::COLOR_RGB2GRAY);

    cv::Size GUASSKERNELSRC(23, 23);
    cv::GaussianBlur(tempcv, tempcv, GUASSKERNELSRC, 0, 0);

    Paintpic();

//    cv::threshold(tempcv,display,this->THRESH,255,cv::THRESH_BINARY);
//    cv::resize(display,display,cv::Size(720,405));
//    QImage tempqt;
//    tempqt=CV2QT(display);
//    ui->labelPic->setPixmap(QPixmap::fromImage(tempqt));
}
adjust::~adjust()
{
    delete ui;
}

void adjust::Decode(){
    //获取视频信息
    int frames = vid_in.get(cv::CAP_PROP_FRAME_COUNT);
    int num=0;
    //生成遮罩
    std::vector<cv::Mat> mask(8);
    for (int i = 0; i < 8; i++) {
        mask[i] = cv::Mat(RSFRAME,CV_8UC1,WHITE);
        cv::rectangle(mask[i], locatPointTopLeft, cv::Point(locatPointTopLeft.x + LOCATERECTSIZE, locatPointTopLeft.y + LOCATERECTSIZE), WHITE, cv::FILLED);
        cv::rectangle(mask[i], locatPointBottomRight, cv::Point(locatPointBottomRight.x + LOCATERECTSIZE, locatPointBottomRight.y + LOCATERECTSIZE), WHITE, cv::FILLED);
        cv::rectangle(mask[i], codePointLocation[i] - locatPointTopLeft,cv::Point(codePointLocation[i].x + RECTSIZE - locatPointTopLeft.x, codePointLocation[i].y + RECTSIZE - locatPointTopLeft.y), BLACK, cv::FILLED);
    }

    //-----------------------------------视频预处理，初步ROI------------------------------//
    std::vector<cv::Mat>video_pro;

    for (int k = 0; k < frames ; k++) {
        emit prvalue(k);

        cv::Mat src;
        vid_in.read(src);

        if (src.empty())
            break;
        //彩色转灰度
        cv::cvtColor(src, src, cv::COLOR_RGB2GRAY);

//        //平滑图像-高斯模糊
//        cv::Size GUASSKERNELSRC(23, 23);
//        cv::GaussianBlur(src, src, GUASSKERNELSRC, 0, 0);

        //阈值
        int SRC_THRESH_LOW = this->THRESH;
        cv::threshold(src, src, SRC_THRESH_LOW, 255, cv::THRESH_BINARY);

//        //形态学开
//        cv::Mat kernelsrc = getStructuringElement(cv::MORPH_RECT, cv::Size(40, 40), cv::Point(-1, -1));
//        cv::morphologyEx(src, src, cv::MORPH_OPEN, kernelsrc);

        //Canny边缘检测
        cv::Mat src_gray = src;
        cv::Mat canny_output;
        cv::Canny(src_gray, canny_output, 200, 255);

        //轮廓提取
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if(contours.size()<1)//若提取失败，跳过此帧
            continue;

        //提取最大矩形，即屏幕现实范围
        cv::Rect poly_rect_max=cv::boundingRect(contours[0]);
        for (int i = 1; i < contours.size(); i++) {
            if(poly_rect_max.area()<cv::boundingRect(contours[i]).area())
                poly_rect_max=cv::boundingRect(contours[i]);
        }

//        //生成ROI遮罩，覆盖屏幕外噪声
//        cv::Mat t=cv::Mat(src.size(),CV_8UC1,cv::Scalar(255,255,255));
//        cv::rectangle(t,poly_rect_max,cv::Scalar(0,0,0),cv::FILLED);
//        src-=t;
        //ROI坐标生成 tl=topleft,br=bottomright
        int tl_x =  poly_rect_max.tl().x;
        int br_x =  poly_rect_max.br().x;
        int tl_y =  poly_rect_max.tl().y;
        int br_y =  poly_rect_max.br().y;

        //ROI区域生成
        cv::Rect rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y);
        cv::Mat srcRoi=cv::Mat::zeros(src.size(), CV_8UC1);
        srcRoi=src(rect);
        cv::resize(srcRoi, srcRoi, RSFRAME);

        //存入预降噪vector保存
        video_pro.push_back(srcRoi);
    }
    num=frames;
    //-------------------------------------前后帧差--------------------------------------//
    std::vector<cv::Mat> video_diff;
    for(int i=1;i<video_pro.size();i++){
        emit prvalue(num+i);

        //差值
        cv::Mat st=video_pro[i]-video_pro[i-1];
//        //平滑图像，高斯模糊
//        cv::Size GUASSKERNELSRC(23, 23);
//        cv::GaussianBlur(st, st, GUASSKERNELSRC, 0, 0);

//        //阈值
//        int SRC_THRESH_LOW = THRESH;
//        cv::threshold(st, st, SRC_THRESH_LOW, 255, cv::THRESH_BINARY);

        //形态学开
        cv::Mat kernelsrc = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(23, 23), cv::Point(-1, -1));
        cv::morphologyEx(st, st, cv::MORPH_OPEN, kernelsrc);


        //存入差值vector

        video_diff.push_back(st);
    }
    num+=video_diff.size();
    //----------------------------------------字符提取------------------------------------//
    for(int i=0;i<video_diff.size();i++){
       // emit prvalue(num+i);
        //过滤低信息图片
      if(cv::countNonZero(video_diff[i])==0)
          continue;
        cv::Mat read=video_diff[i];
        //Canny边缘查找
        cv::Mat src_gray = read;
        cv::Mat canny_output;
        cv::Canny(src_gray, canny_output, 200, 255);


        //轮廓查找
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if(contours.size()<=2)
            continue;
        //从轮廓生成最小矩形
        std::vector<cv::Rect>poly_rect(contours.size());
        for (int i = 0; i < contours.size(); i++) {
            poly_rect[i] = cv::boundingRect(contours[i]);
        }
        //矩形轮廓x坐标整理
        for (int i = poly_rect.size(); i > 0; i--) {
            for (int j = 0; j < i - 1; j++) {
                if (poly_rect[j].tl().x > poly_rect[j + 1].tl().x)
                    swap(poly_rect[j], poly_rect[j + 1]);
            }
        }
        //矩形轮廓y坐标整理
        for (int i = 0; i + 1 < poly_rect.size(); i += 2) {
            if (poly_rect[i].tl().y > poly_rect[i + 1].tl().y)
                swap(poly_rect[i], poly_rect[i + 1]);
        }

        //ROI坐标生成 tl=topleft,br=bottomright
        int tl_x = poly_rect[0].tl().x;
        int br_x = poly_rect[poly_rect.size() - 1].br().x;
        int tl_y = poly_rect[0].tl().y;
        int br_y = poly_rect[poly_rect.size() - 1].br().y;
        if(tl_x&&br_x&&tl_y&&br_y==0)
            continue;

        //矩形轮廓重建
        cv::Mat rscrc = cv::Mat::zeros(canny_output.size(), CV_8UC1);
        for (int i = 0; i < poly_rect.size(); i++) {
            cv::rectangle(rscrc, poly_rect[i], WHITE, cv::FILLED);
        }

        //ROI区域生成
        cv::Rect rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y);
        cv::Mat srcRoi=cv::Mat::zeros(rscrc.size(), CV_8UC1);
        srcRoi=rscrc(rect);
        cv::resize(srcRoi, srcRoi, RSFRAME);

//        //形态学开
//        cv::Mat kernelsrc = getStructuringElement(cv::MORPH_RECT, cv::Size(23, 23), cv::Point(-1, -1));
//        cv::morphologyEx(srcRoi, srcRoi, cv::MORPH_OPEN, kernelsrc);

        //解码
        char c=0;
        for (int i = 7; i >= 0; i--) {
            //缓存帧
            cv::Mat p = cv::Mat::zeros(RSFRAME, CV_8UC1);

            //添加遮罩
            p = srcRoi-mask[i];

            //去噪
//            cv::Mat kernelroi = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10), cv::Point(-1, -1));
//            cv::morphologyEx(p, p,cv::MORPH_OPEN, kernelroi);

            //译码
            if (cv::countNonZero(p) >500)
                c |= 0x01;
            if (i != 0)
                c <<= 1;
        }
        emit textout(c);
    }
    emit decodeFin();
}

QImage  adjust::CV2QT(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)                             //3 channels color image
    {

        cv::cvtColor(cvImg,cvImg,cv::COLOR_BGR2RGB);
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols, cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    else if(cvImg.channels()==1)                    //grayscale image
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_Indexed8);
    }
    else
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }

    return qImg;

}

void adjust::on_pushButton_clicked()
{
    this->close();
}


void adjust::on_btnThreshComf_clicked()
{
    Paintpic();
}

void adjust::on_lineEditThresh_returnPressed()
{
    Paintpic();
}

void adjust::Paintpic(){
    this->THRESH=ui->lineEditThresh->text().toInt();
    cv::Mat display;
    cv::threshold(this->tempcv,display,this->THRESH,255,cv::THRESH_OTSU);
    cv::resize(display,display,cv::Size(720,405));
    QImage tempqt;
    tempqt=CV2QT(display);
    ui->labelPic->setPixmap(QPixmap::fromImage(tempqt));
    ui->labelPic->repaint();
}

void adjust::on_btnDecode_clicked()
{
    ui->labelPic->hide();
    ui->label->hide();
    ui->label_2->hide();
    ui->btnDecode->hide();
    ui->btnThreshComf->hide();
    ui->lineEditThresh->hide();
    ui->pushButton->hide();

    ui->labelStatus->setText("正在解码请稍等....");
     ui->labelStatus->show();

     ui->progressBar->setValue(0);
     ui->progressBar->show();
     ui->textBrowser->setPlaceholderText("现在空空如也...");
     ui->textBrowser->show();


    QtConcurrent::run([this](){
        Decode();
    });


}

void adjust::prchange(int j){
    static int fs=this->vid_in.get(cv::CAP_PROP_FRAME_COUNT)*3-2;
    ui->progressBar->setValue(j*100/fs);
}

void adjust::textappend(char c){
    QString temp=c;
    ui->textBrowser->insertPlainText(temp);
}
void adjust::decodeFinish(void){
ui->progressBar->setValue(100);
ui->labelStatus->setText("解码完成！");
}
