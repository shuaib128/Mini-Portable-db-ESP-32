#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
    ESP32-C3 Super Mini + 128x64 SSD1306 OLED
    + 2x2 keyboard matrix + 6-pin SPI microSD reader

    BUTTON ACTIONS
    --------------
    1 = Create/overwrite /notes.txt
    2 = Append a line to /notes.txt
    3 = Read /notes.txt and display it on OLED
    4 = Show microSD card information

    MICROSD MODULE WIRING
    ---------------------
    SD VCC  -> 3.3V
    SD GND  -> GND
    SD CS   -> GPIO 10
    SD SCK  -> GPIO 6
    SD MOSI -> GPIO 21
    SD MISO -> GPIO 7

    OLED WIRING
    -----------
    OLED VCC -> 3.3V
    OLED GND -> GND
    OLED SDA -> GPIO 8
    OLED SCL -> GPIO 9

    KEYBOARD MATRIX
    ---------------
    Rows    -> GPIO 2, GPIO 3
    Columns -> GPIO 4, GPIO 5
*/

// ============================================================
// OLED CONFIGURATION
// ============================================================

constexpr uint8_t OLED_SDA_PIN = 8;
constexpr uint8_t OLED_SCL_PIN = 9;
constexpr uint8_t OLED_ADDRESS = 0x3C;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1);

// ============================================================
// MICROSD CONFIGURATION
// ============================================================

constexpr uint8_t SD_CS_PIN = 10;
constexpr uint8_t SD_SCK_PIN = 6;
constexpr uint8_t SD_MOSI_PIN = 21;
constexpr uint8_t SD_MISO_PIN = 7;

SPIClass sdSPI(FSPI);

const char *FILE_PATH = "/notes.txt";

// ============================================================
// KEYBOARD MATRIX CONFIGURATION
// ============================================================

constexpr uint8_t ROW_COUNT = 2;
constexpr uint8_t COL_COUNT = 2;

const uint8_t rowPins[ROW_COUNT] = {
    2,
    3};

const uint8_t colPins[COL_COUNT] = {
    4,
    5};

const char keys[ROW_COUNT][COL_COUNT] = {
    {'1', '2'},
    {'3', '4'}};

bool previousState[ROW_COUNT][COL_COUNT] = {
    {false, false},
    {false, false}};

// ============================================================
// GLOBAL STATE
// ============================================================

bool sdReady = false;
unsigned long appendCounter = 0;

// ============================================================
// OLED FUNCTIONS
// ============================================================

void showMessage(
    const String &line1,
    const String &line2 = "",
    const String &line3 = "",
    const String &line4 = "")
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println(line1);

    if (line2.length() > 0)
    {
        display.println(line2);
    }

    if (line3.length() > 0)
    {
        display.println(line3);
    }

    if (line4.length() > 0)
    {
        display.println(line4);
    }

    display.display();
}

void showMenu()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("ESP32 SD Terminal");
    display.println("----------------");
    display.println("1: Write file");
    display.println("2: Append file");
    display.println("3: Read file");
    display.println("4: SD info");

    display.display();
}

void setupOLED()
{
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("OLED initialization failed.");

        while (true)
        {
            delay(1000);
        }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();

    Serial.println("OLED initialized.");
}

// ============================================================
// KEYBOARD FUNCTIONS
// ============================================================

void setupKeyboard()
{
    for (uint8_t row = 0; row < ROW_COUNT; row++)
    {
        pinMode(rowPins[row], OUTPUT);
        digitalWrite(rowPins[row], HIGH);
    }

    for (uint8_t col = 0; col < COL_COUNT; col++)
    {
        pinMode(colPins[col], INPUT_PULLUP);
    }

    Serial.println("2x2 keyboard matrix initialized.");
}

char readKeyboard()
{
    char pressedKey = '\0';

    for (uint8_t row = 0; row < ROW_COUNT; row++)
    {
        // Disable all rows before scanning one row.
        for (uint8_t otherRow = 0; otherRow < ROW_COUNT; otherRow++)
        {
            digitalWrite(rowPins[otherRow], HIGH);
        }

        // Enable the current row.
        digitalWrite(rowPins[row], LOW);
        delayMicroseconds(10);

        for (uint8_t col = 0; col < COL_COUNT; col++)
        {
            bool isPressed = digitalRead(colPins[col]) == LOW;

            // Report only a new button press.
            if (isPressed && !previousState[row][col])
            {
                pressedKey = keys[row][col];
            }

            previousState[row][col] = isPressed;
        }

        digitalWrite(rowPins[row], HIGH);
    }

    delay(20);

    return pressedKey;
}

// ============================================================
// MICROSD FUNCTIONS
// ============================================================

bool setupSDCard()
{
    showMessage("Starting SD card...");

    sdSPI.begin(
        SD_SCK_PIN,
        SD_MISO_PIN,
        SD_MOSI_PIN,
        SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, sdSPI))
    {
        Serial.println("microSD initialization failed.");
        showMessage(
            "SD initialization",
            "failed.",
            "Check wiring/card.");

        return false;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No microSD card detected.");
        showMessage(
            "No SD card found.",
            "Insert a card and",
            "restart ESP32.");

        return false;
    }

    Serial.println("microSD initialized.");
    showMessage("SD card ready.");
    delay(1200);

    return true;
}

