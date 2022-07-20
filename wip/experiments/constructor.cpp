#include <iostream>
#include <string_view>

int starter(std::string_view group, std::string_view name)
{
  std::cout << "starter: " << group << " " << name << std::endl;
  return 0;
}

static const int s = starter("basic", "simple");

int main()
{
  std::cout << "main" << std::endl;
  return 0;
}
