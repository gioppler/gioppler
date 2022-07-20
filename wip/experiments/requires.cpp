#include <string>
#include <string_view>
#include <source_location>
#include <iostream>

enum class BuildMode {off, development, test, profile, qa, production};
constexpr static inline BuildMode g_build_mode = BuildMode::test;

template<BuildMode build_mode = g_build_mode>
void myassert(bool value) requires (build_mode == BuildMode::off)
{
  std::cout << "off" << std::endl;
}

template<BuildMode build_mode = g_build_mode>
void myassert(bool value) requires (build_mode == BuildMode::profile)
{
  std::cout << "profile" << std::endl;
}

template<BuildMode build_mode = g_build_mode>
void myassert(bool value)
{
  std::cout << "default" << std::endl;
}
template<BuildMode build_mode = g_build_mode>
class Function {
 public:
  Function([[maybe_unused]] const std::string_view subsystem = "",
           [[maybe_unused]] const double count = 0.0,
           [[maybe_unused]] std::string session = "",
           [[maybe_unused]] const std::source_location &location =
           std::source_location::current())
  requires (build_mode == BuildMode::off) {
  }

  Function(const std::string_view subsystem = "",
           const double count = 0.0,
           std::string session = "",
           const std::source_location &location = std::source_location::current())
  requires (build_mode == BuildMode::profile) {

  }

  Function(const std::string_view subsystem = "",
           const double count = 0.0,
           std::string session = "",
           const std::source_location &location = std::source_location::current())
  {

  }
};

int main()
{
    Function fn{};
    myassert(true);

    return 0;
}
