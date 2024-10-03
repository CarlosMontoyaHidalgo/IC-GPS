#include <SPI.h>
#include <LoRa.h>

typedef union gpsCoord
{
    float floatValue;
    byte byteValue[4];
};

byte localAddress = 0xFF;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println("LoRa GPS data receiver");
    if (!LoRa.begin(868E6))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
    delay(1000);
}

void loop()
{

    onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize)
{
    if (packetSize == 0)
        return; // if there's no packet, return

    int sender = LoRa.read();
    int recipient = LoRa.read();

    gpsCoord latitude;
    for (int i = 0; i < 4; i++)
        latitude.byteValue[i] = LoRa.read();

    gpsCoord longitude;
    for (int i = 0; i < 4; i++)
        longitude.byteValue[i] = LoRa.read();

    String incoming = "";
    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }

    if (recipient != localAddress && recipient != 0xFF)
    {
        Serial.println("This message is not for me.");
        return; // skip rest of function
    }

    Serial.print("DeviceID: ");
    Serial.print(sender);
    Serial.print("; Payload:");
    Serial.print(" LAT: ");
    Serial.print(latitude.floatValue, 6);
    Serial.print(" LONG: ");
    Serial.print(longitude.floatValue, 6);
    Serial.print(incoming);
    Serial.print(" || RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println();
}