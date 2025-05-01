// logger.cpp
#include "Logger.h"
#include <Arduino.h>
#include <time.h>

#ifndef APP_CPU_NUM
#define APP_CPU_NUM     0
#endif

Logger logger;
QueueHandle_t logQueue = nullptr;
log_function_t default_logging_function = Logger::loggerFunction;
// ---------- 日志任务 ----------
void Logger::printLogTask(void* pvParams) {
    Serial.println("Starting Logger Print Task...");
    char logBuf[LOG_MAX_LENGTH];
    while (1) {
        if (logger.loggerRing.hasData()) {
            size_t len = logger.loggerRing.readLine(logBuf, sizeof(logBuf));
            if (len > 0) {
                size_t dataLen = strlen(logBuf);
                if (dataLen > 0) {
                    Serial.println(logBuf);
                }
            }
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(1));  // 没数据时避免空转
        }
    }
}
// ---------- 初始化函数 ----------
Logger::Logger() {
    // 安全默认值
    if (_curHeapKb <= 0 || _curHeapKb > 999) _curHeapKb = 999;
    if (_minHeapKb <= 0 || _minHeapKb > 999) _minHeapKb = 999;
    // 初始化所有频道默认关闭
    _logHeapEn = false;
    for (int i = 0; i < LOG_MAX_CHANNEL; i++) {
        _logEn[i] = false;
    }
    _logEn[0] = true;
}

void Logger::begin(std::initializer_list<u8_t> logEnChannels) {
    if (!loggerRing.begin()) {
        Serial.println("❌ 初始化 LoggerRing 失败");
        return;
    }
    BaseType_t result = xTaskCreatePinnedToCore(
        printLogTask, "LoggerPrintTask", 1024 * 6,
        NULL, 1, NULL, APP_CPU_NUM);
    if (result != pdPASS) {
        Serial.println("❌ 创建 LoggerPrintTask 失败");
    }
    // set_logging_function(loggerFunction);
    if (logEnChannels.size() == 0) {
        enLog(0);
    }
    else {
        for (auto ch : logEnChannels) {
            enLog(ch);
        }
    }
    _ready = true;
    LogInfo(BASE_L_CH, "Logger initialized");
}
void Logger::setEnabled(u8_t ch, bool en) {
    if (ch >= LOG_MAX_CHANNEL) return;
    _logEn[ch] = en;
}
void Logger::enLog(u8_t ch) {
    if (ch >= LOG_MAX_CHANNEL) return;
    _logEn[ch] = true;
}
bool Logger::toggleEnabled(u8_t ch) {
    if (ch >= LOG_MAX_CHANNEL) return false;
    _logEn[ch] = !_logEn[ch];
    return _logEn[ch];
}
bool Logger::toggleLogHeapEn() {
    _logHeapEn = !_logHeapEn;
    return _logHeapEn;
}
void Logger::heapPrint() {
    uint32_t freeHeap = esp_get_free_heap_size();
    _curHeapKb = (freeHeap) / 1024.0;
    if (_minHeapKb > _curHeapKb) _minHeapKb = _curHeapKb;

    const char* curColor = (_curHeapKb < 30) ? _C_RED
        : (_curHeapKb < 50) ? _C_YEL
        : _C_GREEN;
    const char* minColor = (_minHeapKb < 10) ? _C_RED
        : (_minHeapKb < 30) ? _C_YEL
        : _C_GREEN;
    if (!curColor) curColor = "";   // 防崩溃
    if (!minColor) minColor = "";
    Serial.printf("%s[Heap Monitor]%s Current: %s%6.2f%s KB | Min: %s%6.2f%s KB\n",
        _C_MAGENTA, _C_BASE,
        curColor, _curHeapKb, _C_BASE,
        minColor, _minHeapKb, _C_BASE);
}
// extern void loggerFunction(logLevel log_level, u8_t channel, const char* format, ...) {
void Logger::loggerFunction(logLevel log_level, u8_t channel, const char* format, ...) {
    if (!logger.isEnabled(channel)) return;
    // char buf[256];  // 可以根据需要调整大小
    // va_list args;
    // va_start(args, format);
    // vsnprintf(buf, sizeof(buf), format, args);  // 格式化
    // va_end(args);
    // // 打印，带日志级别和通道信息
    // printf("[LOG %d][CH %d] %s\n", log_level, channel, buf);
    //
    va_list args;
    va_start(args, format);
    loggerFunctionVa(log_level, channel, format, args);
    va_end(args);
}

