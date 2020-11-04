#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h>
#include <EEPROM.h>
#include "lidarlitev3.h"
#include <math.h>
#include "GyverStepper.h"
#include "GyverTimers.h"


GStepper< STEPPER4WIRE> arroundX(4800, 11, 10, 9, 8);
GStepper <STEPPER4WIRE> arroundY(2100, 7, 6, 5, 4);
      
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
bool axisYState = false;
bool axisXState = false;
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
			if (arroundY.getTargetDeg() >= 180)	//Если ось А просканирует 90гр, то нужно парковаться и заканчивать работу
			{
				arroundX.setRunMode(FOLLOW_POS); // режим поддержания скорости
				arroundX.setSpeedDeg(24);        // в градусах/сек
				arroundY.setTarget(-arroundY.getCurrent(), RELATIVE);
				arroundX.setTarget(-(round(modff((float)((float)arroundX.getCurrent()/4800), NULL) * 4800)), RELATIVE);
				return 1;
			}
			arroundY.setTargetDeg(stepCountY, RELATIVE);	//Ось А на шаг больше
			
			
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

	/* arroundX.setRunMode(FOLLOW_POS); // режим поддержания скорости
	arroundX.setSpeedDeg(24);        // в градусах/сек
	arroundX.setCurrentDeg(0); */
	// arroundX.setTargetDeg(-1, RELATIVE);
	arroundY.setRunMode(FOLLOW_POS);
	arroundY.setSpeedDeg(24);        // в градусах/сек
	arroundY.setCurrentDeg(0);
	// arroundY.setTargetDeg(1, RELATIVE);
	// arroundY.setTargetDeg(20, RELATIVE);
	Timer2.setPeriod(4000);
	Timer2.enableISR();
}
void loop()
{
	/* if (axisYState == false)
	{
		static bool dir = true;
    	dir = !dir;
		arroundY.setTargetDeg(dir ? -360 : 360, RELATIVE);
	} */
	
	/* if (!BufferSysTick3)
	{
		BufferSysTick3 = millis();
	}
	else if ((millis() - BufferSysTick3) > 1000)
	{
		Serial.println(arroundY.getCurrent());
		BufferSysTick3 = 0;
	} */

	#if 1
	switch (cmdStartScanFromPC)
	{
		case 0:
		cmdStartScanFromPC = UART_Receive();
		if (cmdStartScanFromPC == 1)
			arroundY.setTargetDeg(45, RELATIVE);	//Ось А от начала идет на 45гр
		break;

		case 1:
		delay(100);
		if (axisYState == false)
			cmdStartScanFromPC = 2;
		break;

		case 2:
			arroundX.setRunMode(KEEP_SPEED); // режим поддержания скорости
			arroundX.setSpeedDeg(24);        // в градусах/сек
			arroundX.setCurrent(0);
			cmdStartScanFromPC = 3;
		break;

		case 3:
		// cmdStartScanFromPC = 0;
		if (MainProcess()) cmdStartScanFromPC = 0;
		break;
		default:
		break;
	}
	#endif

    
}

// обработчик
ISR(TIMER2_A)
{
	axisXState = arroundX.tick();
	axisYState = arroundY.tick();
}

