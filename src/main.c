// src/main.c

#include <generated/csr.h>
#include <generated/soc.h>
#include <generated/mem.h>

static void delay(void) {
    volatile unsigned int i;
    for (i = 0; i < 1000000; i++) {
        __asm__ volatile("nop");
    }
}

int main(void) {
    while (1) {
        // 0xAA pattern (10101010)
        leds_out_write(0xAA);
        delay();

        // 0x55 pattern (01010101)
        leds_out_write(0x55);
        delay();
    }

    return 0;
}
