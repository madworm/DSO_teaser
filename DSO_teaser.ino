#include <util/delay.h>

#define FIVEHUNDRED 492
#define TWOFIFTY 246
#define FIFTY 45

#define UART_TX 4
#define FIVE_VOLTS 3
#define TRITRI_VOLTS 2
#define GND 0
#define UART 7
#define RUNT 1

#define LATCH_LOW  PORTB &= ~_BV(PB2);
#define LATCH_HIGH  PORTB |= _BV(PB2);

void setup(void)
{
	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	pinMode(A5, OUTPUT);
	digitalWrite(A5, LOW);
	pinMode(2, INPUT);
	digitalWrite(2, HIGH);
	pinMode(3, INPUT);
	digitalWrite(3, HIGH);
	pinMode(4, INPUT);
	digitalWrite(4, HIGH);
	pinMode(9, OUTPUT);	// PB1
	digitalWrite(9, LOW);	// discharge the cap
	pinMode(10, OUTPUT);	// PB2
	pinMode(11, OUTPUT);
	pinMode(13, OUTPUT);
	setup_hardware_spi();
	Serial.begin(9600);
}

void loop(void)
{
	static uint8_t mode = 0;

	// Serial.println(mode, BIN);

	_delay_ms(250);

	Serial.print(F("Mode: "));
	Serial.print(mode);

	switch (mode) {

	case 7:
		Serial.println
		    (" - pulse train B: change pulse width with right button - 1: trigger - 4: signal\n");
		Serial.flush();
		cli();		// can't have any interrupts running, too much timing jitter (esp. TCNT0)
		pulse_train_B();
		sei();
		mode = 0;
		_delay_ms(250);
		break;

	case 6:
		Serial.println
		    (" - pulse train A: change trigger with right button - 1: trigger - 4: signal\n");
		Serial.flush();
		cli();		// can't have any interrupts running, too much timing jitter (esp. TCNT0)
		pulse_train_A();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 5:
		Serial.println(" - 'blip': 1: trigger - 4: signal\n");
		Serial.flush();
		cli();
		blip();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 4:
		Serial.println
		    ("- 1kHz - toggle modes with right button - 1: trigger 4: signal\n");
		Serial.flush();
		cli();
		one_kilohertz();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 3:
		Serial.println
		    (" - SPI binary counter: 1: CS - 2: DATA - 3: CLOCK\n");
		Serial.flush();
		cli();
		spi_binary_counter();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 2:
		Serial.println(" - UART 'U\\n': 4: signal\n");
		Serial.flush();
		cli();
		uart_data();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 1:
		Serial.println
		    (" - Runt Pulse: modify with right button - 1: trigger - 4: signal\n");
		Serial.flush();
		cli();
		runt_pulse();
		sei();
		mode++;
		_delay_ms(250);
		break;

	case 0:
		Serial.println
		    (" - Voltage: change with right button - 4: signal\n");
		Serial.flush();
		cli();
		static_voltage();
		sei();
		mode++;
		_delay_ms(250);
		break;

	default:
		Serial.println("something went wrong\n");
		while (digitalRead(3) == HIGH) {
			PORTC = GND;
		}
		mode = 0;
		_delay_ms(250);
		break;
	}
}

void setup_hardware_spi(void)
{
	uint8_t clr;
	// spi prescaler:
	//
	// SPCR: SPR1 SPR0
	// SPSR: SPI2X
	//
	// SPI2X SPR1 SPR0
	//   0     0     0    fosc/4
	//   0     0     1    fosc/16
	//   0     1     0    fosc/64
	//   0     1     1    fosc/128
	//   1     0     0    fosc/2
	//   1     0     1    fosc/8
	//   1     1     0    fosc/32
	//   1     1     1    fosc/64

	/* enable SPI as master */
	SPCR |= (_BV(SPE) | _BV(MSTR));
	/* clear registers */
	clr = SPSR;
	clr = SPDR;
	/* set prescaler to fosc/2 */
	SPCR &= ~(_BV(SPR1) | _BV(SPR0));
	SPSR |= _BV(SPI2X);
}

inline uint8_t spi_transfer(uint8_t data)
{
	SPDR = data;		// Start the transmission
	while (!(SPSR & _BV(SPIF)))	// Wait the end of the transmission
	{
	};
	return SPDR;		// return the received byte. (we don't need that here)
}

