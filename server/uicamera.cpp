#include "uicamera.h"
#include "ui_uicamera.h"

UiCamera::UiCamera(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UiCamera)
{
    ui->setupUi(this);

//    QObject::connect()
}

UiCamera::~UiCamera()
{
    delete ui;
}
