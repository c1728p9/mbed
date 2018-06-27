#include "mbed.h"

bool init_done = false;
DigitalOut d0(D0);
DigitalOut d1(D1);
DigitalOut d2(D2);
DigitalOut d3(D3);
extern "C" void set_pin(uint8_t index, uint8_t value)
{
    if (!init_done) {
        return;
    }

    if (index == 0) {
        d0 = value;
    }
    if (index == 1) {
        d1 = value;
    }
    if (index == 2) {
        d2 = value;
    }
    if (index == 3) {
        d3 = value;
    }
}

void dummy()
{

}

volatile uint8_t handle = 0;
int main()
{
    printf("Starting test\r\n");

    init_done = true;

    LowPowerTicker ticker;
    ticker.attach(dummy, 1);


    while (true) {
        // Process lp_ticker_irq_handler outside the interrupt as a test
        core_util_critical_section_enter();
        if (handle) {
            set_pin(3, 1);
            handle = false;
            lp_ticker_irq_handler();
            set_pin(3, 0);
        }
        core_util_critical_section_exit();
    }
}
