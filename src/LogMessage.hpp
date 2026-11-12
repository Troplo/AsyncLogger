#pragma once
#include <chrono>
#include <optional>
#include <source_location>
#include <string>
#include "LogLevel.hpp"
#include "LogStream.hpp"

namespace al
{
    class LogMessage
    {
    public:
        LogMessage(const eLogLevel level,
                   std::chrono::system_clock::time_point&& timestamp,
#if ENABLE_SOURCE_LOGGING
                   std::source_location&& location,
#endif
                   std::string&& message,
                   std::optional<std::shared_ptr<LogStream>> stream = std::nullopt,
                   std::optional<uint64_t> frameId = std::nullopt);
        virtual ~LogMessage() = default;

        const eLogLevel& Level() const;
#if ENABLE_SOURCE_LOGGING
        const std::source_location& Location() const;
#endif
        const std::string& Message() const;
        const std::chrono::system_clock::time_point& Timestamp() const;
        const std::optional<std::shared_ptr<LogStream>>& Stream() const;
        const std::optional<uint64_t>& FrameId() const;

        std::string FormatFrameId(size_t width = 7) const;
        std::optional<std::string> ScopeName() const;

    private:
        const eLogLevel m_Level;
        const std::chrono::system_clock::time_point m_Timestamp;
#if ENABLE_SOURCE_LOGGING
        const std::source_location m_Location;
#endif
        const std::string m_Message;
        const std::optional<std::shared_ptr<LogStream>> m_Stream;
        const std::optional<uint64_t> m_FrameId;
    };
}