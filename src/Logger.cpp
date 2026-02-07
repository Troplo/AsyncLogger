#include "Logger.hpp"
#include "LogCapture.hpp"
#include "LogMessage.hpp"

namespace al
{
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

    void Logger::AddSink(LogSink sink)
    {
        Logger::GetInstance().m_Sinks.push_back(sink);
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

    void Logger::PushMessage(const eLogLevel level,
                             std::chrono::system_clock::time_point &&timestamp,
#if ENABLE_SOURCE_LOGGING
                             std::source_location &&location,
#endif
                             std::string &&message, std::optional<std::shared_ptr<LogStream> const> &&stream) noexcept
    {
        auto msgPtr = std::make_shared<LogMessage>(level, std::move(timestamp),
#if ENABLE_SOURCE_LOGGING
            std::move(location),
#endif
            std::move(message), std::move(stream));

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