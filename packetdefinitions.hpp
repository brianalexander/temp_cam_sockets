// typedef uint8_t byte;
#include <vector>
#include <string>
#include <iostream>

#define PACK_SIZE 4096

class ConfigurationPacket
{
public:
    std::string device;
    std::string targetPort;
    uint8_t fps;
    uint8_t quality;
    uint16_t resolutionX;
    uint16_t resolutionY;

    static uint8_t *serialize(ConfigurationPacket &packet, uint16_t &numPacks);
    static ConfigurationPacket deserialize(uint8_t *buffer);
};

ConfigurationPacket ConfigurationPacket::deserialize(uint8_t *buffer)
{
    ConfigurationPacket newPacket;
    for (; *buffer != '\0'; ++buffer)
    {
        newPacket.device.push_back(*buffer);
    }

    ++buffer;
    for (; *buffer != '\0'; ++buffer)
    {
        newPacket.targetPort.push_back(*buffer);
    }

    ++buffer;
    newPacket.fps = *buffer;

    ++buffer;
    newPacket.quality = *buffer;

    ++buffer;
    newPacket.resolutionX = (buffer[0] << 8 | buffer[1]);

    buffer += 2;
    newPacket.resolutionY = (buffer[0] << 8 | buffer[1]);

    return newPacket;
};

uint8_t *ConfigurationPacket::serialize(ConfigurationPacket &packet, uint16_t &numPacks)
{
    std::vector<uint8_t> byteBuffer;

    // add device string to buffer
    std::copy(packet.device.begin(), packet.device.end(), std::back_inserter(byteBuffer));
    byteBuffer.push_back('\0');

    std::copy(packet.targetPort.begin(), packet.targetPort.end(), std::back_inserter(byteBuffer));
    byteBuffer.push_back('\0');

    byteBuffer.push_back(packet.fps);
    byteBuffer.push_back(packet.quality);

    uint8_t *byteResX = static_cast<uint8_t *>(static_cast<void *>(&packet.resolutionX));
    byteBuffer.push_back(byteResX[1]);
    byteBuffer.push_back(byteResX[0]);

    uint8_t *byteResY = static_cast<uint8_t *>(static_cast<void *>(&packet.resolutionY));
    byteBuffer.push_back(byteResY[1]);
    byteBuffer.push_back(byteResY[0]);

    numPacks = byteBuffer.size();

    uint8_t *serializedData = (uint8_t *)malloc(sizeof(uint8_t) * byteBuffer.size());
    std::copy(byteBuffer.begin(), byteBuffer.end(), serializedData);

    return serializedData;
};

#define CONN_PACK_SIZE 26

struct ConnectionPacket
{
    char cameraId[CONN_PACK_SIZE];
};