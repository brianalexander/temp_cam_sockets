#ifndef UICAMERA_H
#define UICAMERA_H

#include <QMainWindow>
#include "onecamera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UiCamera; }
QT_END_NAMESPACE

class UiCamera : public QMainWindow
{
    Q_OBJECT

public:
    UiCamera(QWidget *parent = nullptr);
    ~UiCamera();

public:
    Ui::UiCamera *ui;
};
#endif // UICAMERA_H
