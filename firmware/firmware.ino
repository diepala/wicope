/*
          +------------------+
          |   OSCILLOSCOPE   |
          +------------------+

   Connect signal to A0
   Connect trigger threshold level to PIN 7
   Connect trigger signal to PIN 6

*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// For the serial communication
#define USART_BAUDRATE 115200
#define BAUD_PRESCALLER ((F_CPU / (USART_BAUDRATE * 16UL))) //datasheet says to add a -1 to the formula but does not work. It's better to remove the -1.

#define ADC_BUFFER_SIZE 100
#define ADCPIN 0

// Codes to communicate with the PC
#define START 0x10            // start acquisition
#define SET_TIME_BASE_1 0x21  // 20 us/div (1div = 10 samples)
#define SET_TIME_BASE_2 0x22  // 50 us/div
#define SET_TIME_BASE_3 0x23  // 100 us/div
#define SET_TIME_BASE_4 0x24  // 200 us/div
#define SET_TIME_BASE_5 0x25  // 500 us/div
#define SET_TIME_BASE_6 0x26  // 1 ms/div
#define SET_TIME_BASE_7 0x27  // 2 ms/div
#define SET_TIME_BASE_8 0x28  // 5 ms/div
#define SET_TIME_BASE_9 0x29  // 10 ms/div
#define SET_TIME_BASE_10 0x2a // 20 ms/div
#define TRIGGER_ENABLE 0x31
#define TRIGGER_DISABLE 0x32
#define TRIGGER_EDGE_RISING 0x33
#define TRIGGER_EDGE_FALLING 0x34
#define TRIGGER_EDGE_ANY 0x35

volatile struct Settings
{
  uint8_t trigger_mode = 0; //0 rising 1 falling 2 toggle
  uint8_t trigger_on = 0;   //1 on 0 off
} settings;

volatile uint16_t adc_pos = 0;
volatile uint16_t adc_stop = 0;
volatile uint8_t adc_buffer[ADC_BUFFER_SIZE];
volatile uint8_t wait = 1;

int main(void)
{
  // Configure serial communication
  USART_init();

  // Configure ADC
  ADC_init();

  // Configure Analog Comparator
  AnalogComparator_init();

  // Configure Timer1
  timer1_init();

  // Global interrupt enable flag
  sei();

  uint8_t in_command;

  while (1)
  {

    in_command = USART_read();

    switch (in_command)
    {
      case START:

        wait = 1;

        // Check trigger activated or not
        if (settings.trigger_on)
        { //trigger_on
          adc_stop = 0xffff; //never reach that value

          // Enable Analog Comparator Interrupt
          sbi(ACSR, ACIE);
        }
        else
        { //trigger off
          adc_stop = adc_pos;
          // Dissable Analog Comparator Interrupt
          cbi(ACSR, ACIE);
        }

        // Enable timer
        sbi(TCCR1B, CS10);

        while (wait)
          ; // Wait to finish the acquisition.

        // Send data to host
        USART_send_adc_buffer();
        break;

      // 10 samples per division, 10 divisions
      // minimum 14 ADC clocks per conversion
      case SET_TIME_BASE_1: // 20 us/div
        // ADC prescaler to 2
        cbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 32 - 1;
        break;
      case SET_TIME_BASE_2: // 50 us/div
        // ADC prescaler to 4
        cbi(ADCSRA, ADPS2);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 80 - 1;
        break;
      case SET_TIME_BASE_3: // 100 us/div
        // ADC prescaler to 8
        cbi(ADCSRA, ADPS2);
        sbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 160 - 1;
        break;
      case SET_TIME_BASE_4: // 200 us/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 320 - 1;
        break;
      case SET_TIME_BASE_5: // 500 us/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 800 - 1;
        break;
      case SET_TIME_BASE_6: // 1 ms/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 1600 - 1;
        break;
      case SET_TIME_BASE_7: // 2 ms/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 3200 - 1;
        break;
      case SET_TIME_BASE_8: // 5 ms/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 8000 - 1;
        break;
      case SET_TIME_BASE_9: // 10 ms/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 16000 - 1;
        break;
      case SET_TIME_BASE_10: // 20 ms/div
        // ADC prescaler to 16
        sbi(ADCSRA, ADPS2);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        // Timer count
        OCR1A = 32000 - 1;
        break;

      case TRIGGER_ENABLE:
        settings.trigger_on = 1;
        break;
      case TRIGGER_DISABLE:
        settings.trigger_on = 0;
        break;
      case TRIGGER_EDGE_RISING:
        settings.trigger_mode = 0;
        set_trigger_edge();
        break;
      case TRIGGER_EDGE_FALLING:
        settings.trigger_mode = 1;
        set_trigger_edge();
        break;
      case TRIGGER_EDGE_ANY:
        settings.trigger_mode = 2;
        set_trigger_edge();
        break;

      default:
        break;
    }
  }
}

void timer1_init(void)
{
  // Configure timer to CRC mode (Clear Timer on Compare match) with OCR1A
  cbi(TCCR1B, WGM13);
  sbi(TCCR1B, WGM12);
  cbi(TCCR1A, WGM11);
  cbi(TCCR1A, WGM10);

  // The three clock select bits select the clock source to be used by the Timer/Counter
  // CS12 CS11 CS10 Description
  // 0 0 0 No clock source (Timer/Counter stopped).
  // 0 0 1 clk/1 (no prescaling)
  // 0 1 0 clk/8 (from prescaler)
  // 0 1 1 clk/64 (from prescaler)
  // 1 0 0 clk/256 (from prescaler)
  // 1 0 1 clk/1024 (from prescaler)
  // 1 1 0 External clock source on T1 pin. Clock on falling edge.
  // 1 1 1 External clock source on T1 pin. Clock on rising edge
  cbi(TCCR1B, CS12);
  cbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  // Set the output compare register to a value to count. It's a 16 bit register.
  OCR1A = 32000;  // To reset de counter
  OCR1B = 0x0000; // To trigger the adc

  // When this bit is written to one, and the I-flag in the status register is set
  // (interrupts globally enabled), the Timer/Counter1 output compare A match
  // interrupt is enabled. The corresponding interrupt vector is executed when
  // the OCF1A flag, located in TIFR1, is set.
  sbi(TIMSK1, OCIE1B);
}

void USART_init(void)
{
  UBRR0H = (uint8_t)(BAUD_PRESCALLER >> 8);
  UBRR0L = (uint8_t)(BAUD_PRESCALLER);
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

uint8_t USART_read(void)
{
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

void USART_send(uint8_t data)
{
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
}

void USART_send_adc_buffer(void)
{
  uint16_t i;
  for (i = 0; i < ADC_BUFFER_SIZE; i++)
  {
    USART_send(adc_buffer[adc_pos]);
    if (++adc_pos >= ADC_BUFFER_SIZE)
      adc_pos = 0;
  }
}

void set_trigger_edge()
{
  // 0 0 Toggle
  // 0 1 Reserved
  // 1 0 Falling edge
  // 1 1 Rising edge
  switch (settings.trigger_mode)
  {
    case 1: // falling
      sbi(ACSR, ACIS1);
      cbi(ACSR, ACIS0);
      break;

    case 2: // toggle
      cbi(ACSR, ACIS1);
      cbi(ACSR, ACIS0);
      break;

    default: // rising
      sbi(ACSR, ACIS1);
      sbi(ACSR, ACIS0);
      break;
  }
}

void ADC_init(void)
{
  //---------------------------------------------------------------------
  // ADMUX settings
  //---------------------------------------------------------------------
  // TSet ADC voltage reference to (0 1): AVCC with external capacitor at AREF pin
  cbi(ADMUX, REFS1);
  sbi(ADMUX, REFS0);

  // Set one to ADLAR to left adjust the
  // result in the ADC Data Register.
  sbi(ADMUX, ADLAR);

  // Select the analog pin connected to the ADC
  // ADMUX |= 0x0f; Connect to ground
  ADMUX |= (ADCPIN & 0x07);

  //---------------------------------------------------------------------
  // ADCSRA settings
  //---------------------------------------------------------------------
  // Enable the ADC
  sbi(ADCSRA, ADEN);

  // Enable auto trigger. In the ADCSR it will be configured to start conversion when a timer overflows.
  sbi(ADCSRA, ADATE);

  // When this bit is written to one and the I-bit in SREG is set, the
  // ADC Conversion Complete Interrupt is activated.
  sbi(ADCSRA, ADIE);

  // These bits determine the division factor between the system clock
  // frequency and the input clock to the ADC.
  // ADPS2 ADPS1 ADPS0 Division Factor
  // 0 0 0 2
  // 0 0 1 2
  // 0 1 0 4
  // 0 1 1 8
  // 1 0 0 16
  // 1 0 1 32
  // 1 1 0 64
  // 1 1 1 128
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  //---------------------------------------------------------------------
  // ADCSRB settings
  //---------------------------------------------------------------------
  // If ADATE in ADCSRA is written to one, the value of these bits
  // selects which source will trigger an ADC conversion. A conversion will
  // be triggered by the rising edge of the selected Interrupt Flag. If ADEN in ADCSRA is set, this will start a conversion.
  // ADTS2 ADTS1 ADTS0 Trigger source
  // 0 0 0 Free Running mode
  // 0 0 1 Analog Comparator
  // 0 1 0 External Interrupt Request 0
  // 0 1 1 Timer/Counter0 Compare Match A
  // 1 0 0 Timer/Counter0 Overflow
  // 1 0 1 Timer/Counter1 Compare Match B
  // 1 1 0 Timer/Counter1 Overflow
  // 1 1 1 Timer/Counter1 Capture Event
  sbi(ADCSRB, ADTS2);
  cbi(ADCSRB, ADTS1);
  sbi(ADCSRB, ADTS0);

  //---------------------------------------------------------------------
  // DIDR0 settings
  //---------------------------------------------------------------------
  // When this bit is written logic one, the digital input buffer on the
  // corresponding ADC pin is disabled. The corresponding PIN Register
  // bit will always read as zero when this bit is set. When an analog
  // signal is applied to the ADC5..0 pin and the digital input from this
  // pin is not needed, this bit should be written logic one to reduce
  // power consumption in the digital input buffer.
  sbi(DIDR0, ADC5D);
  sbi(DIDR0, ADC4D);
  sbi(DIDR0, ADC3D);
  sbi(DIDR0, ADC2D);
  sbi(DIDR0, ADC1D);
  sbi(DIDR0, ADC0D);
}

void AnalogComparator_init(void)
{
  //---------------------------------------------------------------------
  // ACSR settings
  //---------------------------------------------------------------------
  // When this bit is written logic one, the power to the Analog
  // Comparator is switched off. This bit can be set at any time to turn
  // off the Analog Comparator. This will reduce power consumption in
  // Active and Idle mode. When changing the ACD bit, the Analog
  // Comparator Interrupt must be disabled by clearing the ACIE bit in
  // ACSR. Otherwise an interrupt can occur when the bit is changed.
  cbi(ACSR, ACD);
  // When this bit is set, a fixed bandgap reference voltage replaces the
  // positive input to the Analog Comparator. When this bit is cleared,
  // AIN0 is applied to the positive input of the Analog Comparator. When
  // the bandgap referance is used as input to the Analog Comparator, it
  // will take a certain time for the voltage to stabilize. If not
  // stabilized, the first conversion may give a wrong value.
  cbi(ACSR, ACBG);
  // When the ACIE bit is written logic one and the I-bit in the Status
  // Register is set, the Analog Comparator interrupt is activated.
  // When written logic zero, the interrupt is disabled.
  cbi(ACSR, ACIE);
  // When written logic one, this bit enables the input capture function
  // in Timer/Counter1 to be triggered by the Analog Comparator. The
  // comparator output is in this case directly connected to the input
  // capture front-end logic, making the comparator utilize the noise
  // canceler and edge select features of the Timer/Counter1 Input
  // Capture interrupt. When written logic zero, no connection between
  // the Analog Comparator and the input capture function exists. To
  // make the comparator trigger the Timer/Counter1 Input Capture
  // interrupt, the ICIE1 bit in the Timer Interrupt Mask Register
  // (TIMSK1) must be set.
  cbi(ACSR, ACIC);
  // These bits determine which comparator events that trigger the Analog
  // Comparator interrupt.
  // ACIS1 ACIS0 Mode
  // 0 0 Toggle
  // 0 1 Reserved
  // 1 0 Falling edge
  // 1 1 Rising edge
  sbi(ACSR, ACIS1);
  sbi(ACSR, ACIS0);

  //---------------------------------------------------------------------
  // DIDR1 settings
  //---------------------------------------------------------------------
  // When this bit is written logic one, the digital input buffer on the
  // AIN1/0 pin is disabled. The corresponding PIN Register bit will
  // always read as zero when this bit is set. When an analog signal is
  // applied to the AIN1/0 pin and the digital input from this pin is not
  // needed, this bit should be written logic one to reduce power
  // consumption in the digital input buffer.
  sbi(DIDR1, AIN1D);
  sbi(DIDR1, AIN0D);
}

//-----------------------------------------------------------------------------
// ADC Conversion Complete Interrupt
//-----------------------------------------------------------------------------
ISR(ADC_vect)
{
  // When ADCL is read, the ADC Data Register is not updated until ADCH
  // is read. Consequently, if the result is left adjusted and no more
  // than 8-bit precision is required, it is sufficient to read ADCH.
  // Otherwise, ADCL must be read first, then ADCH.

  adc_buffer[adc_pos] = ADCH;

  if (++adc_pos >= ADC_BUFFER_SIZE)
    adc_pos = 0;

  if (adc_pos == adc_stop)
  {
    // Disable timer
    cbi(TCCR1B, CS10);
    wait = 0;
  }
}

//-----------------------------------------------------------------------------
// Timer 1 interrupt
//-----------------------------------------------------------------------------
ISR(TIMER1_COMPB_vect)
{
  // Empty interrupt to clear the Timer1 interrupt flag.
}

//-----------------------------------------------------------------------------
// Analog Comparator interrupt
//-----------------------------------------------------------------------------
ISR(ANALOG_COMP_vect)
{
  // Disable Analog Comparator interrupt
  cbi(ACSR, ACIE);

  adc_stop = adc_pos;
  //adc_stop = ( adc_pos + 256 ) % ADC_BUFFER_SIZE;
}
