// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <qp.h>
#include "gifs/sanacut.qgf.h"
#include "gifs/monaco129.qgf.h"
#include "fonts/scp.qff.h"
#include "qp_st77xx_opcodes.h"
#include "qp_st7735_opcodes.h"
#include "qp_comms.h"
#include "color.h"
#include "raw_hid.h"
#define DEBUG_ENABLED false
// #define DEBUG_ENABLED true

static char textArr[31];
static char songArr[31];
static uint8_t image_data[8192];
static int image_counter = 0;

enum layer_names{
    _NUMPAD,
    _MEDIA,
    _LAYER3,
    _LAYER4
};

// static char test[] = "Initial";

// uint8_t framebuffer[SURFACE_REQUIRED_BUFFER_BYTE_SIZE(WIDTH, HEIGHT, 16)] = {0}; // this is where your image data is stored
// painter_device_t surface = qp_make_rgb565_surface(WIDTH, HEIGHT, (void *)framebuffer);

static painter_device_t display = NULL;
static painter_image_handle_t image = NULL;
static painter_font_handle_t my_font;
static deferred_token my_anim;
static bool animating = false;
static bool layer_name_displayed = false;
static int messages_since_layer_state = 0;
// static bool special_anim = false;
static bool album_art = false;
static bool timed_out = false;
static bool caps_on;
// static int playing = 0;

void turn_off_screen(void){
    setPinOutput(GP29);
    writePinLow(GP29);
}

void turn_on_screen(void){
    setPinOutput(GP29);
    writePinHigh(GP29);
}

void wipe_image(void){
    qp_rect(display, 0,0,130, 131, HSV_BLACK, true);
}

uint8_t* doubleArray(uint8_t* originalArray, int originalSize){
    uint8_t* newArr = malloc(2*originalSize*sizeof(uint8_t));
    for (int i = 0; i < originalSize; i+=2){
        newArr[i*2]     = originalArray[i];
        newArr[i*2+1]   = originalArray[i+1];
        newArr[i*2+2]   = originalArray[i];
        newArr[i*2+3]   = originalArray[i+1];
    }
    return newArr;
}


// hid function
void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    switch (data[0])
    {
        // new song string
        case 0xFF:
            // uprintf("New song string recieved\n");
            if (data[1]){
                album_art = true;
                messages_since_layer_state += 1;
                if (animating){
                    animating = false;
                    qp_stop_animation(my_anim);
                }
                if (((strncmp(songArr,(char *)(data+2),strlen(songArr)-1) != 0) || layer_name_displayed) && messages_since_layer_state > 3){
                    qp_rect(display, 0,132,131, 162, HSV_BLACK, true);
                    qp_drawtext(display, 2, 138, my_font, (char *)(data+2));
                    strcpy(songArr,(char *)(data+2));
                    layer_name_displayed = false;
                }
            }
            else {
                strcpy(songArr,"\0");
                qp_rect(display, 0,130,131, 162, HSV_BLACK, true);
                // return;
                album_art = false;
            }
            break;
        // new image data (1st hid message)
        case 0xFD:
            // uprintf("New image recieved\n");
            image_counter = 0;
            if (animating){
                animating = false;
                qp_stop_animation(my_anim);
            }
            memcpy(image_data+image_counter,data+1,30);
            image_counter += 30;
            break;
        // intermediate image data
        case 0xEE:
            // uprintf("New intermediate image data recieved\n");
            memcpy(image_data+image_counter,data+1,30);
            image_counter += 30;
            break;
        // final image data (last 2 bytes of 8192)
        case 0xFC:
            // uprintf("New final image data recieved\n");
            wipe_image();
            // uprintf("Final data received, writing to screen\n");
            memcpy(image_data+image_counter,data+1,2);
            uint8_t* pixels = doubleArray(image_data,8192);
            for (int i = 0,c = 0; i < 128; i+=2,c+=1){
                // l, t, r, b
                // draw to 2 columns
                qp_viewport(display, i, 0, i, 128);
                qp_pixdata(display, pixels+c*256, 128);
                qp_viewport(display, i+1, 0, i+1, 128);
                qp_pixdata(display, pixels+c*256, 128);
            }
            free(pixels);
            image_counter = 0;
            break;
        // redraw old art (song didnt change since pausing)
        case 0xFB:
            // uprintf("Re-drawing old art\n");
            if (animating){
                qp_stop_animation(my_anim);
                animating = false;
            }
            album_art = true;
            wipe_image();
            pixels = doubleArray(image_data,8192);
            for (int i = 0,c = 0; i < 128; i+=2,c+=1){
                // l, t, r, b
                // draw to 2 columns
                qp_viewport(display, i, 0, i, 128);
                qp_pixdata(display, pixels+c*256, 128);
                qp_viewport(display, i+1, 0, i+1, 128);
                qp_pixdata(display, pixels+c*256, 128);
            }
            free(pixels);
        case 0xFA:
            ;
            // uprintf("got new progress info\n");
            int length = data[1];
            if (data[2] == 1){
                qp_rect(display, 0, 129, 131, 131, 0, 0, 0, true);
            }
            // uprintf("data value = %d pixels",length);
            // qp_line(device, x0, y0, x1, y1, hue, sat, val);
            // qp_line(display, 0, 129,length,129,255,0,255);
            // qp_rect(device, left, top, right, bottom, hue, sat, val, filled);
            qp_rect(display, 0, 129, length, 131, 255, 0, 255, true);
    }
}

