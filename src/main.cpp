#include <Arduino.h>
#include "LoRaWan_APP.h"
#include <MPU9250.h>
#include <Wire.h>
#include <SPI.h>
MPU9250 mySensor;
/* OTAA para*/
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x6C, 0xD1};
uint8_t appEui[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x09};
uint8_t appKey[] = {0xA1, 0x01, 0x86, 0xB9, 0xFE, 0x3F, 0x10, 0x7C, 0x06, 0x55, 0x9F, 0x50, 0x98, 0xFF, 0xA4, 0x79};
/* ABP para*/
uint8_t nwkSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t devAddr =  ( uint32_t )0x260BCE52;
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
uint32_t appTxDutyCycle = 5000;//5 sg
bool overTheAirActivation = LORAWAN_NETMODE;
bool loraWanAdr = LORAWAN_ADR;
bool keepNet = LORAWAN_NET_RESERVE;
bool isTxConfirmed = LORAWAN_UPLINKMODE;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;
//CONSTRUCTOR
 //I2C
static void prepareTxFrame( uint8_t port )
{
	float aX,aY,aZ,mX,mY,mZ;
	if(mySensor.accelUpdate()==0){
 		aX=mySensor.accelX();
  		aY=mySensor.accelY();
  		aZ=mySensor.accelZ();
	}
	if(mySensor.magUpdate()==0){
		mX=mySensor.magX();
  		mY=mySensor.magY();
  		mZ=mySensor.magZ();
	}
	aX=aX*100;
	aY=aY*100;
	aZ=aZ*100;
	mX=mX*100;	
	mY=mY*100;
	mZ=mZ*100;
	Serial.print("aX:");Serial.print(aX);Serial.print(" aY:");Serial.print(aY);Serial.print(" aZ:");Serial.println(aZ);
  	Serial.print(" mX:");Serial.print(mX);Serial.print(" mY:");Serial.print(mY);Serial.print(" mZ:");Serial.println(mZ);  

	unsigned char *puc;
	puc=(unsigned char*)&aX;
	appDataSize = 16;
	appData[0] = puc[0];
	appData[1] = puc[1];
	appData[2] = puc[2];
	appData[3] = puc[3];
	puc=(unsigned char*)&aY;
	appData[4] = puc[0];
	appData[5] = puc[1];			
	appData[6] = puc[2];
	appData[7] = puc[3];
	puc=(unsigned char*)&aZ;
	appData[8] = puc[0];						
	appData[9] = puc[1];
	appData[10] = puc[2];
	appData[11] = puc[3];
	puc=(unsigned char*)&mX;
	appData[12] = puc[0];
	appData[13] = puc[1];
	appData[14] = puc[2];				
	appData[15] = puc[3];


}
void setup() {
	boardInitMcu();
	Serial.begin(115200);
  Wire.begin();
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();


#if(AT_SUPPORT)
	enableAt();
#endif
	deviceState = DEVICE_STATE_INIT;
	LoRaWAN.ifskipjoin();
}

void loop()
{
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(AT_SUPPORT)
			getDevParam();
#endif
			printDevParam();
			LoRaWAN.init(loraWanClass,loraWanRegion);
			deviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			prepareTxFrame( appPort );
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
			LoRaWAN.sleep();
			break;
		}
		default:
		{
			deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}
