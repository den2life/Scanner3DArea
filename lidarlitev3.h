#ifndef _LIDARLITEV3_H_
#define _LIDARLITEV3_H_

#include <Arduino.h>
#include <Wire.h>

#define HW_ADDR_LIDAR_LITE 0x62


#define LIDARLITEV3_ADDR_REG_ACQ_COMMAND        0x00
#define LIDARLITEV3_ADDR_REG_STATUS             0x01
#define LIDARLITEV3_ADDR_REG_SIG_COUNT_VAL      0x02
#define LIDARLITEV3_ADDR_REG_ACQ_CONFIG         0x04
#define LIDARLITEV3_ADDR_REG_VELOCITY           0x09
#define LIDARLITEV3_ADDR_REG_PEAK_CORR          0x0C
#define LIDARLITEV3_ADDR_REG_NOISE_PEAK         0x0D
#define LIDARLITEV3_ADDR_REG_SIGNAL_STRENGTH    0x0E
#define LIDARLITEV3_ADDR_REG_FULL_DELAY_HIGH	0x0F
#define LIDARLITEV3_ADDR_REG_FULL_DELAY_LOW		0x10
#define LIDARLITEV3_ADDR_REG_OUTER_LOOP_COUNT	0x11
#define LIDARLITEV3_ADDR_REG_REF_COUNT_VAL		0x12
#define LIDARLITEV3_ADDR_REG_LAST_DELAY_HIGH	0x14
#define LIDARLITEV3_ADDR_REG_LAST_DELAY_LOW		0x15
#define LIDARLITEV3_ADDR_REG_UNIT_ID_HIGH		0x16
#define LIDARLITEV3_ADDR_REG_UNIT_ID_LOW		0x17


class LidarLiteV3
{
    public: 
    LidarLiteV3 ()
    {
        Wire.begin();
    }
    
    uint8_t getSystemStatusReg(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS);
    }

    bool isProcessError(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x40;
    }
    bool isHealth(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x20;
    }
    bool isSecondaryReturn(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x10;
    }
    bool isInvalidSignal(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x08;
    }
    bool isSignalOverflow(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x04;
    }
    bool isReferenceOverflow(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x02;
    }
    bool isBusy(void)
    {
        return this->readRegister8(LIDARLITEV3_ADDR_REG_STATUS) & 0x01;
    }
    uint16_t getDistance(void);

    private:
    void writeRegister8(uint8_t reg, uint8_t value);
    uint8_t readRegister8(uint8_t reg);
    uint16_t readRegister16(uint8_t reg);

};

#endif  //LIDARLITEV3
