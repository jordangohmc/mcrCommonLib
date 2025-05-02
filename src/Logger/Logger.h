// Logger.h
#pragma once
#include <stdarg.h>
#include <time.h>
#include "base-debug-log.h"
#include "LogRingBuffer.h"

#define UNIX_EPOCH_START_YEAR 1900
/* --- 参数 --- */
#define LOG_BUFFER_SIZE       1024
#define LOG_QUEUE_LENGTH      10
#define LOG_MAX_LENGTH        1024 * 4
#define LOG_MAX_CHANNEL       16
#define BASE_L_CH             0
/* --- Define LOGGER 配置 --- */
//#define LOG_GMT // Setting LOG_GMT or LocalTime
/* ---  --- */
// #define LOG_DAILY
#define LOG_LEVEL
#define LOG_TIME
#define LOG_MSEC
// #define DISABLE_LOGGING
/* --- ---------------- --- */

// 日志控制开关
/* --- Logging --- */
#ifndef DISABLE_LOGGING

typedef enum log_level_t_enum {
  log_level_debug,
  log_level_info,
  log_level_warn,
  log_level_error
}logLevel;

class Logger {
public:
  Logger(); // 构造函数

  void begin(std::initializer_list<u8_t> logEnChannels = {});
  void enLog(u8_t ch);
  void setEnabled(u8_t ch, bool en); // 设置日志通道开关
  bool toggleLogEn(u8_t ch);
  bool isEnabled(u8_t ch) { return _logEn[ch]; }; // 获取日志通道开关
  void enLogHeapEn() { _logHeapEn = true; };
  bool toggleLogDebugEn();
  bool toggleLogHeapEn();
  bool isLogHeapEn() { return _logHeapEn; };
  void heapPrint();
  // 切换输出方法的回调
  void setExtendLogging(void (*externLogger)(logLevel, const char*, const char*, va_list)) {
    this->externLogger = externLogger;
  }
  static void loggerFunction(logLevel log_level, u8_t channel, const char* format, ...);

  bool _ready = false;
private:
  LogRingBuffer loggerRing;
  tm loggerTimeinfo;
  u_int16_t loggerTimingOffset = 0;

  bool _logEn[LOG_MAX_CHANNEL];
  bool _debugEn;
  bool _logHeapEn;
  float _curHeapKb;
  float _minHeapKb;
  static void printLogTask(void* param);
  static void loggerFunctionVa(logLevel log_level, u8_t channel, char const* format, va_list args);
  void (*externLogger)(logLevel, const char*, const char*, va_list);
};
extern Logger logger;

// 日志级别
typedef void (*log_function_t)(logLevel log_level, u8_t channel, const char* format, ...);
extern log_function_t default_logging_function;

// #define set_logging_function(custom_logging_function) \
//   default_logging_function = custom_logging_function;

#define Log(level, channel, message, ...) default_logging_function(level, channel, message, ##__VA_ARGS__)

#define LogDebug(channel, message, ...) do { Log(log_level_debug, channel, message, ##__VA_ARGS__); } while(0)
#define LogInfo(channel, message, ...) do { Log(log_level_info, channel, message, ##__VA_ARGS__); } while(0)
#define LogWarn(channel, message, ...) do { Log(log_level_warn, channel, message, ##__VA_ARGS__); } while(0)
#define LogError(channel, message, ...) do { Log(log_level_error, channel, message, ##__VA_ARGS__); } while(0)

#else
#define set_logging_function(custom_logging_function)
#define Log(level, message, ...)
#define LogInfo0(message, ...)
#define LogError(message, ...)

#endif // DISABLE_LOGGING


