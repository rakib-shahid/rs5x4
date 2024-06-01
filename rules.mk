

QUANTUM_PAINTER_ENABLE 		= yes
QUANTUM_PAINTER_DRIVERS 	+= st7735_spi
QUANTUM_PAINTER_NUM_IMAGES	= 1
QUANTUM_LIB_SRC += spi_master.c

# gifs
SRC += gifs/monaco129.qgf.c
SRC += gifs/sanacut.qgf.c

# font
SRC += fonts/scp.qff.c

ENCODER_ENABLE = yes
# ENCODER_MAP_ENABLE = yes

# EXTRALDFLAGS = -Wl,--defsym,FLASH_LEN=8192k
# DEFERRED_EXEC_ENABLE = yes
RAW_ENABLE = yes
LTO_ENABLE = yes
# CONSOLE_ENABLE = yes

WS2812_DRIVER = vendor
RGBLIGHT_ENABLE = yes
