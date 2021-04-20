
/*
blank=pd5=P5
mosi=pb3=pin11
ssck=pb5=13
load=pc0=A0
awake=pd3=3


*/

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define VFD_DAT_PORT PORTB
#define VFD_DAT_DDR DDRB
#define VFD_DAT PB3
#define VFD_CLK PB5
// #define VFD_DAT 11
// #define VFD_CLK 13

#define VFD_BLANK_PORT PORTD
#define VFD_BLANK_DDR DDRD
#define VFD_BLANK PD5
// #define VFD_BLANK 5

// #define VFD_LOAD A0
#define VFD_LOAD PC0
#define VFD_LOAD_PORT PORTC
#define VFD_LOAD_DDR DDRC

// #define VFD_AWAKE 3
#define VFD_AWAKE PD3
#define VFD_AWAKE_PORT PORTD
#define VFD_AWAKE_DDR DDRD

#include <util/atomic.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "test.h"
#include "usart.h"

#define SET_BIT(n) (1 << (19 - n))

void setup_interrupts();

inline void display_semisemitick(void)
{
}

uint8_t characters[9];

// vfd digit selection wires are on these MAX6921 pins
const uint8_t vfd_digit_pins[] = {
    3,  // digit 9 (dash & circle)
    7,  // digit 8 (leftmost digit)
    8,  // digit 7
    9,  // digit 6
    6,  // digit 5
    10, // digit 4
    5,  // digit 3
    12, // digit 2
    4,  // digit 1 (rightmost digit)
};

const long vfd_digit_pins_bin[] = {
    SET_BIT(3),  // digit 9 (dash & circle)
    SET_BIT(7),  // digit 8 (leftmost digit)
    SET_BIT(8),  // digit 7
    SET_BIT(9),  // digit 6
    SET_BIT(6),  // digit 5
    SET_BIT(10), // digit 4
    SET_BIT(5),  // digit 3
    SET_BIT(12), // digit 2
    SET_BIT(4),  // digit 1 (rightmost digit)
};

// vfd segment selection wires are on these MAX6921 pins
const uint8_t vfd_segment_pins[] = {
    11, // segment H
    16, // segment G
    18, // segment F
    15, // segment E
    13, // segment D
    14, // segment C
    17, // segment B
    19, // segment A
};

// // display of letters and numbers is coded by
// // the appropriate segment flags:
// #define SEG_A 0x80  //
// #define SEG_B 0x40  //        AAA
// #define SEG_C 0x20  //       F   B
// #define SEG_D 0x10  //       F   B
// #define SEG_E 0x08  //        GGG
// #define SEG_F 0x04  //       E   C
// #define SEG_G 0x02  //       E   C
// #define SEG_H 0x01  //        DDD  H

#define SEG_A SET_BIT(19)
#define SEG_B SET_BIT(17)
#define SEG_C SET_BIT(14)
#define SEG_D SET_BIT(13)
#define SEG_E SET_BIT(15)
#define SEG_F SET_BIT(18)
#define SEG_G SET_BIT(16)
#define SEG_H SET_BIT(11)

const long number_segments[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // 0
    SEG_B | SEG_C,                                         // 1
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                 // 2
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                 // 3
    SEG_B | SEG_C | SEG_F | SEG_G,                         // 4
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                 // 5
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         // 6
    SEG_A | SEG_B | SEG_C,                                 // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 8
    SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,                 // 9
};

uint8_t bChars[10];
int dChars[10];

void tick()
{
  for (int i = 0; i < 10; i++)
  {
    asm("nop");
  }
}

uint8_t getCharacterOrRange(uint8_t indx) {
  if (characters[indx] >= 0 && characters[indx] <= 9) {
    return number_segments[characters[indx]];
  }
  return 0;
}

