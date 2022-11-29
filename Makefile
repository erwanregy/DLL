build: main.cpp dll.cpp uart.cpp mem.cpp
	avr-g++ -mmcu=atmega644p -DF_CPU=12000000 -Wall -Os main.cpp dll.cpp uart.cpp mem.cpp dll_test.cpp -o dll.elf
	avr-objcopy -O ihex dll.elf dll.hex
	
flash: build
	avrdude -c usbasp -p m644p -U flash:w:dll.hex

clean:
	rm dll.elf dll.hex