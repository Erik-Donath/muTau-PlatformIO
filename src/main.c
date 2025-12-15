// src/main.c

#include <generated/csr.h>
#include <generated/soc.h>
#include <generated/mem.h>

int main(void) {
    while (1) {
        leds_out_write(0xAA);
    }

    return 0;
}
