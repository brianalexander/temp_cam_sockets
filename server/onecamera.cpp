#include "onecamera.h"
#include "ui_onecamera.h"

#include <QPushButton>
#include <QDebug>


oneCamera::oneCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::oneCamera)
{
    ui->setupUi(this);
    QPushButton* popupButton = ui->popNewScreenButton;

    QObject::connect(popupButton, &QPushButton::clicked, this, &oneCamera::createWindow);
}

oneCamera::~oneCamera()
{
    delete ui;
}

void oneCamera::createWindow(){
    oneCamera* popup = new oneCamera();
    popup->listenTo(this->getVideoProvider());
    popup->show();
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
