#include "Logger.hpp"
#include "LogCapture.hpp"
#include "LogMessage.hpp"
#include <fstream>
#include <iostream>

namespace al
{
    static thread_local std::vector<std::shared_ptr<LogStream>> t_ScopeStack;

    LogCapture LOG(const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location
#endif
        )
    {
        return LogCapture{ level,
#if ENABLE_SOURCE_LOGGING
            std::move(location),
#endif
            std::nullopt };
    }

    LogCapture LOG(const std::string_view scopeName, const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location
#endif
        )
    {
        auto scope = Logger::GetOrCreateScope(std::string(scopeName));
        return LogCapture{ level,
#if ENABLE_SOURCE_LOGGING
            std::move(location),
#endif
            scope };
    }

    LogCapture LOG(const eLogLevel level, const std::string_view scopeName
#if ENABLE_SOURCE_LOGGING
        , std::source_location location
#endif
        )
    {
        return LOG(scopeName, level
#if ENABLE_SOURCE_LOGGING
            , std::move(location)
#endif
        );
    }

    LogCapture LOG(const std::shared_ptr<LogStream>& stream, const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location
#endif
        )
    {
        return LogCapture{ level,
#if ENABLE_SOURCE_LOGGING
            std::move(location),
#endif
            stream };
    }

    LogCapture LOG(const eLogLevel level, const std::shared_ptr<LogStream>& stream
#if ENABLE_SOURCE_LOGGING
        , std::source_location location
#endif
        )
    {
        return LOG(stream, level
#if ENABLE_SOURCE_LOGGING
            , std::move(location)
#endif
        );
    }

    void Logger::AddSink(LogSink sink)
    {
        Logger::GetInstance().m_Sinks.push_back(std::move(sink));
    }

    void Logger::AddConsoleSink(bool colorize, const LogFormatOptions& options)
    {
        LogFormatOptions opts = options;
        opts.colorize = colorize;
        AddSink([opts](LogMessagePtr msg) {
            std::cout << LogFormatter::Format(msg, opts);
            std::cout.flush();
        });
    }

    void Logger::AddFileSink(const std::filesystem::path& path, const LogFormatOptions& options)
    {
        auto fileStream = std::make_shared<std::ofstream>(path, std::ios::out | std::ios::app);
        LogFormatOptions opts = options;
        opts.colorize = false;
        AddSink([fileStream, opts](LogMessagePtr msg) {
            if (fileStream && fileStream->is_open())
            {
                *fileStream << LogFormatter::Format(msg, opts);
                fileStream->flush();
            }
        });
    }

    void Logger::Destroy()
    {
        Logger::GetInstance().DestroyImpl();
    }

    void Logger::Init()
    {
        Logger::GetInstance().InitImpl();
    }

    void Logger::FlushQueue()
    {
        GetInstance().FlushQueueImpl();
    }

