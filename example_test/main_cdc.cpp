#include "mbed.h"
#include "USBCDC.h"
#include "USBSerial.h"
#include "test_util.h"

static Mutex m;
#define sync_printf(...) {m.lock(); printf(__VA_ARGS__); m.unlock();}

static volatile bool example_running = true;
static volatile bool send_data = false;
static void thread_1(USBCDC *cdc)
{
    uint64_t count = 0;
    while (example_running) {
        uint8_t buf[128];
        sprintf((char*)buf, "Thread 1 %llu\r\n", count);
        if (send_data && cdc->send(buf, strlen((char*)buf))) {
            count++;
            //wait(0.01);
        } else if (send_data) {
            wait(1);
            sync_printf("Thread 1 waiting for connection 2\r\n");
        } else {
            wait(0.1);
        }

    }
}

static void thread_2(USBCDC *cdc)
{
    while (example_running) {
        uint8_t data;
        uint32_t size_read;
        if (cdc->receive(&data, 1, &size_read)) {
            sync_printf("Read %c %i\r\n", (char)data, (int)size_read);
            if (data == '1') {
                send_data = true;
            } else if (data == '2') {
                send_data = false;
            }
        } else {
            wait(1.0);
            sync_printf("Thread 3 waiting for connection 2\r\n");
        }
    }
}

static void thread_3(USBCDC *cdc)
{
    while (example_running) {
        uint8_t data[2];
        if (cdc->receive(data, sizeof(data), NULL)) {
            sync_printf("Read2 %c%c\r\n", (char)data[0], (char)data[1]);
        } else {
            wait(1.0);
            sync_printf("Thread 4 waiting for connection 2\r\n");
        }
    }
}

int main_cdc()
{
    USBCDC serial(0x0d28, 0x0206, 0x0001, true);

    Thread t1;
    Thread t2;
    Thread t3;

    t1.start(callback(thread_1, &serial));
    t2.start(callback(thread_2, &serial));
    t3.start(callback(thread_3, &serial));

    printf("Press enter to finish test\r\n");
    test_util_wait_line();
    example_running = false;
    serial.disconnect();

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
