#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> // for memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <signal.h>

// Data structures
#include <vector>
#include <set>
#include <map>

// Thread management
#include <thread>
#include <chrono>

// OpenCV
#include "opencv2/opencv.hpp"

#include "uicamera.h"
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

    UiCamera ui;
    oneCamera* cameraOne = ui.findChild<oneCamera*>("cameraOne");
    QObject::connect(&portListener1, &VideoListenerThread::frameCompleted, cameraOne, &oneCamera::drawFrame);

    ui.show();
    return a.exec();
}
