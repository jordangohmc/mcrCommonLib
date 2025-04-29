// IndicatorLed.cpp
#include "Indicator/IndicatorLed.h"
// /* --- RGB LED --- */

/*--------------------------------*/
// Freenove_ESP32_WS2812 led_IND = Freenove_ESP32_WS2812(NUM_LEDS, LED_PIN, CHANNEL, TYPE_GRB);
/*--------------------------------*/
// Freenove_ESP32_WS2812 LedIndicator::WS2812;
// bool LedIndicator::paused = false;
// bool LedIndicator::fnDone = true;
// LedIndFn LedIndicator::LedIndFn[MAX_HIS_FN];
// u8_t LedIndicator::fnLast = 0;
// u8_t LedIndicator::outputPin;
// u8_t LedIndicator::run;
// LedType LedIndicator::mode;
// u32_t LedIndicator::setColor;

// u8_t LedIndicator::baseBrightness = 50;
// u32_t LedIndicator::baseColor = CRGB::Black;

LedIndicator LedInd;
SemaphoreHandle_t LedIndicator::ledMutex = nullptr;
/*------------------------------------------------------
    Class function declarations
------------------------------------------------------*/
void LedIndicator::ledTaskLoop() { //Task4
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t freq = 40;// 间隔 50 mSeconds
  while (true) {
    _waitIfHold();
    IndMode mode = IndMode::None;
    u32_t color = 0;

    if (fnLast > 0) {
      mode = fnQueue[fnLast - 1].fn;
      color = fnQueue[fnLast - 1].setColor;
      // 左移队列
      for (u8_t i = 0; i < fnLast; i++) fnQueue[i] = fnQueue[i + 1];
      fnLast--;
    }
    switch (mode) {
    case IndMode::Done:      runDone(color); break;
    case IndMode::Blink:     runBlink(color); break;
    case IndMode::TickBlink: runTickBlink(color); break;
    case IndMode::Warn:      runWarn(color); break;
    case IndMode::Fail:      runFail(); break;
    case IndMode::Error:     runError(); break;
    default: break;
    }
    vTaskDelayUntil(&lastWakeTime, freq);
  }
  taskHandle = nullptr;
  vTaskDelete(NULL);
}
void LedIndicator::ledTaskStatic(void* pvParams) {
  static_cast<LedIndicator*>(pvParams)->ledTaskLoop();
}
/*--- LED CONTROL ---*/
void LedIndicator::_waitIfHold() {
  while (_isHold) {
    if (_holdUntil != 0 && xTaskGetTickCount() >= _holdUntil) {
      _isHold = false;
      break;
    }
    delay(50);
  }
}
void LedIndicator::_ledOff() {
  if (type == LedType::LED2812) {
    _ledOn(0, CRGB::Black);
  }
  else {
    if (outputPin >= 0)
      digitalWrite(outputPin, LOW);
  }
}

