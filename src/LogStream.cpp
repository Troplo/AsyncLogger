#include "LogStream.hpp"
#include "Logger.hpp"
#include "LogMessage.hpp"

namespace al
{
    LogStream::LogStream(std::string name, void* userData, bool enabled, size_t scrollbackLimit) :
        m_Name(std::move(name)),
        m_UserData(userData),
        m_Enabled(enabled),
        m_ScrollbackLimit(scrollbackLimit)
    {
    }

    LogStream::LogStream(const LogStream& other) :
        m_Name(other.m_Name),
        m_UserData(other.m_UserData),
        m_Enabled(other.m_Enabled.load()),
        m_ScrollbackLimit(other.m_ScrollbackLimit.load())
    {
        std::lock_guard<std::mutex> lock(other.m_EntriesMutex);
        m_Entries = other.m_Entries;
    }

    LogStream& LogStream::operator=(const LogStream& other)
    {
        if (this != &other)
        {
            m_Name = other.m_Name;
            m_UserData = other.m_UserData;
            m_Enabled.store(other.m_Enabled.load());
            m_ScrollbackLimit.store(other.m_ScrollbackLimit.load());
            std::scoped_lock lock(m_EntriesMutex, other.m_EntriesMutex);
            m_Entries = other.m_Entries;
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

    size_t LogStream::ScrollbackLimit() const
    {
        return m_ScrollbackLimit.load();
    }

    void LogStream::SetScrollbackLimit(size_t limit)
    {
        m_ScrollbackLimit.store(limit);
        std::lock_guard<std::mutex> lock(m_EntriesMutex);
        if (limit == 0)
        {
            m_Entries.clear();
        }
        else
        {
            while (m_Entries.size() > limit)
            {
                m_Entries.pop_front();
            }
        }
    }

    std::vector<LogMessagePtr> LogStream::GetEntries() const
    {
        std::lock_guard<std::mutex> lock(m_EntriesMutex);
        return std::vector<LogMessagePtr>(m_Entries.begin(), m_Entries.end());
    }

    void LogStream::ClearEntries()
    {
        std::lock_guard<std::mutex> lock(m_EntriesMutex);
        m_Entries.clear();
    }

    size_t LogStream::EntryCount() const
    {
        std::lock_guard<std::mutex> lock(m_EntriesMutex);
        return m_Entries.size();
    }

    void LogStream::StoreEntry(const LogMessagePtr& entry)
    {
        const size_t limit = m_ScrollbackLimit.load();
        if (limit == 0 || !entry)
            return;

        std::lock_guard<std::mutex> lock(m_EntriesMutex);
        m_Entries.push_back(entry);
        while (m_Entries.size() > limit)
        {
            m_Entries.pop_front();
        }
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