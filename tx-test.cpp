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

struct payload_t
{
  char type;
  uint8_t sensor;
  uint8_t value_high;
  uint8_t value_low;
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

	RF24NetworkHeader header(5, 'P');
	int data = 1;
	time_t start, stop;
   	clock_t ticks; long count;
 
	time(&start);
	network.write(header,&data,sizeof(data));
	printf("\nSending ping\n\n");

	while(1)
	{
        	network.update();
        	while ( network.available() )
		{
			// If so, grab it and print it out
			RF24NetworkHeader header;
			network.peek(header);
			if (header.type == 'Q') {
				printf("\nPing reply from: %i\n\n",header.from_node);
				time(&stop);
				printf("Finished in about %.0f seconds. \n", difftime(stop, start));
				return 0;
			}
			network.read(header,0,0);
		}
		delay(5);
	}
	return 0;
}

