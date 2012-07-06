/*
    This file is part of the USB-GBA multiboot project.

    The USB-GBA multiboot project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The USB-GBA multiboot project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the USB-GBA multiboot project.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_serial.h"

#define BIT_TIME 115200

#define STR(x) (x), sizeof(x)-1
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define INIT_TIMER() do { \
    OCR0A=(F_CPU/115200)-1; \
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
    TCNT0 = (F_CPU/115200)/2; \
    WAIT_TIMER(); \
    } while (0)

#define DATA_OUTPUT() GBA_DDR |= (1<<DATA_BIT);
#define DATA_INPUT() GBA_DDR &= ~(1<<DATA_BIT);

#define GBA_DDR DDRB
#define GBA_OUT PORTB
#define GBA_IN PINB
#define DATA_BIT 0
#define MOSI_BIT 2
#define CLK_BIT 1

#define USB_DEBUG(x) usb_serial_write(x"\n",sizeof(x))

#define WAIT_WITH_TIMEOUT(x, y, z) do { \
    int i = 0, timeout = 1; \
    while (i<(y)) { if (x) { timeout = 0; break; } i++; } \
    if (timeout) { z; } \
    } while (0)

int xfer(uint16_t *data_) {
    uint16_t data = *data_;
    uint8_t start, stop;
    int i;
    cli();

    DATA_OUTPUT();
    GBA_OUT |= (1<<MOSI_BIT); // my turn

    RESET_TIMER();
    GBA_OUT &= ~( (1<<CLK_BIT) | (1<<DATA_BIT) ); //Start bit

    WAIT_TIMER();

    for (i=0; i<16; i++) {
        GBA_OUT = (GBA_OUT & ~(1<<DATA_BIT)) | (((data>>i)&1)<<DATA_BIT);
        WAIT_TIMER();
    }

    GBA_OUT |= (1<<DATA_BIT); //Stop bit
    WAIT_TIMER();
    data = 0;
    GBA_OUT &= ~(1<<MOSI_BIT);
    DATA_INPUT();
    WAIT_WITH_TIMEOUT(!(GBA_IN & (1<<DATA_BIT)),200,return -1);
    // Ready to recieve
    WAIT_TIMER();
    WAIT_HALF_TIMER();

    start = GBA_IN & (1<<DATA_BIT);
    WAIT_TIMER();

    for (i=0; i<16; i++) {
        data |= ((GBA_IN>>DATA_BIT)&1)<<i;
        WAIT_TIMER();
    }

    stop = GBA_IN & (1<<DATA_BIT);
    WAIT_TIMER();
    GBA_OUT |= (1<<MOSI_BIT);

    _delay_ms(1);
    GBA_OUT |= (1<<CLK_BIT);

    sei();
*data_ = data;
    if (!start && stop) {

        return 0;
    }else{
        return -1;
    }
}

int main(void) {
    CPU_PRESCALE(0);

    GBA_DDR |= (1<<MOSI_BIT) | (1<<CLK_BIT);
    GBA_OUT |= (1<<MOSI_BIT) | (1<<CLK_BIT);

    usb_init();
    while (!usb_configured());
    _delay_ms(1000);

    INIT_TIMER();

    while (1) {
        if (usb_serial_available() >= 2) {
            uint16_t data = 0;
            data |= (uint32_t)usb_serial_getchar()<<8;
            data |= (uint32_t)usb_serial_getchar();
            xfer(&data);
            usb_serial_putchar((data>>8) & 0xff);
            usb_serial_putchar(data & 0xff);
        }
    }
    return 0;
}