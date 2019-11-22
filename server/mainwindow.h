#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include "onecamera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void addDevice(QString cameraId, int fd);
    void removeDevice(QString cameraId);

public:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW
