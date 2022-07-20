#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <future>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <unordered_map>
#include <variant>
#include <vector>
using namespace std::chrono_literals;

#define FMT_HEADER_ONLY 1
#include <fmt/format.h>
#include <fmt/chrono.h>
template <typename... T>
[[nodiscard]] std::string format(std::string_view fmt, T&&... args) {
  return fmt::vformat(fmt, fmt::make_format_args(args...));
}

/// simplify creating shared pointers to unordered maps
// forces the use of an initialization list constructor
// without this we get an ambiguous overload error
// example: auto foo = make_shared_init_list<std::map<double,std::string>>({{1000, s1}});
// https://stackoverflow.com/questions/36445642/initialising-stdshared-ptrstdmap-using-braced-init
// https://stackoverflow.com/a/36446143/4560224
// https://en.cppreference.com/w/cpp/utility/initializer_list
// https://en.cppreference.com/w/cpp/language/list_initialization
template<class Map>
std::shared_ptr<Map>
make_shared_init_list(std::initializer_list<typename Map::value_type> il) {
    return std::make_shared<Map>(il);
}

using RecordValue = std::variant<bool, int64_t, double, std::string, std::chrono::system_clock::time_point>;
using Record = std::unordered_map<std::string, RecordValue>;
enum class RecordIndex {boolean = 0, integer, real, string, timestamp};

using MyMap = std::unordered_map<std::string,std::string>;

int main( )
{
  std::shared_ptr<Record> record{make_shared_init_list<Record>({
      {"category", {"contract"}},
      {"subcategory", {"argument"}},
      {"message", {123}}
    })};

  std::shared_ptr<MyMap> rec2 = make_shared_init_list<MyMap>( {{"yes", "no"}} );
  std::cout << "test2" << std::endl;

  return 0;
}
