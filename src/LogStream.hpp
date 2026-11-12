#pragma once
#include <atomic>
#include <memory>
#include <string>
#include "LogIntermediate.hpp"

namespace al
{
    class LogStream
    {
    public:
        LogStream(std::string name, void* userData = nullptr, bool enabled = true);
        LogStream(const LogStream& other);
        LogStream& operator=(const LogStream& other);
        virtual ~LogStream() = default;

        const std::string& Name() const;
        const void* UserData() const;
        bool Enabled() const;

        void SetEnabled(bool state);

    private:
        std::string m_Name;
        void* m_UserData;
        std::atomic<bool> m_Enabled;
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

    using ScopedLogScope = LoggerScope;
    using ScopedLogStream = LoggerScope;
}