void one_kilohertz(void)
{
	static uint8_t mode = 0;
	const uint8_t number_of_modes = 7;

	while (digitalRead(3) == HIGH) {

		if (mode == 0) {
			trigger_pulse();

			PORTC = FIVE_VOLTS;
			_delay_us(FIVEHUNDRED);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (mode == 1) {
			trigger_pulse();

			PORTC = TRITRI_VOLTS;
			_delay_us(FIVEHUNDRED);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (mode == 2) {
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);

			trigger_pulse();

			PORTC = TRITRI_VOLTS;
			_delay_us(1);
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (mode == 3) {
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);

			trigger_pulse();

			PORTC = GND;
			asm("nop\n");
			asm("nop\n");
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (mode == 4) {
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);

			trigger_pulse();

			PORTC = TRITRI_VOLTS;
			asm("nop\n");
			asm("nop\n");
			PORTC = FIVE_VOLTS;
			_delay_us(TWOFIFTY);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (mode == 5) {
			PORTC = TRITRI_VOLTS;
			_delay_us(TWOFIFTY);

			trigger_pulse();

			PORTC = FIVE_VOLTS;
			_delay_us(1);
			PORTC = TRITRI_VOLTS;
			_delay_us(TWOFIFTY);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);

		}

		if (mode == 6) {
			PORTC = TRITRI_VOLTS;
			_delay_us(TWOFIFTY);

			trigger_pulse();

			PORTC = FIVE_VOLTS;
			PORTC = TRITRI_VOLTS;
			_delay_us(TWOFIFTY);
			PORTC = GND;
			_delay_us(FIVEHUNDRED);
		}

		if (digitalRead(2) == LOW) {
			mode++;
			_delay_ms(150);
			if (mode > (number_of_modes - 1)) {
				mode = 0;
			}
		}
	}
}

void blip(void)
{
	uint16_t counter2;

	while (digitalRead(3) == HIGH) {
		for (counter2 = 0; counter2 < 50; counter2++) {
			PORTC = GND;
			_delay_us(100);

			PORTC = FIVE_VOLTS;
			_delay_us(20);

			PORTC = GND;
			_delay_us(20);

			PORTC = FIVE_VOLTS;
			_delay_us(20);
		}

		PORTC = GND;
		_delay_us(100);

		PORTC = FIVE_VOLTS;
		_delay_us(20);

		PORTC = GND;
		_delay_us(9);

		trigger_pulse();

		PORTC = FIVE_VOLTS;
		_delay_us(2);

		PORTC = GND;
		_delay_us(9);

		PORTC = FIVE_VOLTS;
		_delay_us(20);
	}
}

void runt_pulse(void)
{
	uint16_t counter2;

	static uint8_t runt_magnitude = 0;
	const uint8_t number_of_magnitudes = 14;

	while (digitalRead(3) == HIGH) {

		for (counter2 = 0; counter2 < 50; counter2++) {
			PORTC = GND;
			_delay_us(100);

			PORTC = FIVE_VOLTS;
			_delay_us(20);

			PORTC = GND;
			_delay_us(20);

			PORTC = FIVE_VOLTS;
			_delay_us(20);
		}

		PORTC = GND;
		_delay_us(80);

		trigger_pulse();

		PORTC = RUNT;
		PORTB |= _BV(PB1);	// charge the cap a bit
		__delay_us(runt_magnitude);
		PORTB &= ~_BV(PB1);	// discharge it again                
		__delay_us(15 - runt_magnitude);
		PORTC = GND;

		PORTC = GND;
		_delay_us(20);

		PORTC = FIVE_VOLTS;
		_delay_us(20);

		PORTC = GND;
		_delay_us(20);

		if (digitalRead(2) == LOW) {
			runt_magnitude++;
			_delay_ms(150);
			if (runt_magnitude > (number_of_magnitudes - 1)) {
				runt_magnitude = 0;
			}
		}

	}
}

void spi_binary_counter(void)
{

	static uint8_t tmp = 0;
	uint16_t delay_counter = 0;
	const uint16_t delay_counter_max = 15000;
	uint8_t binary_counter_is_running = 1;

	while (digitalRead(3) == HIGH) {
		LATCH_LOW;
		spi_transfer(tmp);
		LATCH_HIGH;
		if (digitalRead(2) == LOW) {
			binary_counter_is_running = !binary_counter_is_running;
			_delay_ms(150);
		}
		if (binary_counter_is_running) {
			delay_counter++;
			if (delay_counter == delay_counter_max) {
				tmp++;
				delay_counter = 0;
			}
		}
	}
}

void uart_data(void)
{
	while (digitalRead(3) == HIGH) {
		trigger_pulse();
		PORTC = UART_TX;
		UDR0 = 'U';
		while (!(UCSR0A & _BV(TXC0))) {
			// wait until data is out
			// don't use the Serial.stuf, I need perfect timing here
		}
		UCSR0A |= _BV(TXC0);	// clear flag
		delay(1);
	}
	Serial.println("");
}

void static_voltage(void)
{
	static uint8_t mode = 0;
	const uint8_t number_of_modes = 3;

	while (digitalRead(3) == HIGH) {

		if (mode == 0) {
			PORTC = FIVE_VOLTS;
		}

		if (mode == 1) {
			PORTC = TRITRI_VOLTS;
		}

		if (mode == 2) {
			PORTC = GND;
		}

		if (digitalRead(2) == LOW) {
			mode++;
			_delay_ms(150);
			if (mode > (number_of_modes - 1)) {
				mode = 0;
			}
		}
	}
}

void pulse_train_A(void)
{

	static uint8_t trigger_position = 0;
	const uint8_t number_of_trigger_positions = 16;

	while (digitalRead(3) == HIGH) {

		if (trigger_position == 0) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 1) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		asm("nop\n");
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 2) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		asm("nop\n");
		asm("nop\n");
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 3) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 4) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 5) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		asm("nop\n");
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 6) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(1);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 7) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(1);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 8) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(2);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 9) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(3);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 10) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(4);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 11) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(5);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 12) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(6);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 13) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(7);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 14) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(8);
		PORTC = GND;
		_delay_us(FIFTY);

		if (trigger_position == 15) {
			trigger_pulse();
		}
		PORTC = FIVE_VOLTS;
		_delay_us(9);
		PORTC = GND;
		_delay_us(FIFTY);

		_delay_ms(5);

		if (digitalRead(2) == LOW) {
			trigger_position++;
			_delay_ms(150);
			if (trigger_position >
			    (number_of_trigger_positions - 1)) {
				trigger_position = 0;
			}
		}

	}
}

