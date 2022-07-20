#include <iostream>
#include <fstream>
#include <memory>

std::unique_ptr<std::ostream> get_stream()
{
    return std::make_unique<std::ostream>(std::cout.rdbuf());
}

int main()
{
    auto out1 = get_stream();
    auto out2 = get_stream();
    auto out3 = get_stream();
    auto out4 = get_stream();
    auto out5 = get_stream();
    *out1 << "hello!1" << std::endl;
    *out2 << "hello!2" << std::endl;
    *out3 << "hello!3" << std::endl;
    *out4 << "hello!4" << std::endl;
    *out5 << "hello!5" << std::endl;

    return 0;
}
