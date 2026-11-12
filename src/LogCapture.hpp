#pragma once
#include <chrono>
#include <optional>
#include <source_location>
#include <sstream>
#include "LogLevel.hpp"
#include "LogStream.hpp"

namespace al
{
	class Logger;
	class LogCapture
	{
	public:
		LogCapture(const eLogLevel level,
#if ENABLE_SOURCE_LOGGING
			std::source_location&& location,
#endif
			std::optional<std::shared_ptr<LogStream>> stream = std::nullopt);
		~LogCapture();

		LogCapture(LogCapture&& other) noexcept;
		LogCapture& operator=(LogCapture&& other) noexcept;
		LogCapture(const LogCapture&) = delete;
		LogCapture& operator=(const LogCapture&) = delete;

		bool Enabled() const { return m_Enabled; }

		template<typename T>
		std::ostream& operator<< (const T& d);

	private:
		const eLogLevel m_Level;
		std::chrono::system_clock::time_point m_Timestamp;
#if ENABLE_SOURCE_LOGGING
		std::source_location m_Location;
#endif
		std::ostringstream m_Stream;
		std::optional<std::shared_ptr<LogStream>> m_LogStream;
		std::optional<uint64_t> m_FrameId;
		bool m_Enabled = true;
	};

	template<typename T>
	std::ostream& LogCapture::operator<< (const T& d)
	{
		if (m_Enabled)
			m_Stream << d;
		return m_Stream;
	}
}