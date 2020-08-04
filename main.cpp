#include <Windows.h>
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include "mainwindow.h"

extern "C" {
      //确保连接了nvidia的显示器
//    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
      //确保连接了amd的显示器
//    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QSurfaceFormat format;
//    format.setDepthBufferSize(8);
//    format.setStencilBufferSize(8);
//    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
//    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;
    w.show();

    return a.exec();
}
