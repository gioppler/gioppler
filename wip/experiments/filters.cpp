#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <cstdlib>

#define GIOPPLER_STRINGIZE2(s) #s
#define GIOPPLER_STRINGIZE(s)  GIOPPLER_STRINGIZE2(s)

namespace filter
{

// Goals:
//   1) If empty compile-time filter, disable at compile-time.
//   2) If compile-time GIOPPLER_FILE and/or GIOPPLER_FUNCTION, evaluate them.
//   3) Same compile/run time filter values and check at runtime.

// GIOPPLER_FILE
// GIOPPLER_FUNCTION
// GIOPPLER_FILTER

// GIOPPLER_SUBSYSTEM
// GIOPPLER_CLIENT
// GIOPPLER_REQUEST


bool is_user_enabled(std::string_view current_user)
{
#if defined(GIOPPLER_USER)
constinit static const char* user_filter = GIOPPLER_STRINGIZE(GIOPPLER_USER);
#else
static const char* g_user_filter = std::getenv("GIOPPLER_USER") ? std::getenv("GIOPPLER_USER") : "";
#endif

}


#if defined(FILTER)
consteval bool filter_enabled(std::string_view filter) {
    return (filter == GIOPPLER_STRINGIZE(FILTER));
}
#else
bool filter_enabled(std::string_view filter) {
    const char* filter_env = std::getenv("FILTER");
    return (filter == (filter_env ? filter_env : ""));
}
#endif

bool match(std::string_view filter) {
    if (filter_enabled(filter)) {
        return true;
    } else {
        return false;
    }
}

bool is_enabled(std::string_view filter,
                const std::source_location location = std::source_location::current())
{

}

} // namespace filter

int main()
{
    const bool matched = match("hello");
    std::cout << (matched ? "matched" : "not matched") << std::endl;
    return 0;
}
