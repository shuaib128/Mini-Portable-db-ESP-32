#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

void setup_oled_display();
void show_startup_message();
void show_pressed_key(char key);
void show_message(const char *message);

#endif