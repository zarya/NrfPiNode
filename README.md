# NrfPiNode 

NrfPiNode that makes use of the RF24Network lib and runs on a raspberry pi

The lib is build against the RF24 lib using the GPIO way

Based on the works of https://github.com/farconada/RF24Network

Based on the work of @stanleyseow and @maniacbug
https://github.com/stanleyseow/RF24
Please see the full documentation at http://maniacbug.github.com/RF24Network/index.html

# Features
* Receive sensor metrics and put them in graphite

## Wishlist
* Ability to send commands to the nodes from the main-controller
* Ota config menu

# Commands
## main-controller
This is the main controller the deamon that will do all the communication from and to the sensor network

## ota-example
This is a proof of concept to the the ota-config module

## tx-test
This tool sends a ping request to node 5 and waits for the reply 
