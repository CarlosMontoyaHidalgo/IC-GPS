#include "messaging.hpp"

static constexpr double bandwidth_kHz[10] = {7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,
                                             41.7E3, 62.5E3, 125E3, 250E3, 500E3};

Message::Message(void *data, size_t length, addressType destination, packageType type) : destination{destination}, type{type}, length{length}
{
    this->data = malloc(length);      // Assign memory
    memcpy(this->data, data, length); // Copy
}

Message::~Message()
{
    free(this->data); // Free memory
}

const void *Message::getData() const
{
    return this->data;
}

size_t Message::getLength() const
{
    return this->length;
}

packageType Message::getType() const
{
    return this->type;
}

Message::addressType Message::getDestination() const
{
    return this->destination;
}

void LoRaMessagingSystem::onReceive()
{
}

void LoRaMessagingSystem::TxFinished()
{
#ifdef DEBUG
    Serial.println("TxFinished");
#endif
    this->txDoneFlag = true;
}

void LoRaMessagingSystem::loop()
{
    if (transmitting && txDoneFlag)
    {
        uint32_t TxTime_ms = millis() - this->tx_begin_ms;

#ifdef DEBUG
        Serial.println("loop tx done ");
        Serial.print("----> TX completed in ");
        Serial.print(TxTime_ms);
        Serial.println(" msecs");
#endif

        /*
                // Ajustamos txInterval_ms para respetar un duty cycle del 1%
                uint32_t lapse_ms = tx_begin_ms - lastSendTime_ms;
                lastSendTime_ms = tx_begin_ms;
                float duty_cycle = (100.0f * TxTime_ms) / lapse_ms;

        #ifdef DEBUG
                Serial.print("Duty cycle: ");
                Serial.print(duty_cycle, 1);
                Serial.println(" %\n");
        #endif

                // Solo si el ciclo de trabajo es superior al 1% lo ajustamos
                if (duty_cycle > 1.0f)
                {
                    this->tx_interval_time_ms = TxTime_ms;
                }
         */
        this->transmitting = false;

        LoRa.receive();
    }
}

void LoRaMessagingSystem::init()
{
    while (!LoRa.begin(868E6))
    {
        delay(10);
#ifdef DEBUG
        Serial.println("LoRa init failed. Check your connections.");
#endif
    }

    LoRa.setSignalBandwidth(long(bandwidth_kHz[this->nodeConf.bandwidth_index]));

    LoRa.setSpreadingFactor(this->nodeConf.spreadingFactor);

    LoRa.setCodingRate4(this->nodeConf.codingRate);

    LoRa.setTxPower(this->nodeConf.txPower, PA_OUTPUT_PA_BOOST_PIN);

    LoRa.setSyncWord(0x12);
    LoRa.setPreambleLength(8); // Número de símbolos a usar como preámbulo

    // Indicamos el callback para cuando se reciba un paquete

    LoRa.receive();

#ifdef DEBUG
    Serial.println("LoRa init succeeded.\n");
#endif
}

LoRaMessagingSystem::LoRaMessagingSystem(Message::addressType localAddress) : localAddress{localAddress}
{
    this->init();
}

LoRaMessagingSystem::LoRaMessagingSystem(Message::addressType localAddress, LoRaConfig_t config) : localAddress{localAddress}, nodeConf{config}
{
    this->init();
}

Message *LoRaMessagingSystem::getMessage()
{
}

#ifdef DEBUG
static void printBinaryPayload(void *payload, size_t payloadLength)
{
    for (int i = 0; i < payloadLength; i++)
    {
        Serial.print((payload[i] & 0xF0) >> 4, HEX);
        Serial.print(payload[i] & 0x0F, HEX);
        Serial.print(" ");
    }
}
#endif

void LoRaMessagingSystem::sendMessage(Message &message)
{
    while (!LoRa.beginPacket()) // The packaging of message is started
        delay(10);              //

    this->transmitting = true;
    this->txDoneFlag = false;
    this->tx_begin_ms = millis();

    LoRa.write(message.getDestination());
    LoRa.write(this->localAddress);
    LoRa.write((uint8_t)(this->msgCount >> 7));
    LoRa.write((uint8_t)(this->msgCount & 0xFF));
    this->msgCount++;

    LoRa.write(message.getLength());
    LoRa.write((uint8_t *)message.getData(), message.getLength());
    LoRa.endPacket(true);

#ifdef DEBUG
    Serial.print("Sent message ");
    Serial.print(this->msgCount);
    Serial.print(": ");
    printBinaryPayload(message.getData(), message.getLength());

    Serial.println();
#endif
}