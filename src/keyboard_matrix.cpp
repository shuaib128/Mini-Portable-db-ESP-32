// #include "keyboard_matrix.h"

// constexpr uint8_t ROW_COUNT = 2;
// constexpr uint8_t COL_COUNT = 2;

// const uint8_t rowPins[ROW_COUNT] = {
//     2,
//     3};

// const uint8_t colPins[COL_COUNT] = {
//     4,
//     5};

// const char keys[ROW_COUNT][COL_COUNT] = {
//     {'1', '2'},
//     {'3', '4'}};

// bool previousState[ROW_COUNT][COL_COUNT] = {
//     {false, false},
//     {false, false}};

// void setup_keyboard_matrix()
// {
//     for (uint8_t row = 0; row < ROW_COUNT; row++)
//     {
//         pinMode(rowPins[row], OUTPUT);
//         digitalWrite(rowPins[row], HIGH);
//     }

//     for (uint8_t col = 0; col < COL_COUNT; col++)
//     {
//         pinMode(colPins[col], INPUT_PULLUP);
//     }

//     Serial.println("2x2 button matrix ready");
// }

// char keyboard_matrix_loop()
// {
//     char pressedKey = '\0';

//     for (uint8_t row = 0; row < ROW_COUNT; row++)
//     {
//         // Disable every row.
//         for (uint8_t otherRow = 0; otherRow < ROW_COUNT; otherRow++)
//         {
//             digitalWrite(rowPins[otherRow], HIGH);
//         }

//         // Scan current row.
//         digitalWrite(rowPins[row], LOW);
//         delayMicroseconds(10);

//         for (uint8_t col = 0; col < COL_COUNT; col++)
//         {
//             bool isPressed = digitalRead(colPins[col]) == LOW;

//             // Only report a new press.
//             if (isPressed && !previousState[row][col])
//             {
//                 pressedKey = keys[row][col];
//             }

//             previousState[row][col] = isPressed;
//         }

//         digitalWrite(rowPins[row], HIGH);
//     }

//     delay(20); // Basic debounce

//     return pressedKey;
// }