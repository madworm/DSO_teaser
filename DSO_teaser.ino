#include <util/delay.h>

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
        pinMode(9,OUTPUT); // PB1
        digitalWrite(9,LOW); // discharge the cap
	pinMode(10, OUTPUT); // PB2
	pinMode(11, OUTPUT);
	pinMode(13, OUTPUT);
	setup_hardware_spi();
	Serial.begin(9600);
}

void loop(void)
{
	static uint8_t counter = 0;
	static uint8_t tmp = 0;
	static uint32_t last_increment = 0;
	static uint32_t last_changed = 0;
	const uint32_t start_stop_change_delay = 250;
	const uint32_t increment_delay = 250;
	uint8_t binary_counter_is_running = 1;

	if (counter > 13) {
		counter = 0;
	}
	Serial.println(counter, BIN);

	delay(250);

	switch (counter) {

  	case 13:
		one_kilohertz_spike2();
		counter++;
		delay(250);
		break;
  
  	case 12:
		one_kilohertz_spike();
		counter++;
		delay(250);
		break;
  
  	case 11:
		blip();
		counter++;
		delay(250);
		break;

	case 10:
		one_kilohertz_dip3();
		counter++;
		delay(250);
		break;

	case 9:
		one_kilohertz_dip2();
		counter++;
		delay(250);
		break;

	case 8:
		one_kilohertz_dip();
		counter++;
		delay(250);
		break;

	case 7:
		one_kilohertz_tritri_volts();
		counter++;
		delay(250);
		break;

	case 6:
		one_kilohertz_five_volts();
		counter++;
		delay(250);
		break;

	case 5:
		while (digitalRead(3) == HIGH) {
			LATCH_LOW;
			spi_transfer(tmp);
			LATCH_HIGH;
			if ((digitalRead(2) == LOW)
			    && ((millis() - last_changed) >
				start_stop_change_delay)) {
				binary_counter_is_running =
				    !binary_counter_is_running;
				last_changed = millis();
			}
			if (binary_counter_is_running
			    && ((millis() - last_increment) >
				increment_delay)) {
				tmp++;
				last_increment = millis();
			}
		}
		counter++;
		delay(250);
		break;

	case 4:
		while (digitalRead(3) == HIGH) {
			PORTC = UART_TX;
			Serial.println("U");
		}
		counter++;
		delay(250);
		break;

	case 3:
		while (digitalRead(3) == HIGH) {
			PORTC = FIVE_VOLTS;
		}
		counter++;
		delay(250);
		break;

	case 2:
		while (digitalRead(3) == HIGH) {
			PORTC = TRITRI_VOLTS;
		}
		counter++;
		delay(250);
		break;

	case 1:
                runt_pulse();
		counter++;
		delay(250);
		break;

	case 0:
		while (digitalRead(3) == HIGH) {
			PORTC = GND;
		}
		counter++;
		delay(250);
		break;

	default:
		while (digitalRead(3) == HIGH) {
			PORTC = GND;
		}
		counter++;
		delay(250);
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

void one_kilohertz_five_volts(void)
{
	while (digitalRead(3) == HIGH) {

		PORTC = FIVE_VOLTS;
		delayMicroseconds(495);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_tritri_volts(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = TRITRI_VOLTS;
		delayMicroseconds(495);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_dip(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = FIVE_VOLTS;
		delayMicroseconds(247);
		PORTC = TRITRI_VOLTS;
		delayMicroseconds(1);
		PORTC = FIVE_VOLTS;
		delayMicroseconds(248);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_dip2(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = FIVE_VOLTS;
		delayMicroseconds(247);
                PORTB |= _BV(PB2); // offer trigger ;-)
		PORTC = GND;
                asm("nop\n");
                asm("nop\n");
		PORTC = FIVE_VOLTS;
                PORTB &= ~_BV(PB2);
		delayMicroseconds(248);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_dip3(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = FIVE_VOLTS;
		delayMicroseconds(247);
                PORTB |= _BV(PB2); // offer trigger ;-)
		PORTC = TRITRI_VOLTS;
                asm("nop\n");
                asm("nop\n");
		PORTC = FIVE_VOLTS;
                PORTB &= ~_BV(PB2);
		delayMicroseconds(248);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_spike(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = TRITRI_VOLTS;
		delayMicroseconds(247);
		PORTC = FIVE_VOLTS;
		delayMicroseconds(1);
		PORTC = TRITRI_VOLTS;
		delayMicroseconds(248);
		PORTC = GND;
		delayMicroseconds(495);
	}
}

void one_kilohertz_spike2(void)
{
	while (digitalRead(3) == HIGH) {
		PORTC = TRITRI_VOLTS;
		delayMicroseconds(247);
                PORTB |= _BV(PB2); // offer trigger ;-)
		PORTC = FIVE_VOLTS;
		PORTC = TRITRI_VOLTS;
                PORTB &= ~_BV(PB2);
		delayMicroseconds(248);
		PORTC = GND;
		delayMicroseconds(495);
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
               
                PORTB |= _BV(PB2); // offer trigger
               
		PORTC = RUNT;
                PORTB |= _BV(PB1); // charge the cap a bit
                _delay_us(4);
                PORTB &= ~_BV(PB1); // discharge it again                
		_delay_us(16);
                PORTC = GND;
                
                PORTB &= ~_BV(PB2);

		PORTC = GND;
		_delay_us(20);

		PORTC = FIVE_VOLTS;
		_delay_us(20);
	}
}
