#ifndef ONECAMERA_H
#define ONECAMERA_H

#include <QResizeEvent>
#include <QWidget>

// OpenCV
#include "opencv2/opencv.hpp"
#include "videolistenerthread.h"

namespace Ui {
class oneCamera;
}

class oneCamera : public QWidget
{
    Q_OBJECT

public:
    explicit oneCamera(QWidget *parent = nullptr);

    void listenTo(VideoListenerThread* vidList) {
        videoListener = vidList;

        QObject::connect(videoListener, &VideoListenerThread::frameCompleted, this, &oneCamera::drawFrame);
    }

    VideoListenerThread* getVideoProvider() {
        return videoListener;
    }

    ~oneCamera();

public slots:
    void drawFrame(cv::Mat frame);
    void createWindow();

private:
    Ui::oneCamera *ui;
    QPixmap myPixmap;
    VideoListenerThread* videoListener;
};

#endif // ONECAMERA_H
