#pragma once
#include <memory>
#include <string>
#include "LogLevel.hpp"
#include "LogMessage.hpp"

namespace al
{
    using LogMessagePtr = std::shared_ptr<LogMessage>;

    struct LogFormatOptions
    {
        bool showFrameId = true;
        size_t frameIdWidth = 7;
        bool showTimestamp = true;
        bool showLevel = true;
        bool showScope = true;
#if ENABLE_SOURCE_LOGGING
        bool showLocation = true;
#else
        bool showLocation = false;
#endif
        bool colorize = false;
    };

    class LogFormatter
    {
    public:
        static const char* GetLevelStr(const eLogLevel level);
        static std::string Format(const LogMessagePtr& msg, const LogFormatOptions& options = {});
        static std::string FormatConsole(const LogMessagePtr& msg, bool colorize = true);
        static std::string FormatFile(const LogMessagePtr& msg);
    };
}
