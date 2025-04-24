#ifndef __BASE_DEBUG_H
#define __BASE_DEBUG_H
// If not on PIO or not defined in platformio.ini
#ifndef APPS_DEBUG
// Debug output set to 0 to disable app debug output
#define APPS_DEBUG 2
#endif
//	 \x1b[38;5;51m
//  base 
#ifndef _C_BASE
#define _C_BASE     "\x1b[0m"
#endif
//  Red
#ifndef _C_RED
#define _C_RED      "\x1b[31m"   //"\x1b[38;5;196m"
#endif
//  Green
#ifndef _C_GREEN
#define _C_GREEN    "\x1b[32m"
#endif
//  Blue
#ifndef _C_BLUE
#define _C_BLUE     "\x1b[34m"// "\x1b[38;5;21m"
#endif
//  yellow
#ifndef _C_YEL
#define _C_YEL      "\x1b[38;5;220m"
#endif
//  Orange
#ifndef _C_ORG
#define _C_ORG      "\x1b[38;5;208m"
#endif
// Magenta
#ifndef _C_MAGENTA
#define _C_MAGENTA  "\x1b[35m"
#endif
//  Cyan
#ifndef _C_CYAN
#define _C_CYAN     "\x1b[38;5;51m"
#endif
// Dark blue
#ifndef _C_DARKBLUE
#define _C_DARKBLUE "\x1b[34m"
#endif
//  Light cyan
#ifndef _C_LCYAN
#define _C_LCYAN    "\x1b[38;5;195m"
#endif
//  Light green
#ifndef _C_LGREEN
#define _C_LGREEN    "\x1b[38;5;82m"
#endif
//  Green blue
#ifndef _C_GREEN_BLUE
#define _C_GREEN_BLUE    "\x1b[36m"
#endif

#if APPS_DEBUG > 0
#define LINT_ERR(tag, ...)           \
    do                                  \
    {                                   \
        if (tag)                        \
            Serial.print("\x1b[38;5;196m";)\
            Serial.print("[INIT-ERROR]");\
            Serial.print("\x1B[35m");\
            Serial.printf(" <%s> \x1B[0m", __func__); \
            Serial.print("\x1B[31m");   \
            Serial.printf(__VA_ARGS__); \
            Serial.printf(_C_BASE); \
    } while (0)
#else
#define LINT_ERR(...)
#endif

#if APPS_DEBUG > 1
#define LINT_DBG(tag, ...)           \
    do                                  \
    {                                   \
        if (tag)                        \
            Serial.print("\x1b[38;5;51m");\
            Serial.print("[INIT-ERROR]");\
            Serial.print("\x1B[35m");\
            Serial.printf(" <%s> \x1B[0m", __func__); \
            Serial.print("\[36m");   \
            Serial.printf(__VA_ARGS__); \
            Serial.printf(_C_BASE); \
    } while (0)
#else
#define LINT_DBG(...)
#endif


 // If not on PIO or not defined in platformio.ini
#ifndef SERV_DEBUG
// Debug output set to 0 to disable app debug output
#define SERV_DEBUG 2
#define GET_FILENAME(filename) (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename)
#endif

#if SERV_DEBUG > 0
#define L_ERR( ...)              \
    do                                  \
    {                                   \
        if (SERV_DEBUG)                        \
        {\
            const char* currentFilename = __FILE__; \
            const char* shortFilename = GET_FILENAME(currentFilename); \
            Serial.print("\x1b[38;5;159m");\
            Serial.printf("[%s]-\x1B[0m", shortFilename); \
            Serial.print(_C_RED);\
            Serial.print("[ERROR]");\
            Serial.print("\x1B[35m");\
            Serial.printf(" <%s> ", __func__); \
            Serial.print("\x1B[36m");   \
            Serial.printf(__VA_ARGS__); \
            Serial.printf("\x1B[0m\n"); \
        }\
    } while (0)
#else
#define S_ERR(...)
#endif

#if SERV_DEBUG > 1
#define L_DBG( ...)              \
    do                                  \
    {                                   \
        if (SERV_DEBUG)                        \
        {\
            const char* currentFilename = __FILE__; \
            const char* shortFilename = GET_FILENAME(currentFilename); \
            Serial.print("\x1b[38;5;159m");\
            Serial.printf("[%s]-\x1B[0m", shortFilename); \
            Serial.print(_C_YEL);\
            Serial.print("[DEBUG]");\
            Serial.print("\x1B[35m");\
            Serial.printf(" <%s> ", __func__); \
            Serial.print("\x1B[36m");   \
            Serial.printf(__VA_ARGS__); \
            Serial.printf("\x1B[0m\n"); \
        }\
    } while (0)
#else
#define S_DBG(...)
#endif

#endif
// Serial.print("\x1b[38;5;51m");