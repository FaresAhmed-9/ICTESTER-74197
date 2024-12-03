
#include <avr/io.h>
#include <util/twi.h>			//--- Give Status of I2C Bus will operation
#define F_CPU	16000000UL
#include <util/delay.h>
#include "twi_lcd.h"
#define ERROR(code) while (1) {} // Error handling: infinite loop
#define SET_PIN(port, pin) ((port) |= (1 << (pin)))
#define CLEAR_PIN(port, pin) ((port) &= ~(1 << (pin)))
#define READ_PIN(pin, bit) ((pin) & (1 << (bit)))
#define WRITE_PIN(port, pin, value) \
if (value)                     \
        SET_PIN(port, pin);        \
    else                           \
        CLEAR_PIN(port, pin);

int IC_74197(void) {
	// Configure DDR for inputs and outputs
	DDRA = 0x6D;  // PA0, PA2, PA3, PA5 as output; PA1, PA4 as input
	DDRB = 0x5B;  // PB0, PB1, PB3, PB4, PB6 as output; PB2, PB5 as input

	// Initialize control pins
	CLEAR_PIN(PORTA, 5);  // Clock2 (PA5, PIN6)
	CLEAR_PIN(PORTB, 6);  // Clock1 (PB6, PIN8)
	SET_PIN(PORTA, 0);    // Load (PA0, PIN1)
	SET_PIN(PORTB, 1);    // Clear (PB1, PIN13)

	// Reset using Clear
	CLEAR_PIN(PORTB, 1);  // Clear LOW
	SET_PIN(PORTB, 1);    // Clear HIGH

	// Test PIN5 (PA4, PIN5) for two clock cycles
	for (uint8_t i = 0; i < 2; i++) {
		if (((PINA >> 4) & 1) != i) { // Check if bit 4 of PINA is equal to i
			
			return 1;
		}
		SET_PIN(PORTB, 6);  // Clock1 (PB6, PIN8) HIGH
		CLEAR_PIN(PORTB, 6); // Clock1 (PB6, PIN8) LOW
	}

	// Reset using Clear again
	CLEAR_PIN(PORTB, 1);  // Clear LOW
	SET_PIN(PORTB, 1);    // Clear HIGH

	// Test outputs on PIN12, PIN2, PIN9 (PB2, PA1, PB5) for 8 clock cycles
	for (uint8_t i = 0; i < 8; i++) {
		 // Read and combine the input pins into a 3-bit value
		 uint8_t bit2 = ((PINB >>2)&1);  // PINB[2]
		 uint8_t bit1 = ((PINA>> 1)&1);  // PINA[1]
		 uint8_t bit0 = ((PINB>> 5)&1);  // PINB[5]
		 uint8_t value = (bit2 << 2) | (bit1 << 1) | bit0;
		if (value != i) {
			return 1;
		}
		SET_PIN(PORTA, 5);  // Clock2 (PA5, PIN6) HIGH
		CLEAR_PIN(PORTA, 5); // Clock2 (PA5, PIN6) LOW
	}

	// Check all outputs are zero
	if ((READ_PIN(PINB, 2) | READ_PIN(PINA, 1) | READ_PIN(PINB, 5) | READ_PIN(PINA, 4)) != 0) {
		return 1;
	}

	// Test data loading functionality
	uint8_t d = 0b0101; // Initial data

	for (uint8_t i = 0; i < 2; i++) {
		CLEAR_PIN(PORTA, 0); // Load (PA0, PIN1) LOW

		// Set data on D0, D1, D2, D3 (PA3, PB4, PA2, PB3)
		WRITE_PIN(PORTB,3,(d&0x08));
		WRITE_PIN(PORTA,2,(d&0x04));
		WRITE_PIN(PORTB,4,(d&0x02));
		WRITE_PIN(PORTA,3,(d&0x01));
		SET_PIN(PORTA, 0); // Load (PA0, PIN1) HIGH

		// Verify data on Q0, Q1, Q2, Q3 (PA4, PB5, PA1, PB2)
		uint8_t value = ((PINA >> 4) & 0x01) |       // Q0 -> PA4
		((PINB >> 5) & 0x01) << 1 | // Q1 -> PB5
		((PINA >> 1) & 0x01) << 2 | // Q2 -> PA1
		((PINB >> 2) & 0x01) << 3;  // Q3 -> PB2

		if (value != d) { // Check if the read value matches the expected value
			return 1;     // Error if mismatch
		}

		d ^= 0x0F; // Toggle data bits (0b0101 <-> 0b1010)
	}


	// Final reset
	CLEAR_PIN(PORTB, 1);  // Clear LOW
	SET_PIN(PORTB, 1);    // Clear HIGH

	// Verify final state
	if ((READ_PIN(PINB, 2) | READ_PIN(PINA, 1) | READ_PIN(PINB, 5) | READ_PIN(PINA, 4)) != 0) {
		return 1;
	}

	twi_lcd_msg("IC 74197");
	return 0;
}

int main(void){
	twi_init();
	twi_lcd_init();
	IC_74197();
}