void pulse_train_B(void)
{

	static uint8_t pulse_width = 0;

	while (digitalRead(3) == HIGH) {

		switch (pulse_width) {
		case 0:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			PORTC = GND;
			break;

		case 1:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			asm("nop\n");
			PORTC = GND;
			break;

		case 2:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			asm("nop\n");
			asm("nop\n");
			PORTC = GND;
			break;

		case 3:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			PORTC = GND;
			break;

		case 4:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			PORTC = GND;
			break;

		case 5:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			asm("nop\n");
			PORTC = GND;
			break;

		case 6:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(1);
			PORTC = GND;
			break;

		case 7:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(1);
			PORTC = GND;
			break;

		case 8:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(2);
			PORTC = GND;
			break;

		case 9:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(3);
			PORTC = GND;
			break;

		case 10:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(4);
			PORTC = GND;
			break;

		case 11:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(5);
			PORTC = GND;
			break;

		case 12:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(6);
			PORTC = GND;
			break;

		case 13:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(7);
			PORTC = GND;
			break;

		case 14:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(8);
			PORTC = GND;
			break;

		case 15:
			trigger_pulse();
			PORTC = FIVE_VOLTS;
			_delay_us(9);
			PORTC = GND;
			break;

		default:
			pulse_width = 0;
			break;
		}

		if (digitalRead(2) == LOW) {
			pulse_width++;
			_delay_ms(150);
		}

		_delay_us(FIFTY);
	}
}

void trigger_pulse(void)
{
	PORTB |= _BV(PB2);	// offer trigger
	PORTB &= ~_BV(PB2);
}

void __delay_us(uint16_t us)
{
	while (us) {
		_delay_us(1);
		us--;
	}

}
