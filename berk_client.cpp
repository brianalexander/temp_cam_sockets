/*
 * client.c -- a stream socket client demo
 * g++ berk_client.cpp -o berkcli -lpthread `pkg-config --cflags --libs opencv`
 */

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <random>
#include <cmath>

#include "opencv2/opencv.hpp"
#include "packetdefinitions.hpp"
#include "socketfunctions.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXIMUM_BACKOFF 32000

// function declarations
void tcpListener(int tcpSockFd);
void videoStreamWriter();

// global variables
ConfigurationPacket currentConfig;
char *g_hostPort;
char *g_hostIp;

std::mutex m_connected;
std::condition_variable condVar_connected;
bool g_connected = false;

// vars for thread synchronization
std::mutex m_configuration;
std::condition_variable condVar_configuration;
bool haveNewConfig = false;

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        fprintf(stderr, "usage: ./client cameraId hostname port\n");
        exit(1);
    }

    g_hostIp = argv[2];

    // start videoStreamWriter in paused state
    std::thread(videoStreamWriter).detach();

    //setup random number generator for exponential backoff
    std::random_device rd;
    std::ranlux24 rl24(rd());
    std::uniform_int_distribution<int> dist(1, 1000);
    int tcpSockFd;
    int connectionAttemptCount = 0;
    int waitTime = 0;

    // std::this_thread::sleep_for(std::chrono::seconds(1));

    ConnectionPacket connPack;
    memset(connPack.cameraId, '\0', sizeof(connPack.cameraId));
    strcpy(connPack.cameraId, argv[1]);

    for (;;)
    {
        if (-1 == (tcpSockFd = connectTcpSocketFd(argv[2], argv[3])))
        {
            std::cout << "TCP SOCKET FD" << tcpSockFd << std::endl;
            if (waitTime > MAXIMUM_BACKOFF)
            {
                waitTime = MAXIMUM_BACKOFF + dist(rl24);
            }
            else
            {
                waitTime = static_cast<int>(std::pow(2, connectionAttemptCount)) * 1000 + dist(rl24);
            }

            std::cout << "waiting before retry... " << waitTime << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
            ++connectionAttemptCount;
        }
        else
        {
            std::cout << "TCP SOCKET FD" << tcpSockFd << std::endl;
            g_connected = true;

            // start configuration listener
            std::thread(tcpListener, tcpSockFd).detach();

            // Give thread time to start, then send Id
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // Send user ID
            send(tcpSockFd, &connPack, sizeof(connPack), 0);

            // block until disconnected
            {
                // Wait until we lose our connection
                std::unique_lock<std::mutex> lk(m_connected);
                condVar_connected.wait(lk, [] { return !g_connected; });

                // release lock on mutex
                lk.release();
            }
            std::cout << "==== DISCONNECTED ====" << std::endl;
        }
    }
}

void tcpListener(int tcpSockFd)
{

    int recv_bytes;
    const int buflen = 2000000;
    uint8_t buf[buflen];

    for (;;) // infinite loop
    {
        std::cout << "waiting for new configuration" << std::endl;
        // get the incoming configuration data
        do
        {
            recv_bytes = recv(tcpSockFd, buf, PACK_SIZE, 0);
            std::cout << "received " << recv_bytes << " bytes" << std::endl;

            if (recv_bytes == -1 || recv_bytes == 0)
            {
                // after disconnecting, change ready to false
                {
                    std::lock_guard<std::mutex> lk(m_connected);
                    g_connected = false;
                }
                condVar_connected.notify_all();

                std::cout << "exiting tcp listener" << std::endl;
                close(tcpSockFd);
                return;
            }
        } while (recv_bytes != 2);

        // get the number of packets to be expected.
        unsigned short numPacks = ((unsigned short *)buf)[0];

        // get the configuration data
        std::cout << "waiting for config bytes" << std::endl;
        recv_bytes = recv(tcpSockFd, buf, PACK_SIZE, MSG_NOSIGNAL);

        // verify we got all the packets
        if (recv_bytes == numPacks)
        {
            // make sure we are in the correct state to accept a new configuration
            if (haveNewConfig == false)
            {
                currentConfig = ConfigurationPacket::deserialize(buf);
                std::cout << "deserialize " << currentConfig.targetPort << std::endl;
                printf("new configuration: device: %s port: %s FPS:%hhu QUAL:%hhu X:%hu Y:%hu\n",
                       currentConfig.device.c_str(),
                       currentConfig.targetPort.c_str(),
                       currentConfig.fps,
                       currentConfig.quality,
                       currentConfig.resolutionX,
                       currentConfig.resolutionY);

                std::cout << "NEWCONFIG" << haveNewConfig << std::endl;
                // after receiving a configuration, change ready to true
                {
                    std::lock_guard<std::mutex> lk(m_configuration);
                    haveNewConfig = true;
                }
                condVar_configuration.notify_one();

                std::cout << "NEWCONFIG" << haveNewConfig << std::endl;
            }
        }
    }
}

