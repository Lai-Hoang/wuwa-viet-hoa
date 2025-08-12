#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>

class Logger {
public:
    enum class LogLevel {
        Info,
        Success,
        Error,
        Warning
    };

    struct LogColors {
        WORD labelColor = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        WORD infoColor = FOREGROUND_GREEN | FOREGROUND_BLUE;
        WORD successColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        WORD errorColor = FOREGROUND_RED;
        WORD warningColor = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    };

    static void Init(const std::string& labelText, const LogColors& colors = LogColors()) {
        if (!hConsole_) {
            hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole_ == nullptr || hConsole_ == INVALID_HANDLE_VALUE) {
                if (!AllocConsole()) {
                    throw std::runtime_error("Failed to allocate console.");
                }
                // Gán stream stdout vào console debug
                /*FILE* fp;
                freopen_s(&fp, "CONOUT$", "w", stdout);*/

                // Lấy handle của console
                //HWND hConsole = GetConsoleWindow();
                //if (hConsole) {
                //    // Luôn luôn on top
                //    SetWindowPos(hConsole, HWND_TOPMOST, 0, 0, 0, 0,
                //        SWP_NOMOVE | SWP_NOSIZE);
                //}

                // off on top
                /*SetWindowPos(hConsole, HWND_NOTOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE);*/
                hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
            }
        }

        SetConsoleOutputCP(CP_UTF8);

        labelText_ = labelText;
        colors_ = colors;

        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole_, &consoleInfo);
        originalColor_ = consoleInfo.wAttributes;
    }

    static void popupNguSi()
    {
        HWND hConsole = GetConsoleWindow();
        if (hConsole) {
            // Luôn luôn on top
            SetWindowPos(hConsole, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);
        }

        // off on top
        SetWindowPos(hConsole, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
    }

    static void Log(LogLevel level, const char* format, ...) {
        if (!hConsole_) return;
        popupNguSi(); // nhảy popup khi có log :)

        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        std::string message = "[";
        message += labelText_ + ":" + LogLevelToString(level) + "] " + buffer;

        WriteColoredText(FOREGROUND_WHITE | FOREGROUND_INTENSITY);
        WriteConsoleText("[");
        WriteColoredText(colors_.labelColor);
        WriteConsoleText(labelText_);
        WriteColoredText(FOREGROUND_WHITE | FOREGROUND_INTENSITY);
        WriteConsoleText(":");
        WriteColoredText(GetColorForLogLevel(level));
        WriteConsoleText(LogLevelToString(level));
        WriteColoredText(FOREGROUND_WHITE | FOREGROUND_INTENSITY);
        WriteConsoleText("] ");

        WriteColoredText(GetColorForLogLevel(level));
        WriteConsoleText(buffer);

        WriteColoredText(originalColor_);
        WriteConsoleText("\n");
    }

private:
    inline static HANDLE hConsole_ = nullptr;
    inline static std::string labelText_ = "";
    inline static LogColors colors_ = { 0, 0, 0, 0, 0 };
    inline static WORD originalColor_ = 0;
    inline static const WORD FOREGROUND_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    static void WriteColoredText(WORD color) {
        SetConsoleTextAttribute(hConsole_, color);
    }

    static void WriteConsoleText(const std::string& text) {
        std::wstring wideText(text.begin(), text.end());
        DWORD written;
        WriteConsoleW(hConsole_, wideText.c_str(), static_cast<DWORD>(wideText.length()), &written, nullptr);
    }

    static std::string LogLevelToString(LogLevel level) {
        switch (level) {
        case LogLevel::Info: return "Info";
        case LogLevel::Success: return "Success";
        case LogLevel::Error: return "Error";
        case LogLevel::Warning: return "Warning";
        default: return "Unknown";
        }
    }

    static WORD GetColorForLogLevel(LogLevel level) {
        switch (level) {
        case LogLevel::Info: return colors_.infoColor;
        case LogLevel::Success: return colors_.successColor;
        case LogLevel::Error: return colors_.errorColor;
        case LogLevel::Warning: return colors_.warningColor;
        default: return originalColor_;
        }
    }
};

#define LOG_INFO(format, ...) Logger::Log(Logger::LogLevel::Info, format, ##__VA_ARGS__)
#define LOG_SUCCESS(format, ...) Logger::Log(Logger::LogLevel::Success, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Logger::Log(Logger::LogLevel::Error, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) Logger::Log(Logger::LogLevel::Warning, format, ##__VA_ARGS__)

#endif // LOGGER_H