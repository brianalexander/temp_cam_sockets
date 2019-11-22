// OpenCV
#include "opencv2/opencv.hpp"

#include "mainwindow.h"
#include "onecamera.h"

#include <QThread>
#include <QApplication>

// Thread Objects
#include "videolistenerthread.h"
#include "tcplistenerthread.h"

Q_DECLARE_METATYPE(cv::Mat)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<cv::Mat>();

    TcpListenerThread tcpListener;
    tcpListener.setPort("12345");
    tcpListener.setId(1);
    tcpListener.start();

    VideoListenerThread portListener1;
    portListener1.setPort("23456");
    portListener1.setId(1);
    portListener1.start();

    MainWindow mainWindow;

    oneCamera* cameraOne = mainWindow.findChild<oneCamera*>("cameraOne");
    oneCamera* cameraTwo = mainWindow.findChild<oneCamera*>("cameraTwo");
    oneCamera* cameraThree = mainWindow.findChild<oneCamera*>("cameraThree");
    oneCamera* cameraFour = mainWindow.findChild<oneCamera*>("cameraFour");

    QObject::connect(&portListener1, &VideoListenerThread::frameCompleted, cameraOne, &oneCamera::drawFrame);
    QObject::connect(&tcpListener, &TcpListenerThread::deviceConnected, &mainWindow, &MainWindow::addDevice);
    mainWindow.show();
    return a.exec();
}
