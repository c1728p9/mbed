#include "mbed.h"

int main_cdc();
int main_serial();
int main_keyboard();
int main_mouse();
int main_mouse_keyboard();

//void cb1(void*)
//{
//
//    printf("It ran!\r\n");
//}
//
//void cb2(void*)
//{
//
//    printf("Test 2 failed!\r\n");
//}
//void cb3(void* q);
//
//uint32_t queue_buf[1];
//TaskBase event3((uint8_t*)queue_buf, sizeof(queue_buf), cb3);
//void cb3(void* q)
//{
//    EventQueue *queue = *(EventQueue **)q;
//    wait(1);
//    printf("SPAM!\r\n");
//    queue->post(&event3);
//}
//
//void print_number(void *num)
//{
//    printf("My number is %i\r\n", (int)num);
//}

class A {
public:
    static void print() {
        printf("Hi from A\r\n");
    }
};

class B {
public:
    static void print() {
        printf("Hi from B\r\n");
    }
};

template<typename T>
void run_print()
{
    T::print();
}

int main()
{
    run_print<A>();
    run_print<B>();
//    printf("Test 1\r\n");
//    {
//    EventQueue queue;
//
//    TaskBase event1(NULL, 0, cb1);
//    TaskBase event2(NULL, 0, cb2);
//
//    EventQueue *ptr = &queue;
//    memcpy((void*)queue_buf, (void*)&ptr, sizeof(&queue));
//
//
//    queue.post(&event1);
//    queue.post(&event2);
//    event2.cancel();
//    queue.post(&event3);
//
//    Callback<void()> cb(print_number, (void*)7);
//    Task< Callback<void()> > task(cb);
//    queue.post(&task);
//
//    queue.dispatch();
//    }

//    printf("Test 2\r\n");
//    {
//        EventQueue queue;
//
//        StaticEvent event(NULL, 0, cb);
//
//
//        queue.post(&event);
//        event.cancel();
//
//        queue.dispatch();
//    }
//
//    printf("Test 3\r\n");
//    {
//        EventQueue queue;
//
//        StaticEvent event(NULL, 0, cb);
//
//
//        queue.post(&event);
//        queue.cancel(&event);
//
//        queue.dispatch();
//    }

    main_cdc();
    main_serial();
    main_keyboard();
    main_mouse();
    main_mouse_keyboard();

    return 0;
}