    void Logger::SetFrameIdProvider(FrameIdDelegate provider)
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_FrameIdMutex);
        instance.m_FrameIdDelegate = std::move(provider);
    }

    void Logger::ClearFrameIdProvider()
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_FrameIdMutex);
        instance.m_FrameIdDelegate = nullptr;
    }

    std::optional<uint64_t> Logger::GetCurrentFrameId()
    {
        FrameIdDelegate delegate;
        {
            auto& instance = GetInstance();
            std::lock_guard<std::mutex> lock(instance.m_FrameIdMutex);
            delegate = instance.m_FrameIdDelegate;
        }
        if (delegate)
        {
            return delegate();
        }
        return std::nullopt;
    }

    std::shared_ptr<LogScope> Logger::RegisterScope(const std::string& name, bool enabled, void* userData)
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_ScopeMutex);
        auto it = instance.m_Scopes.find(name);
        if (it != instance.m_Scopes.end())
        {
            it->second->SetEnabled(enabled);
            return it->second;
        }
        auto scope = std::make_shared<LogStream>(name, userData, enabled);
        instance.m_Scopes.emplace(name, scope);
        return scope;
    }

    std::shared_ptr<LogScope> Logger::GetOrCreateScope(const std::string& name, bool defaultEnabled)
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_ScopeMutex);
        auto it = instance.m_Scopes.find(name);
        if (it != instance.m_Scopes.end())
        {
            return it->second;
        }
        auto scope = std::make_shared<LogStream>(name, nullptr, defaultEnabled);
        instance.m_Scopes.emplace(name, scope);
        return scope;
    }

    std::shared_ptr<LogScope> Logger::GetScope(const std::string& name)
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_ScopeMutex);
        auto it = instance.m_Scopes.find(name);
        if (it != instance.m_Scopes.end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool Logger::SetScopeEnabled(const std::string& name, bool enabled)
    {
        auto scope = GetOrCreateScope(name, enabled);
        scope->SetEnabled(enabled);
        return true;
    }

    bool Logger::IsScopeEnabled(const std::string& name)
    {
        auto scope = GetScope(name);
        if (scope)
        {
            return scope->Enabled();
        }
        return true;
    }

    std::vector<std::shared_ptr<LogScope>> Logger::GetAllScopes()
    {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_ScopeMutex);
        std::vector<std::shared_ptr<LogScope>> result;
        result.reserve(instance.m_Scopes.size());
        for (const auto& [name, scope] : instance.m_Scopes)
        {
            result.push_back(scope);
        }
        return result;
    }

    void Logger::PushScope(const std::shared_ptr<LogScope>& scope)
    {
        if (scope)
            t_ScopeStack.push_back(scope);
    }

    void Logger::PushScope(const std::string& scopeName)
    {
        auto scope = GetOrCreateScope(scopeName);
        t_ScopeStack.push_back(scope);
    }

    void Logger::PopScope()
    {
        if (!t_ScopeStack.empty())
            t_ScopeStack.pop_back();
    }

    std::shared_ptr<LogScope> Logger::GetCurrentScope()
    {
        if (!t_ScopeStack.empty())
            return t_ScopeStack.back();
        return nullptr;
    }

    void Logger::RegisterPathScope(std::string pathPrefix, const std::string& scopeName)
    {
        auto scope = GetOrCreateScope(scopeName);
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_PathMutex);
        instance.m_PathScopes.emplace_back(std::move(pathPrefix), scope);
    }

    std::shared_ptr<LogScope> Logger::ResolveScopeFromPath(const std::string_view filePath)
    {
        if (filePath.empty())
            return nullptr;

        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_PathMutex);
        for (const auto& [pattern, scope] : instance.m_PathScopes)
        {
            if (filePath.find(pattern) != std::string_view::npos)
            {
                return scope;
            }
        }
        return nullptr;
    }

    void Logger::PushMessage(const eLogLevel level,
                             std::chrono::system_clock::time_point &&timestamp,
#if ENABLE_SOURCE_LOGGING
                             std::source_location &&location,
#endif
                             std::string &&message,
                             std::optional<std::shared_ptr<LogStream>> &&stream,
                             std::optional<uint64_t> frameId) noexcept
    {
        auto msgPtr = std::make_shared<LogMessage>(level, std::move(timestamp),
#if ENABLE_SOURCE_LOGGING
            std::move(location),
#endif
            std::move(message), std::move(stream), frameId);

        Logger::GetInstance().QueueMessage(std::move(msgPtr));
    }

    void Logger::CallSinks(LogMessagePtr msgPtr)
    {
        for (const auto &sink : m_Sinks)
        {
            sink(msgPtr);
        }
    }

    void Logger::DestroyImpl()
    {
        m_Queue.push([this]
        {
            m_Running = false;
        });
        if (m_LogWorker.joinable())
            m_LogWorker.join();
    }

    void Logger::InitImpl()
    {
        m_Running = true;
        m_LogWorker = std::thread([this]
        {
            while (m_Running)
            {
                std::function<void()> func;
                m_Queue.wait_and_pop(func);
                func();
            }
        });
    }

    void Logger::FlushQueueImpl()
    {
        while (!m_Queue.empty())
        {
            std::function<void()> func;
            m_Queue.wait_and_pop(func);
            func();
        }
    }

    void Logger::QueueMessage(LogMessagePtr msgPtr)
    {
        m_Queue.push([this, msgPtr = std::move(msgPtr)]
        {
            CallSinks(msgPtr);
        });
    }
}