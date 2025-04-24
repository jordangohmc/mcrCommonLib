// LogRingBuffer.h
#pragma once

#include <Arduino.h>

#define END_MARKER              "<END>"

class LogRingBuffer {
public:
    LogRingBuffer() = default;
    ~LogRingBuffer() { end(); }

    bool begin(size_t size = 1024 * 4);
    void end();
    bool write(const char* str, size_t len);
    size_t readLine(char* dest, size_t maxLen); // 读取不超过 maxLen 的数据块
    bool hasData();

private:
    size_t RING_BUFFER_SIZE = 0;
    char* buffer = nullptr;
    size_t bufferSize = 0;
    volatile size_t head = 0;
    volatile size_t tail = 0;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
