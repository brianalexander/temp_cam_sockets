#include "tcplistenerthread.h"

// Configuration
#include <iostream>
#include <fstream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

// Networking
#include "../packetdefinitions.hpp"
#include "../socketfunctions.hpp"

void TcpListenerThread::run()
{
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    int recv_bytes;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

    ConnectionPacket connPack;

    sockfd = bindTcpSocketFd(m_port);
    qDebug() << "server: waiting for connections...";

    for (;;)
    { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        // Verify their information in the config file
        recv_bytes = recv(new_fd, &connPack, CONN_PACK_SIZE, 0);
        if(recv_bytes == -1 || recv_bytes == 0) {
            perror("recv");
            close(new_fd);
            continue;
        }

        if (recv_bytes == CONN_PACK_SIZE)
        {
            qDebug() << connPack.cameraId << " has connected.";
            connectedDevices[QString(connPack.cameraId)] = new_fd;
            emit deviceConnected(QString(connPack.cameraId), new_fd);
        }
    }
}

void TcpListenerThread::sendConfiguration(const QString& cameraId)
{
    // Get handle for config file
    QFile jsonFile("/home/alexanderb/Downloads/temp_cam_sockets/config.json");
    // Read data into json object
    jsonFile.open(QFile::ReadOnly);
    // close file stream
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject configObject = jsonResponse.object();
    QJsonObject configJSON = configObject["devices"].toObject()[cameraId].toObject();

    ConfigurationPacket defaultConfigPacket = {
        configJSON["device"].toString().toStdString(),
        configJSON["targetPort"].toString().toStdString(),
        configJSON["fps"].toInt(),
        configJSON["quality"].toInt(),
        configJSON["resolutionX"].toInt(),
        configJSON["resolutionY"].toInt(),
    };

    uint8_t numPacks;
    uint8_t *serializedConfigPack = ConfigurationPacket::serialize(defaultConfigPacket, numPacks);

    int result;
    qDebug() << "sending configuration...";
    qDebug() << sizeof(numPacks);
    result = send(connectedDevices[cameraId], &numPacks, sizeof(numPacks), MSG_NOSIGNAL);
    if (result == -1)
    {
        perror("send");
    }

    printf("%hhu\n", numPacks);
    result = send(connectedDevices[cameraId], serializedConfigPack, numPacks, MSG_NOSIGNAL);
    if (result == -1)
    {
        perror("send");
    }

    delete serializedConfigPack;
}
