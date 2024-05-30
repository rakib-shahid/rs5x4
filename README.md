# rs5x4

![rs5x4](imgur.com image replace me!)

* occasionally bugs TF out (restarting host fixes, probably due to awful python code)
* potential for 128x128 and dynamic gif for specific songs
* hid code 0xEE is used by vial so spotify data and vial use cannot be simultaneous, pause spotify then use vial

* Keyboard Maintainer: [rakib-shahid](https://github.com/rakib-shahid)
* Hardware Supported: *The PCBs, controllers supported*
* Hardware Availability: *Links to where you can find this hardware*

Make example for this keyboard (after setting up your build environment):

    make rs5x4:default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (usually the top left key or Escape) and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB - some may have pads you must short instead
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available

# Changelog
* Fixed single pixel of progress bar showing when it shouldn'> [!TIP]
* Made layer indicator text more readable (on screen for longer) 
