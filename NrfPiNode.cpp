#include <cstdlib>
#include <iostream>
#include <ctime>
extern "C" {
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
}

#include "config.h"
#include "PracticalSocket.h"

#include "RF24.h"
#include "RF24Network.h"

#include "NrfPiNode.h"

using namespace std;

RF24 radio("/dev/spidev0.0",8000000,25);  // Setup for GPIO 25 CSN
RF24Network network(radio);

#define SERVER_PORT  12345

#define TRUE             1
#define FALSE            0

void send_payload(char *payload)
{
  try {
    TCPSocket sock(GRAPHITE_HOST, GRAPHITE_PORT);
    sock.send(payload, strlen(payload));

  } catch(SocketException &e) {
    cerr << e.what() << endl;
  }
}

void radio_setup() {
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(7,7);
    delay(5);
    network.begin(CHANNEL, NODEID);
}

char* handle_sensor_metric(RF24NetworkHeader header, payload_t payload, int devide)
{
    char* dataupload = new char[255];
    float value;
    int16_t _value = (payload.value_high << 4) | payload.value_low;
    if (devide)
        value = (float)_value/100;
    else
        value = (float)_value;

    if (payload.options == 1){
        sprintf(dataupload,"sensor.net.%i.%c.%i -%.2f -1\n\r",
            header.from_node,payload.type,payload.sensor,value);
    } else {
        sprintf(dataupload,"sensor.net.%i.%c.%i %.2f -1\n\r",
            header.from_node,payload.type,payload.sensor,value);
    }
    return dataupload;
}
//Handle radio output
int handle_radio_tx(uint16_t nodeid, char header_type, char* payload)
{
    RF24NetworkHeader header(nodeid, header_type);
    if (network.write(header,&payload,sizeof(payload))) {
        printf("Command send to node: %i\n",nodeid);
        return 1;
    } else {
        printf("Error sending to node: %i\n",nodeid);
        return 0;
    }
}

