#include "onecamera.h"
#include "ui_onecamera.h"


#include <QDebug>


oneCamera::oneCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::oneCamera)
{
    ui->setupUi(this);
}

oneCamera::~oneCamera()
{
    delete ui;
}


void oneCamera::resizeEvent(QResizeEvent *event) {
//    ui->cameraPic->resize(640,480);
//    ui->cameraPic->setPixmap(myPixmap.scaled(event->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//    this->pixmap()->scaled(event->size(), Qt::KeepAspectRatio);
//    qDebug() << event->size().height();
}

void oneCamera::drawFrame(cv::Mat frame){
    qDebug() << frame.cols << " " << frame.rows;
    // Set the color table (used to translate colour indexes to qRgb values)
    QVector<QRgb> colorTable;
    for (int i=0; i<256; i++)
        colorTable.push_back(qRgb(i,i,i));
    // Create QImage with same dimensions as input Mat
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    img.setColorTable(colorTable);

    ui->cameraPic->setPixmap(
                QPixmap::fromImage(img).scaled(
                    ui->cameraPic->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation));
}
