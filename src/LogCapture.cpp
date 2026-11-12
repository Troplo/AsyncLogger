#include "LogCapture.hpp"
#include "Logger.hpp"

namespace al
{
    LogCapture::LogCapture(const eLogLevel level,
#if ENABLE_SOURCE_LOGGING
        std::source_location&& location,
#endif
        std::optional<std::shared_ptr<LogStream>> stream) :
        m_Level(level),
        m_Timestamp(std::chrono::system_clock::now()),
#if ENABLE_SOURCE_LOGGING
        m_Location(location),
#endif
        m_LogStream(stream),
        m_FrameId(Logger::GetCurrentFrameId()),
        m_Enabled(true)
    {
        if (!m_LogStream.has_value() || !*m_LogStream)
        {
            auto currentScope = Logger::GetCurrentScope();
            if (currentScope)
            {
                m_LogStream = currentScope;
            }
#if ENABLE_SOURCE_LOGGING
            else
            {
                auto pathScope = Logger::ResolveScopeFromPath(m_Location.file_name());
                if (pathScope)
                {
                    m_LogStream = pathScope;
                }
            }
#endif
        }

        if (m_LogStream.has_value() && *m_LogStream)
        {
            m_Enabled = (*m_LogStream)->Enabled();
        }
    }

    LogCapture::LogCapture(LogCapture&& other) noexcept :
        m_Level(other.m_Level),
        m_Timestamp(other.m_Timestamp),
#if ENABLE_SOURCE_LOGGING
        m_Location(std::move(other.m_Location)),
#endif
        m_Stream(std::move(other.m_Stream)),
        m_LogStream(std::move(other.m_LogStream)),
        m_FrameId(other.m_FrameId),
        m_Enabled(other.m_Enabled)
    {
        other.m_Enabled = false;
    }

    LogCapture& LogCapture::operator=(LogCapture&& other) noexcept
    {
        if (this != &other)
        {
            m_Timestamp = other.m_Timestamp;
#if ENABLE_SOURCE_LOGGING
            m_Location = std::move(other.m_Location);
#endif
            m_Stream = std::move(other.m_Stream);
            m_LogStream = std::move(other.m_LogStream);
            m_FrameId = other.m_FrameId;
            m_Enabled = other.m_Enabled;
            other.m_Enabled = false;
        }
        return *this;
    }

    LogCapture::~LogCapture()
    {
        if (!m_Enabled)
            return;

        m_Stream << '\n';
        Logger::PushMessage(m_Level, std::move(m_Timestamp),
#if ENABLE_SOURCE_LOGGING
            std::move(m_Location),
#endif
            std::move(m_Stream.str()), std::move(m_LogStream), m_FrameId);
    }
}