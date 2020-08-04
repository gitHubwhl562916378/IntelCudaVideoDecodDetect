#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include "mainwindow.h"
#include "VideoWidget/videowidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    playBtn_ = new QPushButton("play");
    //rtsp://192.168.2.66/mclz_cooking.mp4 rtsp://192.168.2.66/person.avi
    urlEdit_ = new QLineEdit("rtsp://10.10.8.11/person.avi");
    decoderBox_ = new QComboBox();
    gridLay_ = new QGridLayout;
    dCheck_ = new QCheckBox("auto transform");
    dCheck_->setChecked(true);

    QVBoxLayout *mainLay = new QVBoxLayout;

    decoderBox_->addItems(QStringList() << tr("cpu") << tr("cuda") << tr("cuda_plugin") << tr("qsv"));
    QHBoxLayout *hlay = new QHBoxLayout;
    hlay->addWidget(urlEdit_);
    hlay->addStretch();
    mainLay->addLayout(hlay);

    hlay = new QHBoxLayout;
    hlay->addWidget(decoderBox_);
    hlay->addWidget(dCheck_);
    hlay->addWidget(playBtn_);
    hlay->addStretch();
    mainLay->addLayout(hlay);

    mainLay->addLayout(gridLay_);
    connect(playBtn_, SIGNAL(clicked()), this, SLOT(slotPlayBtnClicked()));

    setLayout(mainLay);

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            QLabel *fpsL = new QLabel;
            QPalette pal = fpsL->palette();
            pal.setColor(QPalette::Foreground, Qt::blue);
            fpsL->setPalette(pal);
            QFont f = fpsL->font();
            f.setPixelSize(18);
            fpsL->setFont(f);
            QLabel *curFpsL = new QLabel;
            curFpsL->setPalette(pal);
            curFpsL->setFont(f);
            VideoWidget *videoW = new VideoWidget;
            connect(videoW, &VideoWidget::sigFps, [=](int fps){
                fpsL->setNum(fps);
                QPalette pal = fpsL->palette();
                pal.setColor(QPalette::Foreground, Qt::green);
                fpsL->setPalette(pal);
            });

            connect(videoW, &VideoWidget::sigCurFpsChanged, [=](int curFps){
                curFpsL->setNum(curFps);
                QPalette pal = curFpsL->palette();
                pal.setColor(QPalette::Foreground, Qt::red);
                curFpsL->setPalette(pal);
            });

            QHBoxLayout *hlay = new QHBoxLayout;
            hlay->addStretch();
            hlay->addWidget(curFpsL);
            hlay->addWidget(fpsL);
            QVBoxLayout *vlay = new QVBoxLayout;
            vlay->addLayout(hlay);
            vlay->addStretch();
            videoW->setLayout(vlay);
            gridLay_->addWidget(videoW, i, j);
        }
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::slotPlayBtnClicked()
{
    VideoWidget *videoW = qobject_cast<VideoWidget*>(gridLay_->itemAt(playIndex % gridLay_->count())->widget());
    QString decoder;
    decoder = decoderBox_->currentText();
    if(dCheck_->isChecked())
    {
        decoder = decoderBox_->itemText(playIndex % decoderBox_->count());
    }
    videoW->slotPlay(urlEdit_->text(), decoder);
    playIndex++;
}
