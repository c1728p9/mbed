#include "mbed.h"
#include "USBSerial.h"

static void drain(USBSerial *serial)
{
    while (serial->readable()) {
        serial->getc();
    }
}


int main_serial()
{
    printf("Running USBSerial test\r\n");
    printf("Open a new terminal for this test\r\n");
    USBSerial serial;


    printf("Testing printf/scanf\r\n");
    while (true) {
        serial.printf("Enter a number into this terminal\r\n");

        int number = 0;
        int ret = serial.scanf("%i", &number);
        MBED_ASSERT(ret >= 0); // Something went wrong with scanf
        if (ret == 1) {
            serial.printf("You entered %i\r\n", number);
            break;
        } else {
            serial.printf("Invalid number\r\n");
        }
        drain(&serial);
    }
    serial.printf("\r\n");


    // Test closing terminal
    printf("Testing terminal disconnect\r\n");
    serial.printf("Close your terminal\r\n");
    while (serial.ready()) {
        wait(1);
    }
    printf("Terminal close detected\r\n");
    MBED_ASSERT(serial.configured());
    MBED_ASSERT(!serial.ready());
    printf("Open a new terminal\r\n");
    serial.wait_ready();
    MBED_ASSERT(serial.ready());
    printf("New terminal detected\r\n");
    serial.printf("\r\n");

    // Test device disconnect
    printf("Testing disconnect\r\n");
    serial.printf("Press any key (terminal will disconnect automatically)\r\n");
    serial.getc();
    printf("Terminal disconnecting\r\n");
    serial.disconnect();
    wait(0.5);
    serial.connect();
    printf("Terminal connecting\r\n");
    printf("Open a new terminal\r\n");
    serial.wait_ready();
    serial.printf("\r\n");

    printf("Test passed\r\n");

    return 0;
}
