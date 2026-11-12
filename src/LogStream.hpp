#pragma once
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "LogIntermediate.hpp"

#ifndef DEFAULT_SCROLLBACK_LIMIT
#define DEFAULT_SCROLLBACK_LIMIT 100
#endif

namespace al
{
    class LogMessage;
    using LogMessagePtr = std::shared_ptr<LogMessage>;

    class LogStream
    {
    public:
        LogStream(std::string name, void* userData = nullptr, bool enabled = true, size_t scrollbackLimit = DEFAULT_SCROLLBACK_LIMIT);
        LogStream(const LogStream& other);
        LogStream& operator=(const LogStream& other);
        virtual ~LogStream() = default;

        const std::string& Name() const;
        const void* UserData() const;
        bool Enabled() const;

        void SetEnabled(bool state);

        size_t ScrollbackLimit() const;
        void SetScrollbackLimit(size_t limit);

        std::vector<LogMessagePtr> GetEntries() const;
        std::vector<LogMessagePtr> GetScrollback() const { return GetEntries(); }
        void ClearEntries();
        void ClearScrollback() { ClearEntries(); }
        size_t EntryCount() const;

        void StoreEntry(const LogMessagePtr& entry);

    private:
        std::string m_Name;
        void* m_UserData;
        std::atomic<bool> m_Enabled;
        std::atomic<size_t> m_ScrollbackLimit;
        mutable std::mutex m_EntriesMutex;
        std::deque<LogMessagePtr> m_Entries;
    };

    using LogScope = LogStream;
    using LogStreamPtr = std::shared_ptr<LogStream>;
    using LogScopePtr = std::shared_ptr<LogScope>;

    class LoggerScope
    {
    public:
        explicit LoggerScope(const std::string& scopeName);
        explicit LoggerScope(const std::shared_ptr<LogStream>& scope);
        ~LoggerScope();

        LoggerScope(const LoggerScope&) = delete;
        LoggerScope& operator=(const LoggerScope&) = delete;
        LoggerScope(LoggerScope&&) = delete;
        LoggerScope& operator=(LoggerScope&&) = delete;
    };
}