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
    // Get handle for config file
    QFile jsonFile("/home/alexanderb/Downloads/temp_cam_sockets/config.json");
    // Read data into json object
    jsonFile.open(QFile::ReadOnly);
    // close file stream
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject configObject = jsonResponse.object();
    QJsonObject configJSON = configObject["devices"].toObject()["camera1"].toObject();

    qDebug() << configJSON["fps"].toInt();


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

        if (recv_bytes == CONN_PACK_SIZE)
        {
            qDebug() << connPack.cameraId << " has connected.";
            ConfigurationPacket defaultConfigPacket = {
                configJSON["device"].toString().toStdString(),
                configJSON["targetPort"].toString().toStdString(),
                configJSON["fps"].toInt(),
                configJSON["quality"].toInt(),
                configJSON["resolutionX"].toInt(),
                configJSON["resolutionY"].toInt(),
            };
            emit deviceConnected(QString(connPack.cameraId), new_fd);
//            idToSocketMap[std::string(connPack.cameraId)] = new_fd;

            uint8_t numPacks;
            uint8_t *serializedConfigPack = ConfigurationPacket::serialize(defaultConfigPacket, numPacks);

            sendConfiguration(new_fd, serializedConfigPack, numPacks);

            delete serializedConfigPack;
        }
    }
}

void TcpListenerThread::sendConfiguration(int socketFd, uint8_t *configurationBuffer, uint8_t numPacks)
{
    int result;
    std::cout << "sending configuration..." << std::endl;
    std::cout << sizeof(numPacks) << std::endl;
    result = send(socketFd, &numPacks, sizeof(numPacks), MSG_NOSIGNAL);
    if (result == -1)
    {
        perror("send");
    }

    printf("%hhu\n", numPacks);
    result = send(socketFd, configurationBuffer, numPacks, MSG_NOSIGNAL);
    if (result == -1)
    {
        perror("send");
    }
}
