#include "mbed.h"
#include "test_env.h"
#include "rtos.h"

#if defined(MBED_RTOS_SINGLE_THREAD)
  #error [NOT_SUPPORTED] test not supported
#endif

#define NUM_THREADS         3

DigitalOut led1(LED1);
volatile bool should_exit = false;

void task_using_io()
{
    char buf[32];
    int len;
    int count = 0;
    while (1) {
        // Repeatedly print to standard out
        len = sprintf(buf, "count %i\r\n", count);
        fwrite(buf, 1, len, stdout);
        count += 1;

        if (should_exit) {
            return;
        }
    }
}

int main()
{
    Thread *thread_list[NUM_THREADS];
    int test_time = 15;
    bool allocation_failure = false;
    GREENTEA_SETUP(20, "default_auto");

    // Allocate threads for the test
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_list[i] = new Thread();
        if (NULL == thread_list[i]) {
            allocation_failure = true;
        }
        thread_list[i]->start(task_using_io);
    }

    // Give the test time to run
    while (test_time) {
        led1 = !led1;
        Thread::wait(1000);
        test_time--;
    }

    // Join and delete all threads
    should_exit = 1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (NULL == thread_list[i]) {
            continue;
        }
        thread_list[i]->join();
        delete thread_list[i];
    }

    GREENTEA_TESTSUITE_RESULT(!allocation_failure);
}
