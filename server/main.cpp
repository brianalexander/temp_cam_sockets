// OpenCV
#include "opencv2/opencv.hpp"

#include "mainwindow.h"
#include "onecamera.h"
#include "global.h"

#include <QThread>
#include <QApplication>
#include <QDebug>

// Thread Objects

#include "tcplistenerthread.h"
#include "../packetdefinitions.hpp"


Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(ConfigurationPacket)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<ConfigurationPacket>();

    MainWindow mainWindow;

    mainWindow.show();
    return a.exec();
}