// bool timeout_task(bool &album_art, bool &timed_out, bool &animating, int activity){
//     if
// }

void writeLayerState(char* toWrite){
    qp_rect(display, 0,132,131, 162, HSV_BLACK, true);
    qp_drawtext(display, 2, 138, my_font, (char *)(toWrite));
}

void housekeeping_task_user(void){
    int last_activity = last_input_activity_elapsed();
    rgblight_set();
    // optionally last_encoder_activity_elapsed() instead
    if (!album_art){
        if (!timed_out){
            // timeout screen no activity
            if (last_activity > 10000){
                turn_off_screen();
                timed_out = true;
                if (animating){
                    qp_stop_animation(my_anim);
                    animating = false;
                }
            }
            // disable animation for encoder
            else if (last_activity < 1000){
                if (animating){
                    qp_stop_animation(my_anim);
                    wipe_image();
                    // possibly do layer stuff here
                    animating = false;
                }
            }
            // turn animation back on
            else if (!animating){
                my_anim = qp_animate(display,0,0,image);
                animating = true;
            }

        }
        // turn screen back on after timeout
        else {
            if (last_activity < 10000){
                turn_on_screen();
                if (!animating){
                    my_anim = qp_animate(display,0,0,image);
                    animating = true;
                }
                timed_out = false;
            }
        }
    }
    else {
        // keep screen on and do album stuff
        if (timed_out){
            turn_on_screen();
            timed_out = false;
        }
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    caps_on = host_keyboard_led_state().caps_lock;
    messages_since_layer_state = 0;
    switch (get_highest_layer(state)){
        case    _NUMPAD:
            writeLayerState("Numpad Layer");
            if (!caps_on) {
                sethsv(127,255,30, (rgb_led_t *)&led[0]);
            }
            break;
        case    _MEDIA:
            writeLayerState("Media Layer");
            if (!caps_on) {
                sethsv(201,255,30, (rgb_led_t *)&led[0]);
            }
            break;
        case    _LAYER3:
            writeLayerState("Navigation Layer");
            if (!caps_on) {
                sethsv(85,255,30, (rgb_led_t *)&led[0]);
            }
            break;
        case    _LAYER4:
            writeLayerState("Macro Layer");
            if (!caps_on) {
                sethsv(21,255,30, (rgb_led_t *)(&led[0]));
            }
            break;
    }
    layer_name_displayed = true;
  return state;
}

__attribute__((weak)) bool qp_st7735_init(painter_device_t device, painter_rotation_t rotation) {
    // clang-format off
    const uint8_t st7735_init_sequence[] = {
        // Command,                 Delay, N, Data[N]
        ST77XX_CMD_RESET,            120,  0,
        ST77XX_CMD_SLEEP_OFF,          5,  0,
        ST77XX_SET_PIX_FMT,            0,  1, 0x55,
        ST77XX_CMD_INVERT_OFF,         0,  0,
        ST77XX_CMD_NORMAL_ON,          0,  0,
        ST7735_SET_PGAMMA,             0, 16, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
        ST7735_SET_NGAMMA,             0, 16, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
        ST77XX_CMD_DISPLAY_ON,        20,  0
    };
    // clang-format on
    qp_comms_bulk_command_sequence(device, st7735_init_sequence, sizeof(st7735_init_sequence));

    // Configure the rotation (i.e. the ordering and direction of memory writes in GRAM)
    const uint8_t madctl[] = {
        [QP_ROTATION_0]   = ST77XX_MADCTL_RGB,
        [QP_ROTATION_90]  = ST77XX_MADCTL_RGB | ST77XX_MADCTL_MX | ST77XX_MADCTL_MV,
        [QP_ROTATION_180] = ST77XX_MADCTL_RGB | ST77XX_MADCTL_MX | ST77XX_MADCTL_MY,
        [QP_ROTATION_270] = ST77XX_MADCTL_RGB | ST77XX_MADCTL_MV | ST77XX_MADCTL_MY,
    };
    qp_comms_command_databyte(device, ST77XX_SET_MADCTL, madctl[rotation]);

    return true;
}

// when KB starts running, set things up
void keyboard_post_init_user(void) {
    // backlight
    setPinOutput(GP29);
    writePinHigh(GP29);
    // setPinInputHigh(GP20);
    // setPinInputHigh(GP19);
    setPinInputHigh(GP22);
    setPinInputHigh(GP21);
    debug_enable = DEBUG_ENABLED;

    // create display
    display = qp_st7735_make_spi_device(130, 161, GP18, GP16, GP17, 0, 3);
    qp_init(display, QP_ROTATION_180);
    qp_rect(display, 0,0,130, 161, HSV_BLACK, true);

    // load image
    image = qp_load_image_mem(gfx_monaco129);
    if (image != NULL){
        my_anim = qp_animate(display,0,0,image);
        animating = true;
    }

    // load font
    my_font = qp_load_font_mem(font_scp);
    qp_drawtext(display, 2, 138, my_font, textArr);

    sethsv(0,0,0, (rgb_led_t *)&led[0]);
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) { /* First encoder */
        if (!clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return false;
}

// bool process_record_user(uint16_t keycode, keyrecord_t * record) {
//     if (record->event.pressed) {

//     }

//     return true;
// }


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    //  NUMPAD
    [0] = LAYOUT_ortho_5x4(
        KC_HOME,    KC_END,     KC_DELETE,  KC_MUTE,
        KC_KP_7,    KC_KP_8,    KC_KP_9,    KC_KP_MINUS,
        KC_KP_4,    KC_KP_5,    KC_KP_6,    KC_KP_PLUS,
        KC_KP_1,    KC_KP_2,    KC_KP_3,    KC_KP_ASTERISK,
        TO(1),      KC_KP_0,    KC_KP_DOT,  KC_KP_SLASH
    ),
    // MEDIA KEYS
    [1] = LAYOUT_ortho_5x4(
        KC_HOME,    KC_END,     KC_DELETE,  KC_MUTE,
        KC_FIND,                KC_CAPS_LOCK,                KC_WH_U,                KC_AUDIO_MUTE,
        KC_MEDIA_PREV_TRACK,    KC_MPLY,    KC_MEDIA_NEXT_TRACK,    KC_AUDIO_VOL_UP,
        KC_CALC,                KC_MYCM,                KC_WH_D,                KC_AUDIO_VOL_DOWN,
        TO(2),                  KC_CUT,                 KC_COPY,                KC_PASTE
    ),
    // NAVIGATION KEYS
    [2] = LAYOUT_ortho_5x4(
        KC_HOME,    KC_END,     KC_DELETE,  KC_MUTE,
        KC_INSERT,      KC_HOME,    KC_PGUP,        KC_PSCR,
        KC_DELETE,      KC_END,     KC_PGDN,        KC_SCROLL_LOCK,
        KC_COPY,        KC_PASTE,    KC_UP,          KC_PAUSE,
        TO(3),          KC_LEFT,    KC_DOWN,        KC_RIGHT
    ),
    // blank for now
    [3] = LAYOUT_ortho_5x4(
        KC_HOME,    KC_END,     KC_DELETE,  KC_MUTE,
        KC_NO,      KC_NO,      KC_NO,      KC_NO,
        KC_NO,      KC_NO,      KC_NO,      KC_NO,
        KC_NO ,     KC_NO,      KC_NO,      KC_NO,
        TO(0),      KC_NO,      KC_NO,      KC_NO
    )
};
