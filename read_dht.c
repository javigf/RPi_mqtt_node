/*
 *  dht.c:
 *	read temperature and humidity from DHT11 or DHT22 sensor
 *
 *  sudo gcc -o dht22 dht22.c -L/usr/local/lib -lwiringPi
 
 */
#define DHT_VER 0.2
#define MAX_ERROR_RETIRES 20
#include "read_dht.h"

float temperature, humidity, dewPoint;

/////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++ INIT READ DHT FUNCTION +++++++++++++++++++++++++++++++++
/////////////////////////////////////////////////////////////////////////////////
 
inline int read_dht_data (uint8_t DHT_PIN) {
	bool laststate		= TRUE;
	uint8_t counter		= 0;
	uint8_t j			= 0, i;
	

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	/* pull pin down for 18 milliseconds */
	pinMode( DHT_PIN, OUTPUT );
	digitalWrite( DHT_PIN, LOW );
	delay( 18 );
 	//pinMode( DHT_PIN, OUTPUT ); //-- ADDED
 	// then pull it up for 40 microseconds
  	digitalWrite( DHT_PIN, HIGH );
  	delayMicroseconds( 40 ); 
	//delayMicroseconds( 20 ); // .. Modified
	/* prepare to read the pin */
	pinMode( DHT_PIN, INPUT );

	/* detect change and read data */
	for ( i = 0; i < MAX_TIMINGS; i++ ){
		counter = 0;
		while ( digitalRead( DHT_PIN ) == laststate ){
			counter++;
			delayMicroseconds( 1 );
			
			if ( counter > MAX_TIMINGS ){
				break;
			}
		}
		laststate = digitalRead( DHT_PIN );

		if ( counter > MAX_TIMINGS )
			break;

		/* ignore first 3 transitions */
		if ((i >= 4) && (i % 2 == 0)){
			/* shove each bit into the storage bytes */
			data[j / 8] <<= 1;
			if ( counter > 16 )
				data[j / 8] |= 1;
			j++;
		}
	}
	

	/*
	 * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */
	if ((j >= 40) && (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
		humidity = (float)((data[0] << 8) + data[1]) / 10;
		
		
		if ( humidity > 100 ){
			humidity = data[0];	// for DHT11
			DHTsensorType = TRUE;
		}
		else
			DHTsensorType = FALSE;
		
		temperature = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
		if ( temperature > 125 ){
			temperature = data[2];	// for DHT11
		}
		if ( data[2] & 0x80 ){
			temperature = -temperature;
		}		
		//float f = temperature * 1.8f + 32;
		
		if ((temperature < -10) || (temperature > 50) || (humidity < 1) || (humidity > 95)){
			fprintf(stderr, "Temp out of range %2.1f\n", temperature);
			fprintf(stderr, "Hum out of range %2.1f\n", humidity);
			fprintf(stderr, "Retrying... on pin %d\n",DHT_PIN);
			return 1;
		}
	}
	else{
		//printf( "DHT Data not good, skip\n" );
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
//----------------------- END DHT READ FUNCTION ---------------------------------
/////////////////////////////////////////////////////////////////////////////////
// This function calculates the DewPoint of a given Hum & Temp

float calcDewPoint (float temp, float hum){

	dewPoint= temp-((100-hum)/5);
	return dewPoint;
}

/////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++ INIT Display FUNCTION ++++++++++++++++++++++++++++++++++
/////////////////////////////////////////////////////////////////////////////////
// This function display the DHT Data of a given pin

int displayDHTData (unsigned short pin) {
	unsigned int counter = 0;
	long _timer = 100;
	
	if (!checkPort (pin)){
		//printf ("NOT DHT Sensor on pin %d \n", pin);  //ERROR 10 DESCONECTADO
		return 10;
	}

	while ((read_dht_data (pin)) && (counter < MAX_ERROR_RETIRES)){
		counter++;
		delay (_timer * counter);
	}
	
	//if (counter)
		//fprintf(stderr, "Times trying to get readings %d on pin %d\n", counter,pin);
	
	if (counter >= MAX_ERROR_RETIRES){
		printf ("Retry reading on pin %d \n", pin);
		return 20;
	}
	else{
		if (!DHTsensorType)
			printf ("Sensor is DHT22 ");
		else
			printf ("Sensor is DHT11 ");
		
		printf ("Temp: %2.1f Hum: %2.1f Dew Point %2.1f\n", temperature, humidity,calcDewPoint (temperature, humidity));
		
		
	
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////
//----------------------- END OF Display FUNCTION -------------------------------
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++ INIT CheckPort FUNCTION ++++++++++++++++++++++++++++++++
/////////////////////////////////////////////////////////////////////////////////

bool checkPort ( uint8_t port ){
	
	pinMode ( port,INPUT );
	bool testPort = digitalRead ( port );

	return testPort;
}
/////////////////////////////////////////////////////////////////////////////////
//----------------------- END OF CheckPort FUNCTION -----------------------------
/////////////////////////////////////////////////////////////////////////////////


//*******************************************************************************
//****************************** END C SCRIPT ***********************************
//*******************************************************************************