void LedIndicator::_ledOn(u8_t brightness, u32_t color) {
  if (type == LedType::LED2812) {
    _waitIfHold();
    // gpio_matrix_in(GPIO_MATRIX_CONST_ONE_INPUT, U2RXD_IN_IDX, false);
    // pinMode(outputPin, OUTPUT);
    WS2812.setBrightness(brightness);
    WS2812.setAllLedsColorData(color);
    WS2812.show();
    // pinMode(outputPin, INPUT);
    // gpio_matrix_in(20, U2RXD_IN_IDX, false);
    // gpio_pad_select_gpio(20);  // 确保引脚配置为 GPIO 模式
    // gpio_set_direction(GPIO_NUM_20, GPIO_MODE_INPUT);  // 设置为输入
  }
  else {
    if (outputPin >= 0)
      digitalWrite(outputPin, HIGH);
  }
}
/*--- Function ---*/
bool LedIndicator::begin(u8_t ledPin, LedType setType, u8_t brightness) {
  type = setType;
  baseBrightness = brightness;
  baseColor = CRGB::DarkGreen;
  if (type == LedType::LED2812) {
    WS2812 = Freenove_ESP32_WS2812(NUM_LEDS, ledPin, CHANNEL, TYPE_GRB);
    WS2812.begin();
    _ledOn(brightness, baseColor);
  }
  else {
    if (ledPin >= 0) {
      outputPin = ledPin;
      pinMode(ledPin, OUTPUT);
      digitalWrite(outputPin, LOW);
    }
  }
  if (!ledMutex) {
    ledMutex = xSemaphoreCreateMutex();
  }
#ifdef APP_CPU_NUM
  xTaskCreatePinnedToCore(
    ledTaskStatic, "LedIndicator", 2048,
    this, 1, &taskHandle, APP_CPU_NUM);
#else
  xTaskCreatePinnedToCore(
    ledTaskStatic, "LedIndicator", 2048,
    this, 1, &taskHandle, PRO_CPU_NUM);
#endif
  return true;
}
void LedIndicator::setBaseColor(u32_t color) {
  baseColor = color;
  _ledOn(baseBrightness, baseColor);
}
void LedIndicator::set(IndMode fn, u32_t color) {
  if (taskHandle == nullptr || fnLast >= MAX_HIS_FN) return;
  fnQueue[fnLast++] = { fn, color };
}
void LedIndicator::pause(TickType_t time) {
  LedInd._isHold = true;
  if (time > 0) {
    LedInd._holdUntil = xTaskGetTickCount() + time;
  }
  else {
    LedInd._holdUntil = 0; // 表示无限期 hold
  }
}
void LedIndicator::resume() {
  LedInd._isHold = false;
  LedInd._holdUntil = 0;
};
/*--- 模式实现 ---*/
void LedIndicator::runTickBlink(u32_t color) {
  for (size_t i = 0; i <= 3; i++) {
    _ledOn(255, color);
    delay(3); //5
    _ledOff();
    delay(2); //10
  }
  _ledOn(baseBrightness, baseColor);
}

void LedIndicator::runBlink(u32_t color) {
  // if (mode == LED2812) {
  for (size_t i = 0; i <= 5; i++) {
    _ledOn(255, color); delay(3);
    _ledOff(); delay(7);
  }
  _ledOn(255, color); delay(50);
  _ledOn(baseBrightness, baseColor);
  // }
  // else {
  //   for (size_t i = 0; i <= 4; i++) {
  //     digitalWrite(outputPin, HIGH);
  //     delay(5);
  //     digitalWrite(outputPin, LOW);
  //     delay(20);
  //   }
  // }
}

void LedIndicator::runDone(u32_t color) {
  // if (mode == LED2812) {
  for (size_t i = 0; i <= 2; i++) {
    _ledOn(255, color); delay(10);
    _ledOff(); delay(90);
  }
  _ledOn(255, color); delay(150);
  _ledOn(baseBrightness, baseColor);
  // }
  // else {
  //   for (size_t i = 0; i <= 2; i++) {
  //     digitalWrite(outputPin, HIGH);
  //     delay(10);
  //     digitalWrite(outputPin, LOW);
  //     delay(90);
  //   }
  // }
}

void LedIndicator::runWarn(u32_t color) {
  if (color == 0) color = CRGB::DarkOrange;
  for (int b = 0; b <= 255; b += 3) {
    _ledOn(b, color); delay(1);
  }
  for (int b = 255; b >= 0; b -= 3) {
    _ledOn(b, color); delay(1);
  }
  _ledOff(); delay(10);
  _ledOn(baseBrightness, baseColor);
}

void LedIndicator::runFail() {
  // Serial.printf("\n--Ind [Fail]\n");
  for (int b = 255; b >= 0; b -= 3) {
    _ledOn(b, CRGB::DarkOrange); delay(1);
  }
  _ledOff(); delay(10);
  _ledOn(baseBrightness, baseColor);
}

void LedIndicator::runError() {
  // if (mode != LED2812) return;
  // Serial.printf("\n--Ind [Error]\n");
  baseColor = CRGB::Red;
  _ledOn(200, CRGB::Red); delay(10);
  _ledOff(); delay(10);
  _ledOn(baseBrightness, baseColor);
}