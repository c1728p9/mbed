#include "mbed.h"
#include "USBMouseKeyboard.h"
#include "test_util.h"

static void test_typing(USBMouseKeyboard *keyboard)
{
    while (true) {
        const char test_msg[] = "`1234567890-=qqwertyuiop[]\\asdfghjkl;zxcvbnm,./";
        printf("Press enter and then an empty text editor\r\n");
        test_util_wait_line();

        test_util_countdown(3);
        keyboard->printf("\n\n%s\n\n", test_msg);

        printf("Paste the characters into the console and press enter\r\n");

        char buf[128];
        memset(buf, 0, sizeof(buf));
        test_util_read_line(buf, sizeof(buf));
        bool same = strcmp(buf, test_msg) == 0;
        if (same) {
            printf("Data matches\r\n");
            break;
        } else {
            printf("Invalid text entered\r\n");
        }
    }
}

static void test_lock(const char *name, uint32_t mask, USBMouseKeyboard *keyboard)
{
    printf("Turn on %s lock\r\n", name);
    while (!(keyboard->lock_status() & mask));
    printf("%s lock ON\r\n\r\n", name);
    printf("Turn off %s lock\r\n", name);
    while (keyboard->lock_status() & mask);
    printf("%s lcok OFF\r\n\r\n", name);
}

int main_mouse_keyboard()
{
    printf("Running USBMouseKeyboard test\r\n");
    USBMouseKeyboard mouse_keyboard;

    test_typing(&mouse_keyboard);
    test_lock("num", 0x1, &mouse_keyboard);
    test_lock("caps", 0x2, &mouse_keyboard);
    test_lock("scroll", 0x4, &mouse_keyboard);

    printf("Press enter\r\n");
    test_util_wait_line();
    for (int i = 0; i < 10; i++) {
        mouse_keyboard.move(20, 20);
        wait(0.1);
    }
    printf("Did the mouse move to the bottom right?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter\r\n");
    test_util_wait_line();
    for (int i = 0; i < 10; i++) {
        mouse_keyboard.move(-20, -20);
        wait(0.1);
    }
    printf("Did the mouse move to the top left?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a scrollable window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    for (int i = 0; i < 10; i++) {
        mouse_keyboard.scroll(20);
        wait(0.1);
    }
    printf("Did the mouse scroll down?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a scrollable window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    for (int i = 0; i < 10; i++) {
        mouse_keyboard.scroll(-20);
        wait(0.1);
    }
    printf("Did the mouse scroll up?\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a window with text\r\n");
    printf("left click will be held for 3 seconds\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    printf("left click pressed\r\n");
    mouse_keyboard.press(MOUSE_LEFT);
    wait(3);
    mouse_keyboard.release(MOUSE_LEFT);
    printf("left click released\r\n");
    printf("Was the left mouse click pressed for 3 seconds\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("Press enter then select a window\r\n");
    test_util_wait_line();
    test_util_countdown(3);
    mouse_keyboard.click(MOUSE_RIGHT);
    printf("Was the right mouse clicked\r\n");
    MBED_ASSERT(test_util_get_response());

    printf("USBMouseKeyboard test passed\r\n");

    return 0;
}
