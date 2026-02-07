#include "LogMessage.hpp"
#include "LogLevel.hpp"
#include <chrono>

namespace al
{
    LogMessage::LogMessage(const eLogLevel level, std::chrono::system_clock::time_point&& timestamp,
#if ENABLE_SOURCE_LOGGING
        std::source_location&& location,
#endif
        std::string&& message, std::optional<std::shared_ptr<LogStream> const> stream) :
        m_Level(level),
        m_Timestamp(timestamp),
#if ENABLE_SOURCE_LOGGING
        m_Location(location),
#endif
        m_Message(message),
        m_Stream(stream)
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

    const std::chrono::system_clock::time_point &LogMessage::Timestamp() const
    {
        return m_Timestamp;
    }

    const std::optional<std::shared_ptr<LogStream> const> LogMessage::Stream() const
    {
        return m_Stream;
    }
}