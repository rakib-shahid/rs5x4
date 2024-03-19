#pragma once

#define SPI_DRIVER SPID0
#define SPI_MOSI_PIN GP7
#define SPI_SCK_PIN GP6

#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 0
#define QUANTUM_PAINTER_SUPPORTS_256_PALETTE TRUE
// #define QUANTUM_PAINTER_SUPPORTS_NATIVE_COLORS TRUE
#define QUANTUM_PAINTER_NUM_IMAGES 2
#define QUANTUM_PAINTER_NUM_FONTS 1
#define QUANTUM_PAINTER_PIXDATA_BUFFER_SIZE 32
// #define QUANTUM_PAINTER_TASK_THROTTLE 500
// #define QUANTUM_PAINTER_DEBUG TRUE

// #define ENCODERS_PAD_A { GP20 }
// #define ENCODERS_PAD_B { GP19 }
#define ENCODERS_PAD_A { GP22 }
#define ENCODERS_PAD_B { GP21 }
#define ENCODER_RESOLUTION 4
// #define ENCODER_DEFAULT_POS 0x3