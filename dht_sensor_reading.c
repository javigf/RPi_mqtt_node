// sudo g++ dht_sensor_reading.c -o dht.sh -L/usr/local/lib -lwiringPi -lmosquitto

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include "read_dht.c"
#define LED 25

unsigned int activePorts [5]={0,2,3,28,29}; 		//-- PINS

int main (int argc, char *argv[]){
	
	if (wiringPiSetup () < 0){

		printf ( "Error unable to setup wiringPi: %s\n", strerror (errno));
		return 1 ;
	}
	
		
	for (int i=0 ; i<5 ; i++)
		pinMode (activePorts[i] , INPUT);
	pinMode (LED, OUTPUT);
		
	for (int i=0 ; i<5 ; i++)
		displayDHTData (activePorts[i]);
	
	if (!(digitalRead (LED)))
		digitalWrite (LED, HIGH);
	else
		digitalWrite (LED, LOW);
	
	delay (1000);
	
	
	return 0;
}
