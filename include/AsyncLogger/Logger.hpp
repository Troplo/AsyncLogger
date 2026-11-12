#pragma once
#if !defined(USE_FMT)
    #include <format>
    #define FORMAT std::format
    #define VFORMAT std::vformat
    #define MAKE_FORMAT_ARGS std::make_format_args
#else
    #include <fmt/core.h>
    #define FORMAT fmt::format
    #define VFORMAT fmt::vformat
    #define MAKE_FORMAT_ARGS fmt::make_format_args
#endif
#ifndef ENABLE_SOURCE_LOGGING
    #define ENABLE_SOURCE_LOGGING 1
#endif
#include <filesystem>
#include <functional>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>
#include "concurrency/shared_queue.hpp"
#include "LogCapture.hpp"
#include "LogFormatter.hpp"
#include "LogIntermediate.hpp"
#include "LogLevel.hpp"
#include "LogMessage.hpp"
#include "LogStream.hpp"

namespace al
{
    using LogMessagePtr = std::shared_ptr<LogMessage>;
    using LogSink = std::function<void(LogMessagePtr)>;
    using FrameIdDelegate = std::function<uint64_t()>;
    using FrameIdProvider = FrameIdDelegate;

    class Logger
    {
    public:
        Logger() = default;
        virtual ~Logger() = default;

        static void AddSink(LogSink sink);
        static void AddConsoleSink(bool colorize = true, const LogFormatOptions& options = {});
        static void AddFileSink(const std::filesystem::path& path, const LogFormatOptions& options = {});
        static void Destroy();
        static void Init();
        /**
         * @brief Attempts to flush the log queue immediately until it's completely empty.
         */
        static void FlushQueue();

        static void SetFrameIdProvider(FrameIdDelegate provider);
        static void ClearFrameIdProvider();
        static std::optional<uint64_t> GetCurrentFrameId();

        static std::shared_ptr<LogScope> RegisterScope(const std::string& name, bool enabled = true, void* userData = nullptr);
        static std::shared_ptr<LogScope> GetOrCreateScope(const std::string& name, bool defaultEnabled = true);
        static std::shared_ptr<LogScope> GetScope(const std::string& name);
        static bool SetScopeEnabled(const std::string& name, bool enabled);
        static bool IsScopeEnabled(const std::string& name);
        static std::vector<std::shared_ptr<LogScope>> GetAllScopes();

        static void PushScope(const std::shared_ptr<LogScope>& scope);
        static void PushScope(const std::string& scopeName);
        static void PopScope();
        static std::shared_ptr<LogScope> GetCurrentScope();

        static void RegisterPathScope(std::string pathPrefix, const std::string& scopeName);
        static std::shared_ptr<LogScope> ResolveScopeFromPath(const std::string_view filePath);

    protected:
        static void PushMessage(const eLogLevel level, std::chrono::system_clock::time_point&& timestamp,
#if ENABLE_SOURCE_LOGGING
            std::source_location&& location,
#endif
            std::string&& message,
            std::optional<std::shared_ptr<LogStream>>&& stream = std::nullopt,
            std::optional<uint64_t> frameId = std::nullopt) noexcept;

    private:
        void CallSinks(LogMessagePtr msgPtr);
        void DestroyImpl();
        void InitImpl();
        /**
         * @brief Attempts to flush the entire log queue before returning
         */
        void FlushQueueImpl();
        void QueueMessage(LogMessagePtr msgPtr);

        static Logger& GetInstance()
        {
            static Logger i{};
            return i;
        }

    private:
        friend class LogCapture;

        shared_queue<std::function<void()>> m_Queue;
        std::vector<LogSink> m_Sinks;
        bool m_Running = false;
        std::thread m_LogWorker;

        FrameIdDelegate m_FrameIdDelegate;
        std::mutex m_FrameIdMutex;

        std::unordered_map<std::string, std::shared_ptr<LogStream>> m_Scopes;
        std::mutex m_ScopeMutex;

        std::vector<std::pair<std::string, std::shared_ptr<LogStream>>> m_PathScopes;
        std::mutex m_PathMutex;
    };

    // Standard LOG(level)
    extern LogCapture LOG(const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location = std::source_location::current()
#endif
    );

    // LOG("Scope", level)
    extern LogCapture LOG(const std::string_view scopeName, const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location = std::source_location::current()
#endif
    );

    // LOG(level, "Scope")
    extern LogCapture LOG(const eLogLevel level, const std::string_view scopeName
#if ENABLE_SOURCE_LOGGING
        , std::source_location location = std::source_location::current()
#endif
    );

    // LOG(scopePtr, level)
    extern LogCapture LOG(const std::shared_ptr<LogStream>& stream, const eLogLevel level
#if ENABLE_SOURCE_LOGGING
        , std::source_location location = std::source_location::current()
#endif
    );

    // LOG(level, scopePtr)
    extern LogCapture LOG(const eLogLevel level, const std::shared_ptr<LogStream>& stream
#if ENABLE_SOURCE_LOGGING
        , std::source_location location = std::source_location::current()
#endif
    );

    template<typename ...Args>
    inline void LOGF(const eLogLevel level, LogIntermediate formatString, Args&&... formatArgs)
    {
        auto capture = LogCapture(level,
#if ENABLE_SOURCE_LOGGING
            std::move(formatString.Location()),
#endif
            std::nullopt);
        if (capture.Enabled())
            capture << VFORMAT(formatString.FormatString(), MAKE_FORMAT_ARGS(formatArgs...));
    }

    template<typename... Args>
    inline void LOGF(const std::shared_ptr<LogStream> stream, const eLogLevel level, LogIntermediate formatString, Args&&... formatArgs)
    {
        if (stream && !stream->Enabled())
            return;

        auto capture = LogCapture(level,
#if ENABLE_SOURCE_LOGGING
            std::move(formatString.Location()),
#endif
            stream);
        if (capture.Enabled())
            capture << VFORMAT(formatString.FormatString(), MAKE_FORMAT_ARGS(formatArgs...));
    }

    template<typename... Args>
    inline void LOGF(const std::string_view scopeName, const eLogLevel level, LogIntermediate formatString, Args&&... formatArgs)
    {
        auto scope = Logger::GetOrCreateScope(std::string(scopeName));
        if (scope && !scope->Enabled())
            return;

        auto capture = LogCapture(level,
#if ENABLE_SOURCE_LOGGING
            std::move(formatString.Location()),
#endif
            scope);
        if (capture.Enabled())
            capture << VFORMAT(formatString.FormatString(), MAKE_FORMAT_ARGS(formatArgs...));
    }
}
