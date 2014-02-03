#include <cstdlib>
#include <iostream>
#include "RF24.h"
#include "RF24Network.h"
#include <ctime>
#include <stdio.h>
#include "config.h"
#include "PracticalSocket.h"
#include <time.h>

/**
 * g++ -L/usr/lib main.cc -I/usr/include -o main -lrrd
 **/
using namespace std;

RF24 radio("/dev/spidev0.0",8000000,25);  // Setup for GPIO 25 CSN
RF24Network network(radio);

struct payload_config_t 
{
  uint8_t pos;
  uint8_t data;
  uint8_t options;
};

int main(int argc, char** argv) 
{
	radio.begin();
	radio.setDataRate(RF24_250KBPS);
	radio.setRetries(7,7);
	delay(5);
	network.begin(CHANNEL, NODEID);
	network.update();

	char a_char[3];
	printf("Enter byte number:");
	cin >> a_char;
	char a_char2[3];
	printf("Enter config value:");
	cin >> a_char2;
	char a_char3[3];
	printf("Enter settings value: ");
	cin >> a_char3;
	payload_config_t payload = {atoi(a_char),atoi(a_char2),atoi(a_char3)};
	

	RF24NetworkHeader header(5, 'C');
	if (network.write(header,&payload,sizeof(payload))) {
		printf("\nSending config\n\n");
	}else{
		printf("\nError\n");
	}
	return 0;
}

