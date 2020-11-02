#include <Arduino.h>
#include "OneWire.h"
#include <Wire.h>
#include <EEPROM.h>
#include "lidarlitev3.h"
#include <math.h>
#include "GyverStepper.h"
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
      
uint16_t stepCountY = 0;         // number of steps the motor has taken

LidarLiteV3 Lidarlite;
bool tmr2= true;

long bufferData = 0;
uint32_t tmr4;
uint8_t CountForRawData = 0;
float getCurrentDeg;
bool flagRxTime = false;
bool isStartEnabled = true;	//Запускаем двигатели
uint8_t data1[2];
volatile uint8_t incomingBytes = 0;
unsigned long BufferSysTick = 0, BufferSysTick2 = 0, BufferSysTick3 = 0;
uint8_t cmdStartScanFromPC = 0;

void UART_WriteByte(uint8_t data)
{
	Serial.write(data);
	Serial.flush();
}

void UART_WriteBurst(uint8_t *data, uint16_t len)
{
	Serial.write(data, len);
	Serial.flush();
}

void UART_Transmit16(uint16_t data)
{
	UART_WriteByte((data & 0xFF00) >> 8);
	UART_WriteByte((data & 0xFF));
}

void UART_TransmitPacket()
{
	uint8_t arr[7];

	bufferData = arroundY.getCurrent();
	arr[0] = (bufferData & 0xFF00) >> 8;
	arr[1] = (bufferData & 0xFF);
	bufferData = arroundX.getCurrent();
	bufferData = bufferData - ((bufferData / 4800) * 4800);
	arr[2] = (bufferData & 0xFF00) >> 8;
	arr[3] = (bufferData & 0xFF);
	bufferData = Lidarlite.getDistance();
	arr[4] = (bufferData & 0xFF00) >> 8;
	arr[5] = (bufferData & 0xFF);
	arr[6] = OneWire::crc8(arr, 6);
	UART_WriteBurst(arr, 7);
}

uint8_t UART_Receive(void)
{
 	int result = 0;

	if (Serial.available())
	{
		flagRxTime = true;
		
		incomingBytes = Serial.read();
		BufferSysTick3 = 0;
		
	}
	if (flagRxTime)
	{
		if (!BufferSysTick3)
		{
			BufferSysTick3 = millis();
		}
		else if ((millis() - BufferSysTick3) > 100)
		{
			if (incomingBytes == 0x01)
			{
				result = 1;
				UART_WriteByte(0xA1);
			}
				
			flagRxTime = false;
			BufferSysTick3 = 0;
		}
	}
	return result;
}

uint8_t MainProcess(void)
{
	if (((int)arroundX.getCurrentDeg() % 360 == 0))	//Постоянно крутим Ось Б если пройдет круг, то поднимаем ось А
	{
		if (!tmr2)
		{
			
			// arroundX.brake();
			// Serial.println(arroundX.getCurrentDeg());
			// Serial.println(arroundX.getCurrent());
			stepCountY += 1;
			if (stepCountY == 270)	//Если ось А просканирует 270гр, то нужно парковаться и заканчивать работу
			{
				return 1;
			}
			arroundY.setTarget(stepCountY, RELATIVE);	//Ось А на шаг больше
			
			
			tmr2 = true;
			// arroundX.setRunMode(KEEP_SPEED); // режим поддержания скорости
			// arroundX.setSpeedDeg(24);        // в градусах/сек
			// arroundX.setCurrent(0);
			
		}
	}
	else
	{
		tmr2 = false;
	}
	#if 1
	if (arroundX.getCurrent() - tmr4 >= 2)	//Здесь будем пытатся сканировать дистанцию каждый шаг
	{
		tmr4 = arroundX.getCurrent();
		// Serial.println(CountForRawData);
		/* if (CountForRawData++ >= 100)	//Каждые 100байт выгружать данные в ПК
		{
			CountForRawData = 0;
			arroundX.brake();
			Serial.println("Data");
			UART_WriteByte(arroundY.getCurrentDeg());	//Угол оси А
			for (uint8_t i = 0; i < 100; i++)
			{
				for (uint8_t j = 0; j < 2; j++)
				{
					data1[0] = (raw_data[i][j] & 0xFF00) >> 8;
					data1[1] = raw_data[i][j] & 0xFF;
					UART_WriteBurst(data1, 2);
				}
			}
			arroundX.setRunMode(KEEP_SPEED);
			isStartEnabled = true;	//Запускаем двигатели
		} */
		UART_TransmitPacket();
		// Serial.print(Lidarlite.getDistance());
		// Serial.print(" | ");
		// Serial.println(arroundX.getCurrentDeg());
		// Serial.print("Distance: ");
		// Serial.print(Lidarlite.getDistance());
		// Serial.println("cm");
		
	}
	#endif
	// if (isStartEnabled)
	// {
	// 	Serial.println("Start axis B");
	// 	arroundX.setTarget(100, RELATIVE);
	// 	isStartEnabled = false;
	// }

	// arroundX.setTarget(1, RELATIVE);		//Если делать все 4800 шагов, но это долго.
	// getCurrentDeg = arroundX.getCurrentDeg();
	return 0;
}
void setup()
{
	Serial.begin(115200);

	
	

	arroundY.setRunMode(FOLLOW_POS);
	arroundY.setSpeedDeg(50);        // в градусах/сек
	arroundY.setCurrentDeg(0);
	// arroundY.setTargetDeg(90, RELATIVE);	//Ось А от начала идет на 45гр
	Timer2.setPeriod(4000);
	Timer2.enableISR();
}
void loop()
{
	switch (cmdStartScanFromPC)
	{
		case 0:
		cmdStartScanFromPC = UART_Receive();
		
		break;

		case 1:
		delay(2000);
		arroundX.setRunMode(KEEP_SPEED); // режим поддержания скорости
		arroundX.setSpeedDeg(24);        // в градусах/сек
		arroundX.setCurrent(0);
		cmdStartScanFromPC = 2;
		break;

		case 2:
		// cmdStartScanFromPC = 0;
		if (MainProcess()) cmdStartScanFromPC = 0;
		break;

		default:
		break;
	}
	

    
}

// обработчик
ISR(TIMER2_A)
{
	arroundX.tick();
	arroundY.tick();

	
}
#endif

