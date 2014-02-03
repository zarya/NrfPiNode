#include <cstdlib>
#include <iostream>
#include "RF24.h"
#include "RF24Network.h"
#include <ctime>
#include <stdio.h>
#include "config.h"
#include "PracticalSocket.h"

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

void send_payload(char *payload)
{
  try {
    TCPSocket sock(GRAPHITE_HOST, GRAPHITE_PORT);
    sock.send(payload, strlen(payload));

  } catch(SocketException &e) {
    cerr << e.what() << endl;
  }
}

int main(int argc, char** argv) 
{
	float value;
	radio.begin();
	radio.setDataRate(RF24_250KBPS);
	radio.setRetries(7,7);
	delay(5);
	network.begin(CHANNEL, NODEID);
	while(1)
	{
		  network.update();
		  while ( network.available() )
		  {
		    // If so, grab it and print it out
		    RF24NetworkHeader header;
		    network.peek(header);
		    payload_t payload;
		    network.read(header,&payload,sizeof(payload));
		    char dataupload[] ="";
	 	    int16_t _value = (payload.value_high << 4) | payload.value_low;
		    if (header.type == 'Q') {
			printf("Received ping from %i",header.from_node)
		    }
		    if (payload.type == 'P' or payload.type == 'W' or payload.type == 'G') {
	 	    	value = (float)_value;
		    } else {
     			value = (float)_value/100;
		    }
		    if (payload.options == 1){
		    	sprintf(dataupload,"sensor.net.%i.%c.%i -%.2f -1\n\r",
				header.from_node,payload.type,payload.sensor,value);
		    } else {
			sprintf(dataupload,"sensor.net.%i.%c.%i %.2f -1\n\r",
                                header.from_node,payload.type,payload.sensor,value);
		    }
		    if (header.from_node != 0) {
		    	printf(dataupload);
		    	send_payload(dataupload);
		    }
		  }
		 delay(5);
	}

	return 0;
}

