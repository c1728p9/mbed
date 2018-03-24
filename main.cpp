#include "mbed.h"

/* Tickless challenge
 *
 * Intstructtions:
 *
 * - Cherry pick support for your board for HAL Sleep and Low Power Ticker APIs from
 *   feature-hal-spec-sleep and feature-hal-spec-ticker on top of tickless_challenge.
 *
 * - Connect Logic Analyser to the board
 *     - CH0 <> D0
 *     - CH1 <> D1
 *     - CH2 <> D2
 *     - GND <> GND
 *
 * run: mbed compile -f --profile release
 *
 * Explanations:
 * CH0 is HIGH when the board is running LOW is when the board is sleeping, it also illustrates systick
 * CH1 is HIGH when the board is in shallow sleep
 * CH2 is HIGH when the board is in deep sleep.
 */

DigitalOut pin_run(D0, 1);
DigitalOut pin_sleep(D1, 0);
DigitalOut pin_deepsleep(D2, 0);

DigitalOut led1(LED1);

// main() runs in its own thread in the OS
int main() {
    while (true) {
        led1 = !led1;
        Thread::wait(500);
    }
}

