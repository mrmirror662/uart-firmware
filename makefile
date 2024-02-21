CC=avr-gcc
CFLAGS= -Os -DF_CPU=16000000 -mmcu=atmega328p

all: blink.out


USBPORT:=COM5

%.out: %.c
	$(CC) $(CFLAGS) $< -o $@

%.hex: %.out
	avr-objcopy -O ihex -R .eeprom $< $@

install: blink.hex
	avrdude -F -V -c arduino -p m328p -P COM3 -b 9600 -U flash:w:$<

clean:
	rm -f *.hex *.out