#pragma once
#include <source_location>
#include <string>
#include "LogLevel.hpp"
#include "LogStream.hpp"

namespace al
{
    class LogMessage
    {
    public:
        LogMessage(const eLogLevel level, std::chrono::system_clock::time_point&& timestamp,
#if ENABLE_SOURCE_LOGGING
            std::source_location&& location,
#endif
            std::string&& message, std::optional<std::shared_ptr<LogStream> const> stream);
        virtual ~LogMessage() = default;

        const eLogLevel& Level() const;
#if ENABLE_SOURCE_LOGGING
        const std::source_location& Location() const;
#endif
        const std::string& Message() const;
        const std::chrono::system_clock::time_point& Timestamp() const;
        const std::optional<std::shared_ptr<LogStream> const> Stream() const;

    private:
        const eLogLevel m_Level;
        const std::chrono::system_clock::time_point m_Timestamp;
#if ENABLE_SOURCE_LOGGING
        const std::source_location m_Location;
#endif
        const std::string m_Message;
        const std::optional<std::shared_ptr<LogStream> const> m_Stream;

    };
}