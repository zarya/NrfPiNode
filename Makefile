#############################################################################
#
# Makefile for NrfPiNode 
#
#
prefix := /usr/local

CCFLAGS=-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -g
PROGRAMS = NrfPiNode
SOURCES = ${PROGRAMS:=.cpp}

all: ${PROGRAMS}

${PROGRAMS}: ${SOURCES}
	g++ ${CCFLAGS} -Wall -I../RF24Network -I../RF24 -lrf24-bcm -lrf24network $@.cpp PracticalSocket.cpp -o $@

clean:
	rm -rf $(PROGRAMS)

install: all
	test -d $(prefix) || mkdir $(prefix)
	test -d $(prefix)/bin || mkdir $(prefix)/bin
	for prog in $(PROGRAMS); do \
	  install -m 0755 $$prog $(prefix)/bin; \
	done

.PHONY: install