int is_valid_fd(int fd)
{
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

//Handle the radio input
void handle_radio(fd_set _working_set, int _max_sd) {
    network.update();
    int y;
    while ( network.available() )
    {
        // If so, grab it and print it out
        RF24NetworkHeader header;
        network.peek(header);
        payload_t payload;
        network.read(header,&payload,sizeof(payload));
        char* client_payload = new char[255];
        switch ( header.type ) {
            case 'Q':
                //Handle ping reply
                printf("Received ping from %i",header.from_node);
                break;
        };
        switch ( payload.type ) {
            case 'T': //Process temperature
                client_payload = handle_sensor_metric(header,payload,1);
                break;
            case 'P': //Process Pulse
                client_payload = handle_sensor_metric(header,payload,0);
                break;
            case 'W': //Process Water
                client_payload = handle_sensor_metric(header,payload,0);
                break;
            case 'G': //Precess Gass
                client_payload = handle_sensor_metric(header,payload,0);
                break;
            default:
                printf("Unknown payload type\n");
                return;
                break;
        };
        //Send packet from nodes
        if (header.from_node != 0) {
            printf(client_payload);
            for (y=0; y <= _max_sd; ++y)
            {
                if (FD_ISSET(y, &_working_set)) {
                    signal(SIGPIPE, SIG_IGN);
                    if (is_valid_fd(y))
                        send(y, client_payload, strlen(client_payload), 0);
                }
            }
            send_payload(client_payload);
        }
        free(client_payload);
    }
}

//Handle incomming packet from tcp socket
void handle_tcp_rx(char buffer[80])
{
    input_msg input_data; 
    memcpy(&input_data, buffer, sizeof input_data);
    printf("Sending message to\n\tNodeID: %i\n",input_data.nodeid);
    printf("\tHeader type %c\n\tPayload: %s\n",input_data.header_type,input_data.payload);

    payload_t payload;

    switch ( input_data.header_type )
    {
        case 'P':
            handle_radio_tx(input_data.nodeid,input_data.header_type,(char*)1);
            printf("Sending ping to %i\n",input_data.nodeid);
            break;
        default:
            printf("Unknown header type\n");
            break;
    }
    //payload_t input_payload;
    //memcpy(&input_payload, input_data.payload, sizeof(input_data.payload));
    
    //handle_radio_tx(uint16_t nodeid, char header_type, payload_t payload)
}

int main (int argc, char *argv[])
{
   int    i, len, rc, on = 1;
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   char   buffer[80];
   sockaddr_in   addr;
   timeval       timeout;
   fd_set        master_set, working_set;

   radio_setup();

   listen_sd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }

   /*************************************************************/
   /* Allow socket descriptor to be reuseable                   */
   /*************************************************************/
   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }

   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port        = htons(SERVER_PORT);
   rc = bind(listen_sd,
             (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Set the listen back log                                   */
   /*************************************************************/
   rc = listen(listen_sd, 32);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Initialize the master fd_set                              */
   /*************************************************************/
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);

   /*************************************************************/
   /* Initialize the timeval struct to 3 minutes.  If no        */
   /* activity after 3 minutes this program will end.           */
   /*************************************************************/
   timeout.tv_sec  = 0;
   timeout.tv_usec = 0;

   do
   {
      memcpy(&working_set, &master_set, sizeof(master_set));

      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

      if (rc == 0)
      {
         handle_radio(master_set, max_sd);
         //Send the queued data to the sockets 
      }

      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
      {
         /*******************************************************/
         /* Check to see if this descriptor is ready            */
         /*******************************************************/
         if (FD_ISSET(i, &working_set))
         {
            /****************************************************/
            /* A descriptor was found that was readable - one   */
            /* less has to be looked for.  This is being done   */
            /* so that we can stop looking at the working set   */
            /* once we have found all of the descriptors that   */
            /* were ready.                                      */
            /****************************************************/
            desc_ready -= 1;

            /****************************************************/
            /* Check to see if this is the listening socket     */
            /****************************************************/
            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
               /*************************************************/
               /* Accept all incoming connections that are      */
               /* queued up on the listening socket before we   */
               /* loop back and call select again.              */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Accept each incoming connection.  If       */
                  /* accept fails with EWOULDBLOCK, then we     */
                  /* have accepted all of them.  Any other      */
                  /* failure on accept will cause us to end the */
                  /* server.                                    */
                  /**********************************************/
                  new_sd = accept(listen_sd, NULL, NULL);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }

                  /**********************************************/
                  /* Add the new incoming connection to the     */
                  /* master read set                            */
                  /**********************************************/
                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;

                  /**********************************************/
                  /* Loop back up and accept another incoming   */
                  /* connection                                 */
                  /**********************************************/
               } while (new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               /*************************************************/
               /* Receive all incoming data on this socket      */
               /* before we loop back and call select again.    */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Receive data on this connection until the  */
                  /* recv fails with EWOULDBLOCK.  If any other */
                  /* failure occurs, we will close the          */
                  /* connection.                                */
                  /**********************************************/
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }

                  /**********************************************/
                  /* Check to see if the connection has been    */
                  /* closed by the client                       */
                  /**********************************************/
                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
                     break;
                  }

                  /**********************************************/
                  /* Data was recevied                          */
                  /**********************************************/
                  len = rc;
                  printf("Buffer: ");
                  printf(buffer);
                  printf("\n");
                  printf("  %d bytes received\n", len);
                    
                  //Doe eens naar de nrf sturen
                  handle_tcp_rx(buffer);
                  /**********************************************/
                  /* Echo the data back to the client           */
                  /**********************************************/
                  /*rc = send(i, buffer, len, 0);
                  if (rc < 0)
                  {
                     perror("  send() failed");
                     close_conn = TRUE;
                     break;
                  }*/
                  break;

               } while (TRUE);

               /*************************************************/
               /* If the close_conn flag was turned on, we need */
               /* to clean up this active connection.  This     */
               /* clean up process includes removing the        */
               /* descriptor from the master set and            */
               /* determining the new maximum descriptor value  */
               /* based on the bits that are still turned on in */
               /* the master set.                               */
               /*************************************************/
               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */

   } while (end_server == FALSE);

   /*************************************************************/
   /* Cleanup all of the sockets that are open                  */
   /*************************************************************/
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
}

