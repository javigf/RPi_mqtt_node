
//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMINGS	85
//#define DHT_PIN		15	/* GPIO-22 */

bool DHTsensorType = FALSE;

int read_dht_data (uint8_t);
bool checkPort (uint8_t);
int displayDHTData (unsigned short);

//uint8_t DHT_PIN = 15;
int data[5] = { 0, 0, 0, 0, 0 };