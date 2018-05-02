#include "mbed.h"

int main_cdc();
int main_serial();
int main_keyboard();
int main_mouse();
int main_mouse_keyboard();

int main()
{
    main_cdc();
    main_serial();
    main_keyboard();
    main_mouse();
    main_mouse_keyboard();

    return 0;
}
