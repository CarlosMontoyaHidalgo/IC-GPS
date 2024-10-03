#pragma once
#ifndef __MESSAGING_HPP__
#define __MESSAGING_HPP__

#include <Arduino.h>
#include <LoRa.h>

enum packageType
{
    GPS,
    Text,
};

typedef struct
{
    uint8_t bandwidth_index; // ANCHO DE BANDA
    uint8_t spreadingFactor; // FACTOR DE PROPAGACIÓN
    uint8_t codingRate;      // TASA DE CODIFICACIÓN
    uint8_t txPower;         // POTENCIA DE TRANSMISIÓN
} LoRaConfig_t;

class Message
{
public:
    typedef uint8_t addressType;
    Message(void *data, size_t length, addressType destination, packageType type);
    ~Message();
    const void *getData() const;
    size_t getLength() const;
    packageType getType() const;
    addressType getDestination() const;

private:
    addressType destination;
    void *data;
    size_t length;
    packageType type;
};

class LoRaMessagingSystem
{
public:
    LoRaMessagingSystem(Message::addressType localAddress);
    LoRaMessagingSystem(Message::addressType localAddress, LoRaConfig_t config);
    ~LoRaMessagingSystem();
    Message *getMessage();
    void sendMessage(Message &message);
    void loop();

private:
    const Message::addressType localAddress;
    uint16_t msgCount = 0;
    volatile bool txDoneFlag = true;
    volatile bool transmitting = false;
    unsigned long tx_begin_ms = 0;
    unsigned long tx_next_begin_ms = 0;
    // unsigned long tx_interval_time_ms = 1000;
    const LoRaConfig_t nodeConf = {6, 10, 5, 2};

    void onReceive();
    void TxFinished();
    void init();
};

#endif // __MESSAGING_HPP__