void videoStreamWriter()
{
    cv::VideoCapture vidCap;
    cv::Mat frame;
    std::vector<int> compression_params;
    std::vector<uchar> encoded;

    unsigned int numPacks;
    unsigned int numBytes;
    int bytesPerFrame;

    for (;;)
    {

        // Block until we receive configuration
        std::cout << "Blocking for new configuration" << std::endl;
        {
            std::unique_lock<std::mutex> lk(m_configuration);
            condVar_configuration.wait(lk, [] { return haveNewConfig; });

            // release lock on mutex
            lk.release();
        }

        int udpSockFd = connectUdpSocketFd(g_hostIp, currentConfig.targetPort.c_str());

        std::cout << "begin streaming" << std::endl;

        for (;;)
        {

            if (std::isdigit(currentConfig.device[0]))
            {
                vidCap.open(currentConfig.device[0] - '0');
            }
            else
            {
                vidCap.open(currentConfig.device);
            }

            if (!vidCap.isOpened())
            {
                std::cout << "failed to open device" << std::endl;
                exit(EXIT_FAILURE);
            }

            vidCap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

            vidCap.set(cv::CAP_PROP_FRAME_WIDTH, currentConfig.resolutionX);
            vidCap.set(cv::CAP_PROP_FRAME_HEIGHT, currentConfig.resolutionY);

            compression_params.clear();
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(currentConfig.quality);

            // reset haveNewConfig to false
            haveNewConfig = false;

            // main loop for broadcasting frames
            for (;;)
            {
                bytesPerFrame = 0;
                // std::cout << "streamingloop" << std::endl;
                // get a video frame from the camera
                vidCap >> frame;

                // if it's empty go back to the start of the loop
                if (frame.empty())
                    continue;

                // change formatting from BRG to RGB when opening images outside of openCV
                cv::cvtColor(frame, frame, CV_BGR2RGB);

                // use opencv to encode and compress the frame as a jpg
                cv::imencode(".jpg", frame, encoded, compression_params);

                // get the number of packets that need to be sent of the line
                numBytes = encoded.size();
                numPacks = (numBytes / PACK_SIZE) + 1;

                // send initial int that says how many more packets need to be read
                send(udpSockFd, &numBytes, sizeof(numBytes), MSG_NOSIGNAL);
                // udpSocket.send_to(asio::buffer(&numBytes, sizeof(numBytes)), remote_endpoint);

                for (int i = 0; i < numPacks; i++)
                {
                    // bytesPerFrame += udpSocket.send_to(asio::buffer(&encoded[i * PACK_SIZE], PACK_SIZE), remote_endpoint);
                    bytesPerFrame = send(udpSockFd, &encoded[i * PACK_SIZE], PACK_SIZE, MSG_NOSIGNAL);
                }

                // display bytes recieved and reset count to 0
                // printf("bytes sent : %i\n", bytesPerFrame);

                // at the end...
                if (haveNewConfig)
                {
                    std::cout << "new config available" << std::endl;
                    break;
                }

                if (!g_connected)
                {
                    close(udpSockFd);
                    vidCap.release();
                    break;
                }

                // sleep for an appropriate amount of time to send out the desired FPS
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / currentConfig.fps));
            }

            if (!g_connected)
            {
                break;
            }
        }
    }
}

// if (!haveNewConfig)
// {
// }
// else // configuration received. start stream & block until we need to try and reconnect
// {
//     // reset connection attemps
//     connectionAttemptCount = 0;

//     {
//         // Wait until we lose our connection
//         std::unique_lock<std::mutex> lk(m);
//         condVar.wait(lk, [] { return lostConnection; });

//         // release lock on mutex
//         lk.release();
//     }
// }

// int main(int argc, char *argv[])
// {
//     int sockfd, numbytes;
//     char buf[MAXDATASIZE];
//     struct addrinfo hints, *servinfo, *p;
//     int rv;
//     char s[INET6_ADDRSTRLEN];

//     if (argc != 2)
//     {
//         fprintf(stderr, "usage: client hostname\n");
//         exit(1);
//     }

//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;

//     if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
//     {
//         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
//         return 1;
//     }

//     // loop through all the results and connect to the first we can
//     for (p = servinfo; p != NULL; p = p->ai_next)
//     {
//         if ((sockfd = socket(p->ai_family, p->ai_socktype,
//                              p->ai_protocol)) == -1)
//         {
//             perror("client: socket");
//             continue;
//         }

//         if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
//         {
//             close(sockfd);
//             perror("client: connect");
//             continue;
//         }

//         break;
//     }

//     if (p == NULL)
//     {
//         fprintf(stderr, "client: failed to connect\n");
//         return 2;
//     }

//     inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
//               s, sizeof s);
//     printf("client: connecting to %s\n", s);

//     freeaddrinfo(servinfo); // all done with this structure
//     for (;;)
//     {
//         if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
//         {
//             perror("recv");
//             exit(1);
//         }

//         buf[numbytes] = '\0';

//         printf("client: received '%s'\n", buf);
//     }

//     close(sockfd);

//     return 0;
// }