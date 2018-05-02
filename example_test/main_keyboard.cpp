#include "mbed.h"
#include "USBKeyboard.h"
#include "test_util.h"


static void test_typing(USBKeyboard *keyboard)
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

static void test_lock(const char *name, uint32_t mask, USBKeyboard *keyboard)
{
    printf("Turn on %s lock\r\n", name);
    while (!(keyboard->lock_status() & mask));
    printf("%s lock ON\r\n\r\n", name);
    printf("Turn off %s lock\r\n", name);
    while (keyboard->lock_status() & mask);
    printf("%s lcok OFF\r\n\r\n", name);
}

int main_keyboard()
{
    printf("Running USBKeyboard test\r\n");
    USBKeyboard keyboard;

    test_typing(&keyboard);
    test_lock("num", 0x1, &keyboard);
    test_lock("caps", 0x2, &keyboard);
    test_lock("scroll", 0x4, &keyboard);

    printf("keyboard test passed\r\n");

    return 0;
}
