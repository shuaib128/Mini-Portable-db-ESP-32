# ESP32-C3 Super Mini Wiring Guide

This wiring matches the current `main.cpp` setup for:

- 128×64 SSD1306 OLED display
- 6-pin SPI microSD card reader
- 2×2 keyboard matrix

## OLED Display Wiring

| OLED Pin | ESP32-C3 Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 8 |
| SCL | GPIO 9 |

## microSD Card Reader Wiring

| SD Reader Pin | ESP32-C3 Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CS | GPIO 10 |
| SCK / CLK | GPIO 6 |
| MOSI / DI | GPIO 21 |
| MISO / DO | GPIO 7 |

### microSD Pin Name Notes

Some microSD readers use different labels:

- `DI` means MOSI
- `DO` means MISO
- `CLK` means SCK
- `CS` may also be labeled `SS`

## 2×2 Keyboard Matrix Wiring

| Keyboard Connection | ESP32-C3 Pin |
|---|---|
| Row 1 | GPIO 2 |
| Row 2 | GPIO 3 |
| Column 1 | GPIO 4 |
| Column 2 | GPIO 5 |

### Button Layout

```text
             Column 1       Column 2
              GPIO 4         GPIO 5

Row 1 GPIO 2     1              2

Row 2 GPIO 3     3              4
```

## Complete GPIO Map

```text
GPIO 2  -> Keyboard Row 1
GPIO 3  -> Keyboard Row 2
GPIO 4  -> Keyboard Column 1
GPIO 5  -> Keyboard Column 2

GPIO 6  -> SD SCK / CLK
GPIO 7  -> SD MISO / DO

GPIO 8  -> OLED SDA
GPIO 9  -> OLED SCL

GPIO 10 -> SD CS
GPIO 21 -> SD MOSI / DI

3.3V    -> OLED VCC and SD VCC
GND     -> OLED GND and SD GND
```

## Power Notes

The OLED and microSD reader can share the ESP32-C3's `3.3V` and `GND` pins.

A breadboard power rail can make this easier:

```text
ESP32 3.3V -> Breadboard positive rail
ESP32 GND  -> Breadboard ground rail

OLED VCC   -> Positive rail
SD VCC     -> Positive rail

OLED GND   -> Ground rail
SD GND     -> Ground rail
```

Use `3.3V` for the microSD reader unless the specific module clearly states that it requires `5V`.
