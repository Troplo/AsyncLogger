#include "LogStream.hpp"
#include "Logger.hpp"

namespace al
{
    LogStream::LogStream(std::string name, void* userData, bool enabled) :
        m_Name(std::move(name)),
        m_UserData(userData),
        m_Enabled(enabled)
    {
    }

    LogStream::LogStream(const LogStream& other) :
        m_Name(other.m_Name),
        m_UserData(other.m_UserData),
        m_Enabled(other.m_Enabled.load())
    {
    }

    LogStream& LogStream::operator=(const LogStream& other)
    {
        if (this != &other)
        {
            m_Name = other.m_Name;
            m_UserData = other.m_UserData;
            m_Enabled.store(other.m_Enabled.load());
        }
        return *this;
    }

    const std::string& LogStream::Name() const
    {
        return m_Name;
    }

    const void* LogStream::UserData() const
    {
        return m_UserData;
    }

    bool LogStream::Enabled() const
    {
        return m_Enabled.load();
    }

    void LogStream::SetEnabled(bool state)
    {
        m_Enabled.store(state);
    }

    LoggerScope::LoggerScope(const std::string& scopeName)
    {
        Logger::PushScope(scopeName);
    }

    LoggerScope::LoggerScope(const std::shared_ptr<LogStream>& scope)
    {
        Logger::PushScope(scope);
    }

    LoggerScope::~LoggerScope()
    {
        Logger::PopScope();
    }
}