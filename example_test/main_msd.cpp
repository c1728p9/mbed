#include "mbed.h"
#include "SDBlockDevice.h"
#include "USBMSD.h"

Semaphore proc;
SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);


void run_processing()
{
    proc.release();
}

int main_msd() {
    USBMSD usb(&sd);

    sd.frequency(25000000);
    usb.attach(run_processing);
    usb.connect();

    while(proc.wait()) {
        usb.process();
    }

    return 0;
}
