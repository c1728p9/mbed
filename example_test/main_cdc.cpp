#include "mbed.h"
#include "USBCDC.h"
#include "USBSerial.h"
#include "test_util.h"

static Mutex m;
#define sync_printf(...) {m.lock(); printf(__VA_ARGS__); m.unlock();}

static volatile uint32_t read_count = 0;
static volatile bool example_running = true;
static volatile bool send_data = false;
static void write_main(USBCDC *cdc)
{
    uint64_t count = 0;
    while (example_running) {
        uint8_t buf[128];
        sprintf((char*)buf, "Writer thread %llu\r\n", count);
        if (send_data && cdc->send(buf, strlen((char*)buf))) {
            count++;
        } else if (send_data) {
            sync_printf("Writer thread 1 waiting for connection\r\n");
            wait(1);
        } else {
            wait(0.1);
        }

    }
}

static void read_main(USBCDC *cdc)
{
    while (example_running) {
        uint8_t data;
        uint32_t size_read;
        if (cdc->receive(&data, 1, &size_read)) {
            sync_printf("Read: %c\r\n", (char)data);
            read_count++;
        } else {
            sync_printf("Reader thread 1 waiting for connection\r\n");
            wait(1);
        }
    }
}

int main_cdc()
{
    printf("Running USBCDC test\r\n");

    printf("Open serial port\r\n");
    USBCDC serial(0x0d28, 0x0206, 0x0001, true);
    Thread t1(osPriorityHigh);
    Thread t2(osPriorityHigh);
    Thread t3(osPriorityHigh);
    t1.start(callback(write_main, &serial));
    t2.start(callback(read_main, &serial));

    printf("Enter 5 characters\r\n");
    read_count = 0;
    while (read_count < 5) {
        wait(0.1);
    }
    printf("Were the characters you typed printed correctly?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter to start concurrent test\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    send_data = true;
    printf("Transmitter active\r\n");
    printf("Enter 5 characters\r\n");
    read_count = 0;
    while (read_count < 5) {
        wait(0.1);
    }
    send_data = false;
    printf("Were the characters you typed printed correctly?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter to disconnect\r\n");
    test_util_wait_line();
    example_running = false;
    serial.disconnect();
    t1.join();
    t2.join();

    printf("USBCDC test pass\r\n");

    return 0;
}
