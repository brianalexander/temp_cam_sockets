#ifndef ONECAMERA_H
#define ONECAMERA_H

#include <QResizeEvent>
#include <QWidget>

// OpenCV
#include "opencv2/opencv.hpp"

namespace Ui {
class oneCamera;
}

class oneCamera : public QWidget
{
    Q_OBJECT

public:
    explicit oneCamera(QWidget *parent = nullptr);
    ~oneCamera();
protected:
    void resizeEvent(QResizeEvent *event);
public slots:
    void drawFrame(cv::Mat frame);

private:
    Ui::oneCamera *ui;
    QPixmap myPixmap;
};

#endif // ONECAMERA_H
