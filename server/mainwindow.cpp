#include "uicamera.h"
#include "ui_uicamera.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    QObject::connect()
}

MainWindow::~MainWindow()
{
    delete ui;
}