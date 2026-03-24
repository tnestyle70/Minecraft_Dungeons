#pragma once

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <windows.h>

// =====================================================================
//  ServerLog.h  —  서버 콘솔 로그 유틸리티
//  레벨: INFO / NET / WARN
//  포맷: [레벨] HH:MM:SS 메시지
//  예:   [NET] 14:32:01 Client connected: id=1
// =====================================================================

// 로그 레벨별 콘솔 색상 코드 (Windows Console)
#define LOG_COLOR_INFO  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE  // 흰색
#define LOG_COLOR_NET   FOREGROUND_GREEN | FOREGROUND_INTENSITY              // 밝은 녹색
#define LOG_COLOR_WARN  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY // 노란색

inline void ServerLog(const char* level, WORD color, const char* fmt, ...)
{
    // 현재 시각
    time_t now = time(nullptr);
    struct tm t;
    localtime_s(&t, &now);
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

    // 메시지 포맷
    char msgBuf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    va_end(args);

    // 색상 적용 후 출력
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    printf("[%-4s] %s %s\n", level, timeBuf, msgBuf);

    // 색상 복원
    SetConsoleTextAttribute(hConsole, LOG_COLOR_INFO);
}

#define LOG_INFO(fmt, ...)  ServerLog("INFO", LOG_COLOR_INFO, fmt, ##__VA_ARGS__)
#define LOG_NET(fmt, ...)   ServerLog("NET",  LOG_COLOR_NET,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ServerLog("WARN", LOG_COLOR_WARN, fmt, ##__VA_ARGS__)
