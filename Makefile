SRCS = main.cpp dll.cpp mem.cpp ../UART/uart.c

build: $(SRC)
	avr-g++ -mmcu=atmega644p -DF_CPU=12000000 -Wall -Os $(SRCS) -o dll.elf
	avr-objcopy -O ihex dll.elf dll.hex
	
flash: build
	avrdude -c usbasp -p m644p -U flash:w:dll.hex

clean:
	rm dll.elf dll.hex