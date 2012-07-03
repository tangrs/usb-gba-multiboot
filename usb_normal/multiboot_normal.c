#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_serial.h"

#define BIT_TIME 256000

#define STR(x) (x), sizeof(x)-1
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define INIT_TIMER() do { \
    OCR0A=F_CPU/256000; \
    TCCR0A = 1<<WGM00 | 1<<WGM01 | 1<<COM0A0; \
    TCCR0B = 1<<WGM02 | 1<<CS00 ; \
    TIMSK0 = 0; \
    } while (0)
#define RESET_TIMER() { \
    TCNT0 = 0;  \
    TIFR0 = 1<<OCF0A; \
    } while (0)
#define WAIT_TIMER() do { \
    while((TIFR0 & 1<<OCF0A )==0); \
    TIFR0 = 1<<OCF0A; \
    } while (0)
#define WAIT_HALF_TIMER() do { \
    TCNT0 = BIT_TIME/2; \
    WAIT_TIMER(); \
    } while (0)

#define GBA_DDR DDRB
#define GBA_OUT PORTB
#define GBA_IN PINB
#define MOSI_BIT 2
#define MISO_BIT 3
#define CLK_BIT 1
#define CLK_HIGH() GBA_OUT |= (1<<CLK_BIT)
#define CLK_TOGGLE() GBA_OUT ^= (1<<CLK_BIT)

#define USB_DEBUG(x) usb_serial_write(x"\n",sizeof(x))

#define WAIT_WITH_TIMEOUT(x, y, z) do { \
    int i = 0, timeout = 1; \
    while (i<(y)) { if (x) { timeout = 0; break; } i++; WAIT_TIMER(); } \
    if (timeout) { z; } \
    } while (0)

int xfer(uint32_t *data_);

int xfer(uint32_t *data_) {
    uint32_t data = *data_;
    int i;
    //WAIT_WITH_TIMEOUT(!(GBA_IN&(1<<MISO_BIT)), 25600, goto error);
    cli();

    RESET_TIMER();
    for (i=0;i<32;i++) {
        GBA_OUT &= ~((1<<CLK_BIT) | (1<<MOSI_BIT));

        GBA_OUT |= ((data>>31)&1)<<MOSI_BIT;
        WAIT_TIMER();
        GBA_OUT |= (1<<CLK_BIT);

        data<<=1;
        data |= (GBA_IN>>MISO_BIT)&1;
        WAIT_TIMER();
    }

    sei();
    *data_ = data;
    return 0;

    error:
    return -1;
}

int main(void) {
    CPU_PRESCALE(0);

    GBA_DDR &= ~(1<<MISO_BIT);
    GBA_DDR |= (1<<MOSI_BIT) | (1<<CLK_BIT);
    CLK_HIGH();

    usb_init();
    while (!usb_configured());
    _delay_ms(1000);

    INIT_TIMER();

    while (1) {
        if (usb_serial_available() >= 4) {
            uint32_t data = 0;
            data |= (uint32_t)usb_serial_getchar()<<24;
            data |= (uint32_t)usb_serial_getchar()<<16;
            data |= (uint32_t)usb_serial_getchar()<<8;
            data |= (uint32_t)usb_serial_getchar();
            xfer(&data);
            usb_serial_putchar((data>>24) & 0xff);
            usb_serial_putchar((data>>16) & 0xff);
            usb_serial_putchar((data>>8) & 0xff);
            usb_serial_putchar(data & 0xff);
            usb_serial_flush_output();
        }
    }
    return 0;
}