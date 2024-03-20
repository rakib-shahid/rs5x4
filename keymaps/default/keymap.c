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

// static char test[] = "Initial";

// uint8_t framebuffer[SURFACE_REQUIRED_BUFFER_BYTE_SIZE(WIDTH, HEIGHT, 16)] = {0}; // this is where your image data is stored 
// painter_device_t surface = qp_make_rgb565_surface(WIDTH, HEIGHT, (void *)framebuffer);

static painter_device_t display = NULL;
static painter_image_handle_t image = NULL;
static painter_font_handle_t my_font;
static deferred_token my_anim;
static bool animating = false;
// static bool special_anim = false;
static bool album_art = false;
static bool timed_out = false;
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
    qp_rect(display, 0,0,130, 130, HSV_BLACK, true);
}

// void time_out(void){
//     if (){

//     }
// }

uint8_t* doubleArray(uint8_t* originalArray, size_t originalSize){
    uint8_t* newArr = malloc(2*originalSize*sizeof(uint8_t));
    // if (newArr == NULL){
    //     return NULL;
    // }
    for (int i = 0; i < originalSize; i+=2){
        newArr[i*2]     = originalArray[i];
        newArr[i*2+1]   = originalArray[i+1];
        newArr[i*2+2]   = originalArray[i];
        newArr[i*2+3]   = originalArray[i+1];
    }
    return newArr;
}


// hid function
void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t response[length];
    // uprintf("size of data = %d\n",sizeof(*data));
    switch (data[0])
    {
    case 0xFF:
        // uprintf("animating = %i\n",animating);
        // uprintf("album_art = %i\n",album_art);
        // uprintf("timed_out = %i\n",timed_out);
        // uprintf("special_anim = %i\n",special_anim);
        // uprintf("----------------------------\n");
        if (data[1]){
            album_art = true;
            uprintf((char *)(data+2));
            uprintf("\n");
            if (animating){
                animating = false;
                qp_stop_animation(my_anim);
                wipe_image();
            }
            // int compare = strncmp(textArr,(char *)(data+2),strlen(textArr)-1);
            if ((strncmp(songArr,(char *)(data+2),strlen(songArr)-1) != 0)){
                // strcpy(textArr,"\0");
                uprintf("New song string is different\n");
                qp_rect(display, 0,130,131, 162, HSV_BLACK, true);
                qp_drawtext(display, 2, 138, my_font, (char *)(data+2));
                strcpy(songArr,(char *)(data+2));
                // uprintf(textArr);
            }
            // else {
            //     qp_stop_animation(my_anim);
            //     // special_anim = false;
            // }
        }
        else {
            strcpy(songArr,"\0");
            // return;
            album_art = false;
        }



        break;
    case 0xFD:
        uprintf("New image recieved\n");
        uprintf("size of image_data = %d\n", sizeof(image_data));
        image_counter = 0;
        if (animating){
            animating = false;
            qp_stop_animation(my_anim);
        }
        if (timed_out){
            turn_on_screen();
        }
        // wipe_image();
        // qp_circle(display, 32, 64, 5, 255, 0, 255, true);
        // qp_circle(display, 64, 64, 5, 255, 0, 255, true);
        // qp_circle(display, 96, 64, 5, 255, 0, 255, true);
        memcpy(image_data+image_counter,data+1,30);
        // for (int i = 0; i < 30; i++) {
        //     uprintf("%d %d\n", image_data[i],data[i+2]);
        // }
        image_counter += 30;
        break;
    case 0xFE:
        memcpy(image_data+image_counter,data+1,30);
        image_counter += 30;
        break;
    case 0xFC:

        // for (int i = 0; i < sizeof(image_data)/sizeof(image_data[0]); i++) {
        //     printf("%d ", image_data[i]);
        //     if (i % 64 == 0){
        //         printf("\n");
        //     }
        // }
        memcpy(image_data+image_counter,data+1,2);
        // image_counter += 2;
        // for (int i = 0; i < 8192; i++){
        //     uprintf("%d ", image_data[i]);
        // }
        // qp_viewport(display, 0, 0, 64, 64);
        // qp_pixdata(display, image_data, 4096);
        // wipe_image();
        for (int i = 0; i < 64; i++){
            // l, t, r, b
            qp_viewport(display, i, 0, i, 64);
            qp_pixdata(display, image_data+i*128, 64);
        }
        // for (int i = 0; i < 8192; i++){
        //     uprintf("%d ",image_data[i]);
        //     if (i != 0 && i % 30 == 0){
        //         uprintf("\n");
        //     }
        // }
        
        
        break;
    case 0xFB:
        uprintf("C++ Host data received\n");
        uprintf("Data = %s\n",(char *)(data+1));
    }
    raw_hid_send(response, length);
}

// bool timeout_task(bool &album_art, bool &timed_out, bool &animating, int activity){
//     if 
// }

void housekeeping_task_user(void){
    int last_activity = last_input_activity_elapsed();
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

    
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) { /* First encoder */
        if (clockwise) {
            tap_code(KC_A);
        } else {
            tap_code(KC_B);
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

    [0] = LAYOUT_ortho_1x2(
        KC_W, KC_L
    )
};
