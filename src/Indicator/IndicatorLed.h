// IndicatorLed.h
#pragma once

#include <Freenove_WS2812_Lib_for_ESP32.h>
#include "crgb-color.h"

#define NUM_LEDS    1
#define CHANNEL		0
#define MAX_HIS_FN	20

enum LedType {
    SingleLED,
    LED2812
};

enum IndMode {
    None,
    Done,
    Blink,
    TickBlink,
    Warn,
    Fail,
    Error
};

struct LedIndFn {
    IndMode fn;        // 发出者 ID
    u32_t setColor;
};

class LedIndicator {
public:
    bool begin(u8_t ledPin, LedType setType, u8_t brightness = 50);
    void setBaseColor(u32_t color);
    void set(IndMode fn, u32_t color);
    static void pause(TickType_t time = portMAX_DELAY);   // <-- 新增
    static void resume();

private:
    Freenove_ESP32_WS2812 WS2812 = Freenove_ESP32_WS2812(NUM_LEDS, 0, CHANNEL, TYPE_GRB);
    LedIndFn fnQueue[MAX_HIS_FN];
    u8_t fnLast = 0;
    u8_t outputPin = -1;
    LedType type = LedType::SingleLED;
    u8_t baseBrightness = 50;
    u32_t baseColor = CRGB::Black;
    bool _isHold = false;
    TickType_t _holdUntil = 0;

    TaskHandle_t taskHandle = nullptr;
    static SemaphoreHandle_t ledMutex;

    void ledTaskLoop();
    static void ledTaskStatic(void* arg);
    void _waitIfHold();
    void _ledOn(u8_t brightness, u32_t color);
    void _ledOff();

    void runDone(u32_t color);
    void runBlink(u32_t color);
    void runTickBlink(u32_t color);
    void runWarn(u32_t setColor);
    void runFail();
    void runError();
};
extern LedIndicator LedInd;