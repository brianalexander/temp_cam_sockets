#ifndef MAINWINDOW
#define MAINWINDOW

#include <QMainWindow>
#include "onecamera.h"

#include <vector>
#include "videolistenerthread.h"
#include "tcplistenerthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void buildConfiguration(const QString& cameraId, int index);

    ~MainWindow();

signals:
    void forwardConfiguration(const QString& cameraId, ConfigurationPacket confPack);

public slots:
    void addDevice(QString cameraId, int fd);
    void removeDevice(QString cameraId, int fd);
    void buildOneConfiguration(const QString& cameraId);
    void buildConfigurations(const QString& configurationId);

private:
    Ui::MainWindow *ui;
    std::vector<VideoListenerThread*> *videoListenerThreads;
    TcpListenerThread *tcpListener;
};
#endif // MAINWINDOW
