#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h>
#include <EEPROM.h>
#include "lidarlitev3.h"

LidarLiteV3 Lidarlite;

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    Serial.print("Distance: ");
    Serial.print(Lidarlite.getDistance());
    Serial.println("cm");

    delay(500);
}
