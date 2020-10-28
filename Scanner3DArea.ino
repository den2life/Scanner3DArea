#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h>
#include <EEPROM.h>
#include "lidarlitev3.h"
#include <math.h>
#include <GyverStepper.h>
#include "GyverTimers.h"

/**
 * Написал два метода, уголовой и шаговый, они по своей сути одиноковы, просто градусами удобнее работать до 0.5гр, дальшще там погрешности могут быть поэтому
 * менее точный метод и шаговый Шаговый более точный, так как программа пытается использовать каждый шаг для сканирования объекта.
 * */


#define SCANNER3D_DEGREE_METHODE	0		//1-Уголовой метод 0-Шаговый метод

#if SCANNER3D_DEGREE_METHODE > 0	//Уголовой метод сканирования (менее точный)

GStepper <STEPPER4WIRE> arroundX(4800, 11, 10, 9, 8);	//Двигатель 28byj-48-5v высчислено на глаз 4800 шагов/оборот. Видимо шестерня "редуцирует" и вносит такие показания на кол-во шагов
GStepper <STEPPER4WIRE> arroundY(4800, 7, 6, 5, 4);
      

int stepCountX = 0;         // number of steps the motor has taken
int stepCountY = 0;         // number of steps the motor has taken
int speedCount = 0;         // number of steps the motor has taken

LidarLiteV3 Lidarlite;
uint32_t tmr2;
uint32_t tmr3;
float getCurrentDeg;
void setup()
{
	Serial.begin(9600);

	arroundX.setRunMode(KEEP_SPEED); // режим поддержания скорости
	arroundX.setSpeedDeg(24);        // в градусах/сек
	arroundX.setCurrent(0);

	arroundY.setRunMode(FOLLOW_POS); 
	arroundY.setSpeedDeg(50);        // в градусах/сек
	arroundY.setCurrent(0);
	//arroundX.setTargetDeg(360, RELATIVE);

	Timer2.setPeriod(1000);
	Timer2.enableISR();
}
void loop()
{
	// getCurrentDeg = arroundX.getCurrentDeg();
	if ((int)arroundX.getCurrentDeg() % 360 == 0)	//Постоянно крутим Ось Б если пройдет круг, то поднимаем ось А
	{
		if (!tmr2)
		{
			Serial.println(arroundX.getCurrentDeg());
			arroundY.setTargetDeg(stepCountY);
			stepCountY += 1;
			if (stepCountY == 270)	//Если ось А просканирует 270гр, то нужно парковаться и заканчивать работу
			{

			}
			tmr2 = 1;
		}
		
		// stepCountX += 10;
		// 
		// arroundX.setTargetDeg(stepCountX, RELATIVE);
	}
	else
	{
		tmr2 = 0;
	}
	
	if ((int)(modff(getCurrentDeg = arroundX.getCurrentDeg(), NULL) * 10) % 5 == 0)	//Замеряем дистанцию каждые 0.5градусов всего 720 точек
	{
		if (!tmr3)
		{
			Serial.print("Degree: ");
			Serial.println(getCurrentDeg);
			tmr3 = 1;
		}
		
	}
	else
	{
		tmr3 = 0;
	}
	
    // if (millis() - tmr2 > 50)
    // {
		
		
	// 	tmr2 = millis();
    // }
	
    // Serial.print("Distance: ");
    // Serial.print(Lidarlite.getDistance());
    // Serial.println("cm");
}

// обработчик
ISR(TIMER2_A)
{
	arroundX.tick();
	arroundY.tick();
}
#else		//Шаговый метод сканирования (более точный)

GStepper< STEPPER4WIRE> arroundX(4800, 11, 10, 9, 8);
GStepper <STEPPER4WIRE> arroundY(4800, 7, 6, 5, 4);
      

int stepCountX = 0;         // number of steps the motor has taken
int stepCountY = 0;         // number of steps the motor has taken
int speedCount = 0;         // number of steps the motor has taken

LidarLiteV3 Lidarlite;
uint32_t tmr2;
uint32_t tmr3;
uint32_t tmr4;
float getCurrentDeg;
void setup()
{
	Serial.begin(9600);

	arroundX.setRunMode(KEEP_SPEED); // режим поддержания скорости
	arroundX.setSpeedDeg(24);        // в градусах/сек
	arroundX.setCurrent(0);

	arroundY.setRunMode(FOLLOW_POS); // режим поддержания скорости
	arroundY.setSpeedDeg(50);        // в градусах/сек
	arroundY.setCurrent(0);
	//arroundX.setTargetDeg(360, RELATIVE);

	Timer2.setPeriod(100);
	Timer2.enableISR();
}
void loop()
{
	// arroundX.setTarget(1, RELATIVE);		//Если делать все 4800 шагов, но это долго.
	// getCurrentDeg = arroundX.getCurrentDeg();
	if ((int)arroundX.getCurrentDeg() % 360 == 0)	//Постоянно крутим Ось Б если пройдет круг, то поднимаем ось А
	{
		if (!tmr2)
		{
			Serial.println(arroundX.getCurrentDeg());
			Serial.println(arroundX.getCurrent());
			arroundY.setTargetDeg(stepCountY, RELATIVE);
			stepCountY += 1;
			if (stepCountY == 270)	//Если ось А просканирует 270гр, то нужно парковаться и заканчивать работу
			{

			}
			tmr2 = 1;
		}
	}
	else
	{
		tmr2 = 0;
	}
	
	if (arroundX.getCurrent() - tmr4 >= 1)	//Здесь будем пытатся сканировать дистанцию каждый шаг
	{
		tmr4 = arroundX.getCurrent();
		// Serial.print("Distance: ");
		// Serial.print(Lidarlite.getDistance());
		// Serial.println("cm");
	}

    
}

// обработчик
ISR(TIMER2_A)
{
	arroundX.tick();
	arroundY.tick();
}
#endif

