#include <display.h>


void turn_off_screen(void) {
    setPinOutput(GP29);
    writePinLow(GP29);
}

void turn_on_screen(void) {
    setPinOutput(GP29);
    writePinHigh(GP29);
}

void wipe_image(painter_device_t display) {
    qp_rect(display, 0, 0, 130, 130, HSV_BLACK, true);
}