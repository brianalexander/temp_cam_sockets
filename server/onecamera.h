#ifndef ONECAMERA_H
#define ONECAMERA_H

#include <QResizeEvent>
#include <QWidget>

// OpenCV
#include "opencv2/opencv.hpp"
#include "videolistenerthread.h"
#include "../packetdefinitions.hpp"

namespace Ui {
class oneCamera;
}

class oneCamera : public QWidget
{
    Q_OBJECT

public:
    explicit oneCamera(QWidget *parent = nullptr);

    void setViewIndex(int id) {
        m_id = id;
    }

    void setCameraId(QString cameraId) {
        m_currentCamera = cameraId;
    }

    ~oneCamera();

signals:
    void requestPopup(int videoListenerIndex);
    void qualityChanged(int viewIndex, QString cameraId, int quality);

public slots:
    void drawFrame(cv::Mat frame);
    void popupButtonClicked();
    void qualityButtonClicked();

private:
    Ui::oneCamera *ui;
    QPixmap myPixmap;
    int m_id;
    QString m_currentCamera;
};

#endif // ONECAMERA_H