// extern void loggerFunctionVa(logLevel log_level, u8_t channel, char const* format, va_list args) {
void Logger::loggerFunctionVa(logLevel log_level, u8_t channel, char const* format, va_list args) {
    if (!logger.isEnabled(channel)) return;
    // char logMessage[256];
    // vsnprintf(logMessage, sizeof(logMessage), format, args);
    // Serial.printf("begin test message level: %d channel: %d message: %s\n", log_level, channel, logMessage);
    // 获取锁
    time_t now;
    time(&now);
    localtime_r(&now, &logger.loggerTimeinfo);
    u32_t long_ms = millis() - logger.loggerTimingOffset;
    u16_t now_ms = long_ms % 1000;
    // 构造时间+等级前缀（含颜色）
    int offset = 0;
    char prefixBuf[128];
    char prefixHeapBuf[32];
#ifndef LOG_GMT
    // loggerTimeinfo = *localtime(&now);
#else

#endif // DISABLE LOG_GMT

#ifdef LogShow1Day_Enable
    offset += snprintf(prefixBuffer + offset, sizeof(prefixBuffer) - offset,
        "%02d/%02d/%02d ",
        loggerTimeinfo.tm_year + UNIX_EPOCH_START_YEAR,
        loggerTimeinfo.tm_mon + 1,
        loggerTimeinfo.tm_mday);
#endif // LogShow1Day_Enable

#ifdef LOG_TIME
    offset += snprintf(prefixBuf + offset, sizeof(prefixBuf) - offset,
        "%s%02d:%02d%s",
        _C_GREEN_BLUE,
        logger.loggerTimeinfo.tm_min,
        logger.loggerTimeinfo.tm_sec,
        _C_BASE);
#endif // LOG_TIME

#ifdef LOG_MSEC
    offset += snprintf(prefixBuf + offset, sizeof(prefixBuf) - offset,
        " %s%03d%s",
        _C_DARKBLUE,
        now_ms,
        _C_BASE);
#endif // LOG_MSEC

#ifdef LOG_LEVEL
    const char* levelColor =
        (log_level == log_level_debug) ? _C_GREEN :
        (log_level == log_level_info) ? _C_MAGENTA :
        (log_level == log_level_warn) ? _C_YEL :
        _C_RED;
    const char* levelText =
        (log_level == log_level_debug) ? "[DEBUG]" :
        (log_level == log_level_info) ? "[INFO] " :
        (log_level == log_level_warn) ? "[WARN] " :
        "[ERROR]";
    offset += snprintf(prefixBuf + offset, sizeof(prefixBuf) - offset,
        " %s%s%s", levelColor, levelText, _C_BASE);
#endif // LOG_LEVEL
    // 格式化用户提供的信息
    if (logger._logHeapEn) {
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        const char* taskName = pcTaskGetName(currentTask);
        uint32_t freeHeap = esp_get_free_heap_size(); // 获取当前堆内存剩余量（字节数）
        UBaseType_t stackWaterMark = uxTaskGetStackHighWaterMark(currentTask); // 获取当前任务的最小剩余栈空间（以字为单位）
        const char* stackColor =
            (stackWaterMark < 1 * 1024) ? _C_RED :
            (stackWaterMark < 2 * 1024) ? _C_YEL : _C_GREEN;
        offset += snprintf(prefixBuf + offset, sizeof(prefixBuf) - offset,
            " %s%5.1f|%6.3f KB %s%-8.8s%s ",
            stackColor,
            (freeHeap / 1024.0),
            (stackWaterMark / 1024.0),
            _C_RED, taskName, _C_BASE);
        logger._curHeapKb = freeHeap / 1024.0;
        snprintf(prefixHeapBuf, sizeof(prefixHeapBuf),
            "%5.1f|%6.3f KB %-8.8s",
            (freeHeap / 1024.0),
            (stackWaterMark / 1024.0),
            taskName);
        if (logger._minHeapKb > logger._curHeapKb) logger._minHeapKb = logger._curHeapKb;
    }
    else {
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        const char* taskName = pcTaskGetName(currentTask);
        snprintf(prefixHeapBuf, sizeof(prefixHeapBuf), "%-8.8s", taskName);
    }
    int prefixLen = strlen(prefixBuf);
    // 格式化用户日志内容长度
    va_list args_copy;
    va_copy(args_copy, args);
    int userMsgLen = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    const char* truncMsg = "...(truncated)";
    const int truncMsgLen = strlen(truncMsg);
    int totalLen = prefixLen + userMsgLen + 1; // +1 for '\0'

    bool needTrunc = false;
    if (totalLen > LOG_MAX_LENGTH) {
        totalLen = LOG_MAX_LENGTH;
        needTrunc = true;
    }
    // 分配最终的 msg 缓冲区
    char* finalMsg = nullptr;
    if (totalLen > 0) {
        finalMsg = (char*)malloc(totalLen);
        if (!finalMsg) {
            Serial.println("Failed to allocate memory for logger!");
            // 错误处理：例如记录到外部日志，避免继续执行
            if (logger.externLogger)
                logger.externLogger(log_level, prefixHeapBuf, "Failed to allocate memory for logger!", args);
            return; // 直接返回，避免后续崩溃
        }
    }
    // 拷贝 prefix 到 finalMsg
    memcpy(finalMsg, prefixBuf, prefixLen);
    // 格式化用户内容，写到 prefix 后面
    va_list args_copy2;
    va_copy(args_copy2, args);
    vsnprintf(finalMsg + prefixLen, totalLen - prefixLen, format, args_copy2);
    va_end(args_copy2);
    if (needTrunc) {
        int spaceLeft = LOG_MAX_LENGTH - truncMsgLen - 1;
        finalMsg[spaceLeft] = '\0';
        memcpy(finalMsg + spaceLeft, truncMsg, truncMsgLen + 1);
    }
    // Serial.printf("finalMsg2 level: %d channel: %d message: %s\n", log_level, channel, finalMsg);
    // 推入环形缓冲队列
    if (!logger._ready) {
        Serial.printf("%s\n", finalMsg);
    }
    else if (logger.loggerRing.write(finalMsg, strlen(finalMsg))) {
        if (logger.externLogger)
            logger.externLogger(log_level, prefixHeapBuf, format, args);
    }
    else {
        Serial.println("Log pool exhausted!");
        Serial.printf("%s\n", finalMsg);
        if (logger.externLogger)
            logger.externLogger(log_level, prefixHeapBuf, format, args);
    }
    free(finalMsg);
}