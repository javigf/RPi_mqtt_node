//sudo gcc mqttex.c -o mqttex.sh -L/usr/local/lib -lwiringPi -lmosquitto
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <mosquitto.h>
#include "read_dht.c"
#define LED 25
//#define host 54.186.199.187
//#define port 1883

char str_received [32];
unsigned int activePorts [5]={0,2,3,28,29}; 		//-- PINS

char t [5];
char h [5];
char dp [5];
char ledstatus [4];

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		printf("%s %s\n", message->topic, message->payload);
		strcpy (str_received, message->payload);
		
	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int i;
	if(!result){
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "RPi3/in/led/#", 2);
	}else{
		fprintf(stderr, "Connect failed\n");
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	printf("%s\n", str);
}

int main(int argc, char *argv[]){
	int i, counter = 0;
	
	const char host[] = "54.186.199.187";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mosq = NULL;

//////////////////////////////////////////////////////////////////////////	
	if (wiringPiSetup () < 0){

		printf ( "Error unable to setup wiringPi: %s\n", strerror (errno));
		return 1 ;
	}
	
	for (i=0 ; i<5 ; i++)
		pinMode (activePorts[i] , INPUT);
	pinMode (LED, OUTPUT);
	
	for (i=0 ; i<5 ; i++)
		displayDHTData (activePorts[i]);
//////////////////////////////////////////////////////////////////////////	
	
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, clean_session, NULL);
	
	if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	
	mosquitto_log_callback_set(mosq, my_log_callback);
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

	if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}
	
	mosquitto_publish (mosq,NULL,"RPI/out",4,"hola",1,false);
	
	//mosquitto_loop_forever(mosq, -1, 1);
	while(!mosquitto_loop(mosq, -1, 1)){
	
		if (!(strcmp ( str_received, "TOGGLE"))){
			if (digitalRead (LED)){
				digitalWrite (LED, LOW);
				sprintf(ledstatus, "ON");
				sprintf(str_received, "ON");
				
			}
			else{
				digitalWrite (LED, HIGH);
				sprintf(ledstatus, "OFF");
				sprintf(str_received, "OFF");
				
			}
			mosquitto_publish (mosq,NULL,"RPi3/out/led/status",4,ledstatus,1,true);
			printf ("LED %s \n",ledstatus );
		}
		
		++counter;
		
		if (counter > 45)
			if (!displayDHTData (0)){
			
				sprintf(t, "%2.1f", temperature);
				sprintf(h, "%2.1f", humidity);
				sprintf(dp, "%2.1f", dewPoint);
				
				mosquitto_publish (mosq,NULL,"RPi3/out/Sensor/Temperature",4,t,1,false);
				mosquitto_publish (mosq,NULL,"RPi3/out/Sensor/Humidity",4,h,1,false);
				mosquitto_publish (mosq,NULL,"RPi3/out/Sensor/DewPoint",4,dp,1,false);		
				counter = 0;
			}
			
	/*	if (digitalRead (LED))
			digitalWrite (LED, LOW);
		else
			digitalWrite (LED, HIGH);*/			
	}
	
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}
		
