#pragma once
#if ENABLE_SOURCE_LOGGING
#include <source_location>
#endif
#include <string_view>

namespace al
{
    class LogIntermediate
    {
    public:
        LogIntermediate(const std::string_view formatString
#if ENABLE_SOURCE_LOGGING
            , const std::source_location location = std::source_location::current()
#endif
            );
        LogIntermediate(const char* formatString
#if ENABLE_SOURCE_LOGGING
           , const std::source_location location = std::source_location::current()
#endif
        );
        LogIntermediate(const std::string& formatString
#if ENABLE_SOURCE_LOGGING
            , const std::source_location location = std::source_location::current()
#endif
        );
        virtual ~LogIntermediate() = default;

        const std::string_view FormatString() const;
#if ENABLE_SOURCE_LOGGING
        std::source_location&& Location();
#endif

    private:
        const std::string_view m_FormatString;
#if ENABLE_SOURCE_LOGGING
        std::source_location m_Location;
#endif
    };
}