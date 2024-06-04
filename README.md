# rs5x4

![rs5x4](/images/rs5x4.jpg)

A major evolution of the [rs5x4](https://github.com/rakib-shahid/rs4x4), now featuring a hot swap PCB, a 128x128 color display, and an optional rotary encoder. 

Capable of displaying live Spotify activity on the screen!!

## **\*Important Note\*** 
* In order to enable Spotify activity on the screen, some setup is required. Follow the guide on the [rs5x4_host repository](https://github.com/rakib-shahid/rs5x4_host) 
* Due to HID code conflict, pause spotify when using Vial (will possibly be fixed later)
* Currently the rotary encoder is not reassignable in Vial, keymap code edit and recompilation is required (again will possibly be fixed later)


## To-do

- [ ] Fix occasional image misalignment. (image_counter not properly incrementing)


## Changelog
* switched to 128x128 from 64x64 
* enabled tap dance

### Vial Make

    make rs5x4:vial