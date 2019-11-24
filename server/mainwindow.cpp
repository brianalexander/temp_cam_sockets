#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "tcplistenerthread.h"

#include <vector>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItem>
#include <QStandardItemModel>
#include "videolistenerthread.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcpListener = new TcpListenerThread();
    tcpListener->setPort(12345);
    tcpListener->setId(1);
    tcpListener->start();

    videoListenerThreads = new std::vector<VideoListenerThread*>();

    for(int i = 0; i < 4; i++) {
        VideoListenerThread *videoListener = new VideoListenerThread();
        videoListener->setPort(23456+i);
        videoListener->setId(i);
        videoListener->start();
        videoListenerThreads->push_back(videoListener);
    }

    oneCamera* cameraOne = ui->cameraOne;
    oneCamera* cameraTwo = ui->cameraTwo;
    oneCamera* cameraThree = ui->cameraThree;
    oneCamera* cameraFour = ui->cameraFour;

    QComboBox* singleCameraComboBox = ui->singleCameraCB;
    QComboBox* configurationComboBox = ui->configurationCB;

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>( singleCameraComboBox->model() );
    QStandardItem* item = model->item(0, 0);
    item->setEnabled(false);

    model = dynamic_cast<QStandardItemModel*>( configurationComboBox->model() );
    item = model->item(0, 0);
    item->setEnabled(false);

    // Add configurations to combobox
    QJsonObject configurationList = global::configObject["configurations"].toObject();
    configurationComboBox->addItems(configurationList.keys());

    QObject::connect(tcpListener, &TcpListenerThread::deviceConnected, this, &MainWindow::addDevice);
    QObject::connect(tcpListener, &TcpListenerThread::deviceDisconnected, this, &MainWindow::removeDevice);
    QObject::connect(singleCameraComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MainWindow::buildOneConfiguration);
    QObject::connect(configurationComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MainWindow::buildConfigurations);

    QObject::connect(this, &MainWindow::forwardConfiguration, tcpListener, &TcpListenerThread::sendConfiguration);

    QObject::connect(videoListenerThreads->at(0), &VideoListenerThread::frameCompleted, cameraOne, &oneCamera::drawFrame);
    QObject::connect(videoListenerThreads->at(1), &VideoListenerThread::frameCompleted, cameraTwo, &oneCamera::drawFrame);
    QObject::connect(videoListenerThreads->at(2), &VideoListenerThread::frameCompleted, cameraThree, &oneCamera::drawFrame);
    QObject::connect(videoListenerThreads->at(3), &VideoListenerThread::frameCompleted, cameraFour, &oneCamera::drawFrame);

}

void MainWindow::buildConfigurations(const QString& configurationId) {
    QJsonObject configurationList = global::configObject["configurations"].toObject();
    QJsonArray cameraList = configurationList[configurationId].toArray();

    for(int i = 0; i < cameraList.size(); i++) {
        buildConfiguration(cameraList[i].toString(), i);
    }
}

void MainWindow::buildOneConfiguration(const QString& cameraId) {
    buildConfiguration(cameraId, 0);
}

void MainWindow::buildConfiguration(const QString& cameraId, int index) {
    QJsonObject deviceJSON = global::configObject["devices"]
            .toObject()[cameraId]
            .toObject();

    ConfigurationPacket confPack = {
        deviceJSON["device"].toString().toStdString(),
        QString(videoListenerThreads->at(index)->getPort()).toStdString(),
        static_cast<u_int8_t>(deviceJSON["fps"].toInt()),
        static_cast<u_int8_t>(deviceJSON["quality"].toInt()),
        static_cast<u_int16_t>(deviceJSON["resolutionX"].toInt()),
        static_cast<u_int16_t>(deviceJSON["resolutionY"].toInt()),
    };

    emit forwardConfiguration(cameraId, confPack);
}

void MainWindow::addDevice(QString cameraId, int fd) {
    // Only add to the list if it doesn't already exist
    // Might occur on quick reconnects
    if(this->ui->singleCameraCB->findText(cameraId) == -1) {
        this->ui->singleCameraCB->addItem(cameraId);
    }
}

void MainWindow::removeDevice(QString cameraId, int fd) {
    this->ui->singleCameraCB->removeItem(
                this->ui->singleCameraCB->findText(cameraId)
                );
}

MainWindow::~MainWindow()
{
    delete ui;
}
