#include "LogFormatter.hpp"
#include <filesystem>
#include <sstream>

#if !defined(USE_FMT)
    #include <format>
    #define FORMAT std::format
#else
    #include <fmt/core.h>
    #define FORMAT fmt::format
#endif

namespace al
{
    const char* LogFormatter::GetLevelStr(const eLogLevel level)
    {
        switch (level)
        {
            case eLogLevel::VERBOSE: return "VERBOSE";
            case eLogLevel::INFO:    return "INFO";
            case eLogLevel::WARNING: return "WARNING";
            case eLogLevel::FATAL:   return "FATAL";
        }
        return "UNKNOWN";
    }

    std::string LogFormatter::Format(const LogMessagePtr& msg, const LogFormatOptions& options)
    {
        if (!msg)
            return "";

        std::stringstream out;

        if (options.showFrameId && msg->FrameId().has_value())
        {
            out << "[" << msg->FormatFrameId(options.frameIdWidth) << "]";
        }

        if (options.showTimestamp)
        {
            const auto timestamp = FORMAT("{0:%H:%M:%S}", msg->Timestamp());
            out << "[" << timestamp << "]";
        }

        if (options.showLevel)
        {
            if (options.colorize)
            {
                const char* colorCode = "\x1b[0m";
                switch (msg->Level())
                {
                    case eLogLevel::VERBOSE: colorCode = "\x1b[34m"; break;
                    case eLogLevel::INFO:    colorCode = "\x1b[32m"; break;
                    case eLogLevel::WARNING: colorCode = "\x1b[33m"; break;
                    case eLogLevel::FATAL:   colorCode = "\x1b[31m"; break;
                }
                out << colorCode << "[" << GetLevelStr(msg->Level()) << "]\x1b[0m";
            }
            else
            {
                out << "[" << GetLevelStr(msg->Level()) << "]";
            }
        }

        if (options.showScope && msg->ScopeName().has_value())
        {
            out << "[" << *msg->ScopeName() << "]";
        }

#if ENABLE_SOURCE_LOGGING
        if (options.showLocation)
        {
            const auto file = std::filesystem::path(msg->Location().file_name()).filename().string();
            out << "[" << file << ":" << msg->Location().line() << "]";
        }
#endif

        out << " " << msg->Message();

        return out.str();
    }

    std::string LogFormatter::FormatConsole(const LogMessagePtr& msg, bool colorize)
    {
        LogFormatOptions opts;
        opts.colorize = colorize;
        return Format(msg, opts);
    }

    std::string LogFormatter::FormatFile(const LogMessagePtr& msg)
    {
        LogFormatOptions opts;
        opts.colorize = false;
        return Format(msg, opts);
    }
}
