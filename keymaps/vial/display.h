#ifndef DISPLAY_H
#define DISPLAY_H

#include <qp.h>
#include "qp_comms.h"
#include "color.h"
#include "raw_hid.h"

void turn_off_screen(void);
void turn_on_screen(void);
void wipe_image(painter_device_t display);

#endif
