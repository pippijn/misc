CFLAGS	= -std=c99 -Wall -pedantic
LDFLAGS	= -lm

genland: land.h

land.wrl: genland
	./$< > $@

show: land.wrl
	freewrl $<
