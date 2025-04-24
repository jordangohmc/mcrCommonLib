// LogRingBuffer.cpp
#include "Logger/LogRingBuffer.h"

bool LogRingBuffer::begin(size_t size) {
    end(); // 清理旧的
    RING_BUFFER_SIZE = size;
    buffer = (char*)malloc(size);
    if (!buffer) return false;
    bufferSize = size;
    head = tail = 0;
    return true;
}

void LogRingBuffer::end() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
        bufferSize = 0;
    }
}

bool LogRingBuffer::write(const char* str, size_t len) {
    const size_t endLen = strlen(END_MARKER);
    portENTER_CRITICAL(&mux);
    for (size_t i = 0; i < len; i++) {
        size_t next = (head + 1) % bufferSize;
        if (next == tail) {
            portEXIT_CRITICAL(&mux);
            return false;
        }
        buffer[head] = str[i];
        head = next;
    }
    for (size_t i = 0; i < endLen; i++) {
        size_t next = (head + 1) % bufferSize;
        if (next == tail) {
            portEXIT_CRITICAL(&mux);
            return false;
        }
        buffer[head] = END_MARKER[i];
        head = next;
    }
    portEXIT_CRITICAL(&mux);
    // Serial.printf("LogRingBuffer write head: %d tail: %d message: %s\n", head, tail, str);
    return true;
}


size_t LogRingBuffer::readLine(char* dest, size_t maxLen) {
    size_t count = 0;
    const size_t endLen = strlen(END_MARKER);
    char temp[endLen] = { 0 };
    size_t matchIndex = 0;
    portENTER_CRITICAL(&mux);
    while (tail != head && count < maxLen - 1) {
        char ch = buffer[tail];
        tail = (tail + 1) % RING_BUFFER_SIZE;
        // 保存到目标
        dest[count++] = ch;
        // 匹配 END_MARKER
        if (ch == END_MARKER[matchIndex]) {
            matchIndex++;
            if (matchIndex == endLen) {
                // 匹配成功, 移除 <END> 标记
                count -= endLen;
                dest[count] = '\0';
                portEXIT_CRITICAL(&mux);
                return count;
            }
        }
        else {
            matchIndex = 0; // reset if mismatch
        }
    }
    portEXIT_CRITICAL(&mux);
    dest[count] = '\0';
    return 0; // 未找到完整一条
}

bool LogRingBuffer::hasData() {
    // Serial.printf("hasData head: %d tail: %d\n", head, tail);
    return head != tail;
}
