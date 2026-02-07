#include "LogCapture.hpp"
#include "Logger.hpp"

namespace al
{
    LogCapture::LogCapture(const eLogLevel level,
#if ENABLE_SOURCE_LOGGING
        std::source_location&& location,
#endif
        std::optional<std::shared_ptr<LogStream> const> stream) :
        m_Level(level),
        m_Timestamp(std::chrono::system_clock::now()),
#if ENABLE_SOURCE_LOGGING
        m_Location(location),
#endif
        m_LogStream(stream)
    {}

    LogCapture::~LogCapture()
    {
        m_Stream << '\n';
        Logger::PushMessage(m_Level, std::move(m_Timestamp),
#if ENABLE_SOURCE_LOGGING
            std::move(m_Location),
#endif
     std::move(m_Stream.str()), std::move(m_LogStream));
    }
}