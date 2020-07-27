#include <Windows.h>
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include "mainwindow.h"

//extern "C" {
//    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//}
//extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QSurfaceFormat format;
//    format.setDepthBufferSize(24);
//    format.setStencilBufferSize(8);
//    format.setVersion(3, 2);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;
    w.show();

    return a.exec();
}
