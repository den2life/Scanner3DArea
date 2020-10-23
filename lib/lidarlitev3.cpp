#include "lidarlitev3.h"



/**
 * Функция чтения 8битов
 * @param reg Регистр Lidar Lite v3
 * @param value Значение для записи в регистр
 * */
void LidarLiteV3::writeRegister8(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(HW_ADDR_LIDAR_LITE);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value));
    Wire.endTransmission();
}

uint8_t LidarLiteV3::readRegister8(uint8_t reg)
{
    Wire.beginTransmission(HW_ADDR_LIDAR_LITE);
    Wire.write(reg);
    Wire.endTransmission(false); // MMA8451 + friends uses repeated start!!
    Wire.requestFrom(HW_ADDR_LIDAR_LITE, 1);
    if (! Wire.available()) return -1;
    return (Wire.read());
}

uint16_t LidarLiteV3::readRegister16(uint8_t reg)
{
    uint8_t buf[2];
    Wire.beginTransmission(HW_ADDR_LIDAR_LITE);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(HW_ADDR_LIDAR_LITE, 2);
    Wire.readBytes(buf, 2);
    // if (! Wire.available()) return -1;
    return (buf[1] << 8) | buf[0];
}

/**
 * Функция для получения дистанции
 * @return 0-40000 см.
 * */
uint16_t LidarLiteV3::getDistance(void)
{
    uint16_t lidarDistance;
    this->writeRegister8(0x00, 0x04);
    while(isBusy());
    lidarDistance = this->readRegister8(0x0f) << 8;
    lidarDistance |= this->readRegister8(0x10);
    return lidarDistance;
}

/**
 * Функция для получения среднего значения дистанции
 * @param avarage 0-255 Значение накопления среднего
 * @return 0-40000 см.
 * */
uint16_t LidarLiteV3::getDistanceWithAvarage(uint8_t avarage)
{
    uint16_t lidarDistance;
    unsigned long int bufLidar = 0;
    for (uint8_t i = 0; i < avarage; i++)
    {
        this->writeRegister8(0x00, 0x04);
        while(isBusy());
        lidarDistance = this->readRegister8(0x0f) << 8;
        lidarDistance |= this->readRegister8(0x10);
        bufLidar += lidarDistance; 
    }
    
    return bufLidar / avarage;
}
