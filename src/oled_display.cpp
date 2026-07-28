#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace
{
constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);
}

void setup_oled_display()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("Starting OLED...");

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("SSD1306 initialization failed!");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("OLED initialized!");

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

void show_startup_message()
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(0, 0);

    display.println("Hello!");
    display.println("ESP32-C3");

    display.display();
}

void show_pressed_key(char key)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Pressed:");

    display.setTextSize(5);
    display.setCursor(48, 24);
    display.println(key);

    display.display();
}

void show_message(const char *message)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(message);

    display.display();
}