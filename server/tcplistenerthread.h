#ifndef TCPLISTENERTHREAD_H
#define TCPLISTENERTHREAD_H

// Networking
#include "../packetdefinitions.hpp"
#include "../socketfunctions.hpp"

#include <QThread>

class TcpListenerThread : public QThread
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
    void sendConfiguration(int socketFd, uint8_t *configurationBuffer, uint8_t numPacks);

signals:
    void deviceConnected(QString cameraId, int socketFd);

private:
    const char* m_port;
    int m_id;
};

#endif // TCPLISTENERTHREAD_H
