#include <SPI.h>
#include <LoRa.h>
#include <Arduino_MKRGPS.h>
#include <RTCZero.h>

typedef union gpsCoord
{
    float floatValue;
    byte byteValue[4];
};

gpsCoord latitude;
gpsCoord longitude;
float altitude;
float speed;
int satellites;

volatile bool read_gps = true;

byte localAddress = 0x02; // address of this device
byte destination = 0xFF;  // where we are sending data to

volatile uint32_t _period_sec = 0;

#define DEBUG

#ifdef DEBUG
constexpr unsigned long period_time = 5000;
unsigned long lastSendTime = millis();
#endif

void setup()
{
#ifdef DEBUG
    // initialize serial communications and wait for port to open:
    Serial.begin(9600);

    while (!Serial)
        ;
#endif

    if (!LoRa.begin(868E6))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }

    if (!GPS.begin())
    {

#ifdef DEBUG
        Serial.println("Failed to initialize GPS!");
#endif
        while (1)
            ;
    }

    setPeriodicAlarm(10, 5);

    rtc.attachInterrupt(alarmCallback);
}

void loop()
{
    // check if there is new GPS data available
    if (read_gps && GPS.available())
    {
        // read GPS values
        latitude.floatValue = GPS.latitude();
        longitude.floatValue = GPS.longitude();
        altitude = GPS.altitude();
        speed = GPS.speed();
        satellites = GPS.satellites();

        read_gps = false;

#ifdef DEBUG
        // print GPS values
        printValues();
#endif

        LoRa_send();
    }
#ifdef DEBUG
    else
    {
        satellites = GPS.satellites();

        unsigned long currentTime = millis();
        if (currentTime - lastSendTime > period_time)
        {
            lastSendTime = currentTime;
            Serial.println("\n\nSending packet\n\n");
            printValues();
            LoRa_send();
        }
    }
#endif

    delay(1);
}

// function to send information over LoRa network
void LoRa_send()
{
    LoRa.beginPacket(); // creates a LoRa packet
    LoRa.write(localAddress);
    LoRa.write(destination); // destination address
    LoRa.write(latitude.byteValue, 4);
    LoRa.write(longitude.byteValue, 4);
#ifdef DEBUG
    LoRa.print(" SAT: ");
    LoRa.print(satellites);
#endif
    LoRa.endPacket(); // sends the LoRa packet
}

#ifdef DEBUG
// function that prints all readings in the Serial Monitor
void printValues()
{
    Serial.print("Location: ");
    Serial.print(latitude.floatValue, 7);
    Serial.print(", ");
    Serial.println(longitude.floatValue, 7);
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println("m");
    Serial.print("Ground speed: ");
    Serial.print(speed);
    Serial.println(" km/h");
    Serial.print("Number of satellites: ");
    Serial.println(satellites);
    Serial.println();
}
#endif

// --------------------------------------------------------------------------------
// Programa la alarma del RTC para que se active de en period_sec segundos a
// partir de "offset" en segundos desde el instante actual
// --------------------------------------------------------------------------------
void setPeriodicAlarm(uint32_t period_sec, uint32_t offsetFromNow_sec)
{
    _period_sec = period_sec;
    rtc.setAlarmEpoch(rtc.getEpoch() + offsetFromNow_sec);

    // Ver enum Alarm_Match en RTCZero.h
    rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS);
}

// --------------------------------------------------------------------------------
// Rutina de servicio asociada a la interrupción provocada por la expiración de la
// alarma.
// --------------------------------------------------------------------------------
void alarmCallback()
{
    // Incrementamos la variable bandera
    read_gps = true;

    // Reprogramamos la alarma usando el mismo periodo
    rtc.setAlarmEpoch(rtc.getEpoch() + _period_sec);
}