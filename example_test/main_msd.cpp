#include "mbed.h"
#include "SDBlockDevice.h"
#include "USBMSD.h"
#include "test_util.h"
#include "usb_phy_api.h"
#include "USBPhyTest.h"


static volatile bool done = false;

void run_processing(Semaphore *sem)
{
    sem->release();
}

void msd_process_main(USBMSD *msd)
{
    Semaphore proc;
    msd->attach(callback(run_processing, &proc));

    while (!done) {
        proc.wait(100);
        msd->process();
    }

    msd->attach(NULL);
}

int main_msd() {

    printf("Attach an SD card to your board and press enter\r\n");
    test_util_wait_line();

    {
        Thread msd_thread(osPriorityHigh);
        SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);
        sd.frequency(25000000);
        USBPhyTest phy(get_usb_phy());
        USBMSD usb(&sd, &phy);
        msd_thread.start(callback(msd_process_main, &usb));

        printf("Connecting USB\r\n");
        test_util_countdown(3);
        usb.connect();

        printf("Did a mass storage drive mount correctly?\r\n");
        MBED_ASSERT(test_util_get_response());

        printf("Disconnecting USB\r\n");
        test_util_countdown(3);
        usb.disconnect();

        printf("Did the mass storage drive dismount correctly?\r\n");
        MBED_ASSERT(test_util_get_response());

        printf("Connecting USB\r\n");
        test_util_countdown(3);
        usb.connect();
        printf("Did a mass storage drive mount correctly?\r\n");
        MBED_ASSERT(test_util_get_response());

        done = true;
        msd_thread.join();

        printf("Deconstructing USB USB\r\n");
        test_util_countdown(3);
    }

    printf("Did the mass storage drive dismount correctly?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("MSD test passed\r\n");
    return 0;
}
