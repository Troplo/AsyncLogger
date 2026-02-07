#include "LogIntermediate.hpp"

namespace al
{
    LogIntermediate::LogIntermediate(const std::string_view formatString
#if ENABLE_SOURCE_LOGGING
                                     , const std::source_location location
#endif
) :
        m_FormatString(formatString)
#if ENABLE_SOURCE_LOGGING
        , m_Location(location)
#endif
    {
        
    }

    LogIntermediate::LogIntermediate(const char* formatString
#if ENABLE_SOURCE_LOGGING
                                     , const std::source_location location
#endif
) :
        m_FormatString(formatString)
#if ENABLE_SOURCE_LOGGING
        , m_Location(location)
#endif
    {

    }

LogIntermediate::LogIntermediate(const std::string &formatString
#if ENABLE_SOURCE_LOGGING
                                 , const std::source_location location
#endif
) :
    m_FormatString(formatString)
#if ENABLE_SOURCE_LOGGING
    , m_Location(location)
#endif
    {

    }

    const std::string_view LogIntermediate::FormatString() const
    {
        return m_FormatString;
    }

#if ENABLE_SOURCE_LOGGING
    std::source_location&& LogIntermediate::Location()
    {
        return std::move(m_Location);
    }
#endif
}
