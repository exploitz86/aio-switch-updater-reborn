#ifndef VERSION_CONFIG_H
#define VERSION_CONFIG_H

#include <cstddef>

// Version is pulled from Makefile via -DAPP_VERSION
#define VERSION_APP APP_VERSION
#define VERSION_TAG_APP ""

// Compile-time string parsing helper functions
namespace version_parser {
    constexpr unsigned int parse_number(const char* str, size_t& pos) {
        unsigned int result = 0;
        while (str[pos] >= '0' && str[pos] <= '9') {
            result = result * 10 + (str[pos] - '0');
            pos++;
        }
        return result;
    }

    constexpr unsigned int get_major(const char* version) {
        size_t pos = 0;
        return parse_number(version, pos);
    }

    constexpr unsigned int get_minor(const char* version) {
        size_t pos = 0;
        // Skip major version
        parse_number(version, pos);
        if (version[pos] == '.') pos++;
        return parse_number(version, pos);
    }

    constexpr unsigned int get_micro(const char* version) {
        size_t pos = 0;
        // Skip major version
        parse_number(version, pos);
        if (version[pos] == '.') pos++;
        // Skip minor version
        parse_number(version, pos);
        if (version[pos] == '.') pos++;
        return parse_number(version, pos);
    }
}

// Parse version components from APP_VERSION string at compile time
#define VERSION_MAJOR_APP version_parser::get_major(APP_VERSION)
#define VERSION_MINOR_APP version_parser::get_minor(APP_VERSION)
#define VERSION_MICRO_APP version_parser::get_micro(APP_VERSION)

// Version getter functions
inline unsigned int get_version_major()
{
    return VERSION_MAJOR_APP;
}

inline unsigned int get_version_minor()
{
    return VERSION_MINOR_APP;
}

inline unsigned int get_version_micro()
{
    return VERSION_MICRO_APP;
}

inline const char* get_version_tag(){
    return VERSION_TAG_APP;
}

#endif // VERSION_CONFIG_H