bool writeFile(const char *path, const String &text)
{
    if (!sdReady)
    {
        showMessage("SD card not ready.");
        return false;
    }

    File file = SD.open(path, FILE_WRITE);

    if (!file)
    {
        Serial.println("Could not open file for writing.");
        showMessage(
            "Write failed.",
            "Could not open file.");

        return false;
    }

    // FILE_WRITE appends by default, so remove the old file first
    // when an overwrite is required.
    file.close();
    SD.remove(path);

    file = SD.open(path, FILE_WRITE);

    if (!file)
    {
        Serial.println("Could not recreate file.");
        showMessage("Could not create file.");
        return false;
    }

    bool success = file.print(text);
    file.close();

    if (!success)
    {
        Serial.println("Writing data failed.");
        showMessage("Writing failed.");
        return false;
    }

    Serial.println("File written successfully.");
    showMessage(
        "File written:",
        path,
        text);

    return true;
}

bool appendFile(const char *path, const String &text)
{
    if (!sdReady)
    {
        showMessage("SD card not ready.");
        return false;
    }

    File file = SD.open(path, FILE_APPEND);

    if (!file)
    {
        Serial.println("Could not open file for appending.");
        showMessage(
            "Append failed.",
            "Could not open file.");

        return false;
    }

    bool success = file.println(text);
    file.close();

    if (!success)
    {
        Serial.println("Appending data failed.");
        showMessage("Append failed.");
        return false;
    }

    Serial.print("Appended: ");
    Serial.println(text);

    showMessage(
        "Appended:",
        text);

    return true;
}

void readFileAndDisplay(const char *path)
{
    if (!sdReady)
    {
        showMessage("SD card not ready.");
        return;
    }

    File file = SD.open(path, FILE_READ);

    if (!file)
    {
        Serial.println("File does not exist.");
        showMessage(
            "File not found:",
            path,
            "Press 1 first.");

        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("Contents:");
    display.println("--------");

    Serial.println();
    Serial.println("File contents:");
    Serial.println("--------------");

    int displayedCharacters = 0;
    constexpr int MAX_DISPLAY_CHARACTERS = 105;

    while (file.available())
    {
        char character = static_cast<char>(file.read());

        Serial.print(character);

        if (displayedCharacters < MAX_DISPLAY_CHARACTERS)
        {
            display.print(character);
            displayedCharacters++;
        }
    }

    file.close();

    display.display();

    Serial.println();
    Serial.println("--------------");
}

String getCardTypeName(uint8_t cardType)
{
    switch (cardType)
    {
    case CARD_MMC:
        return "MMC";

    case CARD_SD:
        return "SDSC";

    case CARD_SDHC:
        return "SDHC/SDXC";

    default:
        return "Unknown";
    }
}

void showSDInfo()
{
    if (!sdReady)
    {
        showMessage("SD card not ready.");
        return;
    }

    uint8_t cardType = SD.cardType();

    uint64_t cardSizeMB = SD.cardSize() / (1024ULL * 1024ULL);
    uint64_t totalSizeMB = SD.totalBytes() / (1024ULL * 1024ULL);
    uint64_t usedSizeMB = SD.usedBytes() / (1024ULL * 1024ULL);

    String cardTypeName = getCardTypeName(cardType);

    Serial.println();
    Serial.println("microSD information:");
    Serial.print("Type: ");
    Serial.println(cardTypeName);
    Serial.print("Card size: ");
    Serial.print(cardSizeMB);
    Serial.println(" MB");
    Serial.print("Used: ");
    Serial.print(usedSizeMB);
    Serial.println(" MB");
    Serial.print("Filesystem total: ");
    Serial.print(totalSizeMB);
    Serial.println(" MB");

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("microSD Information");
    display.println("-------------------");
    display.print("Type: ");
    display.println(cardTypeName);
    display.print("Size: ");
    display.print(cardSizeMB);
    display.println(" MB");
    display.print("Used: ");
    display.print(usedSizeMB);
    display.println(" MB");

    display.display();
}

// ============================================================
// ARDUINO SETUP AND LOOP
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    setupKeyboard();
    setupOLED();

    showMessage(
        "ESP32 SD Terminal",
        "Starting...");

    delay(1000);

    sdReady = setupSDCard();

    delay(1200);
    showMenu();
}

void loop()
{
    char key = readKeyboard();

    if (key == '\0')
    {
        return;
    }

    Serial.print("Button pressed: ");
    Serial.println(key);

    switch (key)
    {
    case '1':
    {
        String initialText =
            "Hello from ESP32-C3!\n"
            "The microSD card works.\n";

        writeFile(FILE_PATH, initialText);
        break;
    }

    case '2':
    {
        appendCounter++;

        String newLine =
            "Added line #" +
            String(appendCounter) +
            " at " +
            String(millis()) +
            " ms";

        appendFile(FILE_PATH, newLine);
        break;
    }

    case '3':
        readFileAndDisplay(FILE_PATH);
        break;

    case '4':
        showSDInfo();
        break;

    default:
        break;
    }

    // Keep the result visible for three seconds,
    // then return to the menu.
    delay(3000);
    showMenu();
}