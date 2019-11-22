#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::addDevice(QString cameraId, int fd) {
    this->ui->singleCameraCB->addItem(cameraId);
}

void MainWindow::removeDevice(QString cameraId) {
    this->ui->singleCameraCB->removeItem(
                this->ui->singleCameraCB->findText(cameraId)
                );
}

MainWindow::~MainWindow()
{
    delete ui;
}