void refresh()
{
  long data = 0;
  for (int i = 0; i <= 8; i++)
  {
    data = vfd_digit_pins_bin[i];
    data |= getCharacterOrRange(i);

    // digitalWrite(VFD_LOAD, LOW);
    VFD_LOAD_PORT &= ~_BV(VFD_LOAD);

    for (uint8_t i = 0; i < 20; i++)
    {
      // digitalWrite(VFD_CLK, LOW);
      VFD_DAT_PORT &= ~_BV(VFD_CLK);
      if (bitRead(data, i))
      {
        VFD_DAT_PORT |= _BV(VFD_DAT);
        tick();
      }
      else
      {
        VFD_DAT_PORT &= ~_BV(VFD_DAT);
        tick();
      }
      // VFD_DAT_PORT (VFD_DAT, bitRead(data, i));
      VFD_DAT_PORT |= _BV(VFD_CLK);
      // digitalWrite(VFD_CLK, HIGH);
    }
    // digitalWrite(VFD_LOAD, HIGH);
    VFD_LOAD_PORT |= _BV(VFD_LOAD);
  }
  // digitalWrite(VFD_BLANK, LOW);
  VFD_BLANK_PORT &= ~_BV(VFD_BLANK);
  tick();
}

int main()
{
  clock_prescale_set(clock_div_1);

  usart_init();
  usart_wake();
  usart_print_str("Init");

  VFD_DAT_DDR |= _BV(VFD_DAT);
  VFD_DAT_DDR |= _BV(VFD_CLK);
  VFD_LOAD_DDR |= _BV(VFD_LOAD);
  VFD_AWAKE_DDR |= _BV(VFD_AWAKE);
  VFD_BLANK_DDR |= _BV(VFD_BLANK);

  VFD_BLANK_PORT &= ~_BV(VFD_BLANK);
  VFD_AWAKE_PORT |= _BV(VFD_AWAKE);
  VFD_LOAD_PORT &= ~_BV(VFD_LOAD);
  VFD_DAT_PORT &= (~_BV(VFD_DAT) & ~_BV(VFD_CLK));

  setup_interrupts();

  sei();
  while (1)
  {
    // busy loop
  }
}


static uint8_t semicounter = 1;
static uint16_t blockcounter = 1;
ISR(TIMER0_OVF_vect)
{
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    if (semicounter && !--semicounter)
    {
      refresh();
      semicounter = 32;
    }
    if (blockcounter && !--blockcounter) {
      blockcounter=300;
      // echo test
      char in = usart_getc();
      while (in != -1) {
        if (in == 'r') {
          for (uint8_t n = 0; n <= 8; n++) {
            characters[n] = -1;
          }
        } else if (in >= '0' && in <= '9' || in == ' ') {
          characters[0] = in == ' ' ? -1 : in - '0';
          for (uint8_t n = 0; n < 8; n++) {
            uint8_t tmp = characters[n+1];
            characters[n+1] = characters[n];
            characters[n] = tmp;
          }
        }
        char *str = "000000000";
        for (uint8_t n = 0; n < 8; n++) {
          str[n] = characters[n]+'0';
        }
        usart_print_str(str);
        in = usart_getc();
      }
    }
  }
}

// runs every second
ISR(TIMER2_COMPB_vect)
{
  NONATOMIC_BLOCK(NONATOMIC_FORCEOFF)
  {
    usart_print_str("stick");
    // count = 0;
  }
}

void setup_interrupts()
{
  for (uint8_t i = 0; i < 8; i++) {
    characters[i] = i%4;
  }
  cli();

  power_timer2_enable(); // enable timer2
  power_timer0_enable();

  // setup timer2 for timekeeping with clock crystal
  ASSR |= _BV(AS2);               // clock with crystal oscillator
  TCCR2A = _BV(WGM21);            // clear counter on compare match
  TCCR2B = _BV(CS22) | _BV(CS21); // divide clock by 256

  // clock crystal resonates at 32.768 kHz and
  // 32,786 Hz / 256 = 128, so set compare match to
  OCR2A = 127; // 128 values, including zero

  // run the once-per-second interrupt when TCNT2 is zero
  OCR2B = 0;

  // call interrupt on compare match (once per second)
  TIMSK2 = _BV(OCIE2B);

  // timer 0
  TCCR0A = _BV(COM0A1) | _BV(COM0B0) | _BV(COM0B1) | _BV(WGM00) | _BV(WGM01);
  TCCR0B = _BV(CS00);  // clock counter0 with system clock
  TIMSK0 = _BV(TOIE0); // enable counter0 overflow interrupt


  DDRD |= (1 << DDD6);
  OCR0A = 180; // 50% duty cycle
  // // setup boost
  TCCR0A = _BV(COM0A1) | _BV(CS00) | _BV(CS01) | _BV(WGM00) | _BV(WGM01);

  usart_print_int(598);

  sei();
}
