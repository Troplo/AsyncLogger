#include "LogMessage.hpp"
#include "LogLevel.hpp"
#include <chrono>
#if !defined(USE_FMT)
    #include <format>
    #define FORMAT std::format
#else
    #include <fmt/core.h>
    #define FORMAT fmt::format
#endif

namespace al
{
    LogMessage::LogMessage(const eLogLevel level,
                           std::chrono::system_clock::time_point&& timestamp,
#if ENABLE_SOURCE_LOGGING
                           std::source_location&& location,
#endif
                           std::string&& message,
                           std::optional<std::shared_ptr<LogStream>> stream,
                           std::optional<uint64_t> frameId) :
        m_Level(level),
        m_Timestamp(timestamp),
#if ENABLE_SOURCE_LOGGING
        m_Location(location),
#endif
        m_Message(std::move(message)),
        m_Stream(stream),
        m_FrameId(frameId)
    {
    }

    const eLogLevel& LogMessage::Level() const
    {
        return m_Level;
    }

#if ENABLE_SOURCE_LOGGING
    const std::source_location& LogMessage::Location() const
    {
        return m_Location;
    }
#endif

    const std::string& LogMessage::Message() const
    {
        return m_Message;
    }

    const std::chrono::system_clock::time_point& LogMessage::Timestamp() const
    {
        return m_Timestamp;
    }

    const std::optional<std::shared_ptr<LogStream>>& LogMessage::Stream() const
    {
        return m_Stream;
    }

    const std::optional<uint64_t>& LogMessage::FrameId() const
    {
        return m_FrameId;
    }

    std::string LogMessage::FormatFrameId(size_t width) const
    {
        if (!m_FrameId.has_value())
            return "";
        return FORMAT("{:0{}d}", *m_FrameId, width);
    }

    std::optional<std::string> LogMessage::ScopeName() const
    {
        if (m_Stream.has_value() && *m_Stream)
        {
            return (*m_Stream)->Name();
        }
        return std::nullopt;
    }
}