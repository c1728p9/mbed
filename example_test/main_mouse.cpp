#include "mbed.h"
#include "USBMouse.h"
#include "test_util.h"

int main_mouse()
{
    printf("Running USBMouse test\r\n");
    USBMouse mouse;

    printf("Press enter\r\n");
    test_util_wait_line();
    for (int i = 0; i < 10; i++) {
        mouse.move(20, 20);
        wait(0.1);
    }
    printf("Did the mouse move to the bottom right?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter\r\n");
    test_util_wait_line();
    for (int i = 0; i < 10; i++) {
        mouse.move(-20, -20);
        wait(0.1);
    }
    printf("Did the mouse move to the top left?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a scrollable window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    for (int i = 0; i < 10; i++) {
        mouse.scroll(20);
        wait(0.1);
    }
    printf("Did the mouse scroll down?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a scrollable window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    for (int i = 0; i < 10; i++) {
        mouse.scroll(-20);
        wait(0.1);
    }
    printf("Did the mouse scroll up?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a window with text\r\n");
    printf("left click will be held for 3 seconds\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    printf("left click pressed\r\n");
    mouse.press(MOUSE_LEFT);
    wait(3);
    mouse.release(MOUSE_LEFT);
    printf("left click released\r\n");
    printf("Was the left mouse click pressed for 3 seconds\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    mouse.click(MOUSE_RIGHT);
    printf("Was the right mouse clicked\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("mouse test passed\r\n");

    return 0;
}
