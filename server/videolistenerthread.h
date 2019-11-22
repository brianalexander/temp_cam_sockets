#ifndef VIDEOLISTENERTHREAD_H
#define VIDEOLISTENERTHREAD_H

// Networking
#include "../packetdefinitions.hpp"
#include "../socketfunctions.hpp"

#include <QThread>

// OpenCV
#include "opencv2/opencv.hpp"

class VideoListenerThread : public QThread
{
    Q_OBJECT
public:
    void run(void);
    void setPort(const char* port) {
        m_port = port;
    }
    void setId(int id) {
        m_id = id;
    }

signals:
    void frameCompleted(cv::Mat frame);
private:
    const char* m_port;
    int m_id;
};

#endif // VIDEOLISTENERTHREAD_H
