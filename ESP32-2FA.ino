#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TOTP.h>

int pinCS = 2; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
/**
 * ESP8266 Pin
 * D4 -> CS    -> CS   GPIO2
 * D5 -> HSCLK -> CLK  GPIO14
 * D7 -> HMOSI -> DIN  GPIO13
 */
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
#define _DISPLAY_ROTATE 1

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

#ifndef STASSID
#define STASSID "█████████████████"
#define STAPSK  "█████████"
#endif

String tape = "123456";

const char* ssid = STASSID;
const char* password = STAPSK;

// TOTP
uint8_t hmacKey[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "cn.pool.ntp.org");
TOTP totp = TOTP(hmacKey, 10);

// 3x5 char maps
uint16_t numMap3x5[] = {
    32319, // 0
    10209, // 1
    24253, // 2
    22207, // 3
    28831, // 4
    30391, // 5
    32439, // 6
    16927, // 7
    32447, // 8
    30399  // 9
};

#define PIXEL_SHOW HIGH
#define PIXEL_HIDE LOW

void _drawPixel(Max72xxPanel display, uint8_t x, uint8_t y, uint8_t pixel) {
#ifdef _DISPLAY_FLIP
    display.drawPixel(x, 7 - y, pixel);
#else
    display.drawPixel(x, y, pixel);
#endif
}

void _drawRoundRect(Max72xxPanel display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t pixel) {
#ifdef _DISPLAY_FLIP
    display.drawRoundRect(x, 7 - (h + y - 1), w, h, r, pixel);
#else
    display.drawRoundRect(x, y, w, h, r, pixel);
#endif
}

void _drawLine(Max72xxPanel display, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
#ifdef _DISPLAY_FLIP
    display.drawLine(x1, 7 - y1, x2, 7 - y2, pixel);
#else
    display.drawLine(x1, y1, x2, y2, pixel);
#endif
}

void drawLogo(Max72xxPanel display, int eye_offset) {
    display.fillRect(0, 0, 8, 8, PIXEL_HIDE);
    _drawPixel(display, 1, 0, PIXEL_SHOW);
    _drawPixel(display, 2, 1, PIXEL_SHOW);
    _drawPixel(display, 6, 0, PIXEL_SHOW);
    _drawPixel(display, 5, 1, PIXEL_SHOW);
    _drawPixel(display, 2, 4 + eye_offset, PIXEL_SHOW);
    _drawPixel(display, 5, 4 + eye_offset, PIXEL_SHOW);
    _drawRoundRect(display, 0, 2, 8, 6, 1, PIXEL_SHOW);
}

void drawSplashtop(Max72xxPanel display) {
    // b
    _drawLine(display, 10, 1 - 1, 10, 6 - 1, PIXEL_SHOW);
    _drawLine(display, 11, 3 - 1, 12, 3 - 1, PIXEL_SHOW);
    _drawLine(display, 11, 6 - 1, 12, 6 - 1, PIXEL_SHOW);
    _drawLine(display, 13, 4 - 1, 13, 5 - 1, PIXEL_SHOW);

    // i
    _drawLine(display, 15, 1 - 1, 15, 6 - 1, PIXEL_SHOW);
    _drawPixel(display, 15, 2 - 1, PIXEL_HIDE);

    // l
    _drawLine(display, 17, 1 - 1, 17, 6 - 1, PIXEL_SHOW);

    // i
    _drawLine(display, 19, 1 - 1, 19, 6 - 1, PIXEL_SHOW);
    _drawPixel(display, 19, 2 - 1, PIXEL_HIDE);

    // b
    _drawLine(display, 21, 1 - 1, 21, 6 - 1, PIXEL_SHOW);
    _drawLine(display, 22, 3 - 1, 23, 3 - 1, PIXEL_SHOW);
    _drawLine(display, 22, 6 - 1, 23, 6 - 1, PIXEL_SHOW);
    _drawLine(display, 24, 4 - 1, 24, 5 - 1, PIXEL_SHOW);

    // i
    _drawLine(display, 26, 1 - 1, 26, 6 - 1, PIXEL_SHOW);
    _drawPixel(display, 26, 2 - 1, PIXEL_HIDE);

    // l
    _drawLine(display, 28, 1 - 1, 28, 6 - 1, PIXEL_SHOW);

    // i
    _drawLine(display, 30, 1 - 1, 30, 6 - 1, PIXEL_SHOW);
    _drawPixel(display, 30, 2 - 1, PIXEL_HIDE);

    display.write();
}

void drawMapValue3x5(Max72xxPanel display, uint8_t x, uint8_t y, uint32_t val) {
    for (uint8_t i = 0; i < 20; i++) {
        if ((val >> i) & 1 == 1) {
            display.drawPixel(x + (3 - i / 5) - 1, y + (4 - i % 5), HIGH);
        }
    }
}

void setup() {
    SPI.begin();
    SPI.setFrequency(10000000); // Here is 10Mhz
    matrix.setIntensity(2);     // Set brightness between 0 and 15
    for (int i = 0; i < numberOfHorizontalDisplays; i++) {
        matrix.setRotation(i, _DISPLAY_ROTATE);
    }
    matrix.fillScreen(PIXEL_HIDE);
    drawLogo(matrix, 0);
    drawSplashtop(matrix);

    WiFi.mode(WIFI_STA);
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    WiFi.begin(ssid, password);

    Serial.begin(115200);
    Serial.println();
    Serial.printf("Flash: %d\n", ESP.getFlashChipRealSize());
    Serial.print("Connecting");
    int processbar = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
        drawLogo(matrix, processbar % 2);
        matrix.drawPixel(8 + (processbar % 24), 7, (processbar / 24) % 2 == 0 ? HIGH : LOW);
        matrix.write();
        processbar++;
    }
    Serial.println();

    Serial.print("Connected to wifi. My address:");
    IPAddress myAddress = WiFi.localIP();
    Serial.println(myAddress);

    // TOTP
    timeClient.begin();
    timeClient.setTimeOffset(0); // UTC+8 28800

    WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 1000);
    matrix.fillScreen(PIXEL_HIDE);
    drawLogo(matrix, 0);
    drawSplashtop(matrix);
}

void loop() {
    // TOTP
    timeClient.update();
    matrix.fillScreen(LOW);

    drawLogo(matrix, 0);
    int x = 9;
    
    tape = String(totp.getCode(timeClient.getEpochTime()));
    int count = timeClient.getSeconds() % 30;
    uint32_t waitBarLen = count;
    for (uint8_t waitBar = 0; waitBar < waitBarLen; waitBar++) {
        if (waitBar < 15) {
            _drawPixel(matrix, 9 + waitBar, 6, PIXEL_SHOW);
        } else {
            _drawPixel(matrix, 17 + waitBar - 15, 7, PIXEL_SHOW);
        }
    }

    for (int i = 0; i < tape.length(); i++) {
        if (tape[i] >= '0' && tape[i] <= '9') {
            drawMapValue3x5(matrix, x, 0, numMap3x5[tape[i] - '0']);
            x += 4;
        } else if (tape[i] == '.') {
            matrix.drawPixel(x, 4, HIGH);
            x += 2;
        } else if (tape[i] == ' ') {
            x += 1;
        } else {
            matrix.drawChar(x, 0, tape[i], HIGH, LOW, 1);
            x += 5;
        }
    }
    matrix.write(); // Send bitmap to display
    delay(1000);
}
