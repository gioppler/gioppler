#include <iostream>
#include <memory>
#include <functional>

int ff(std::function<int()> fp)
{
    std::cout << "ff called" << std::endl;
    const int fpval = fp();
    std::cout << "fp()=" << fpval << std::endl;
    return fpval;
}

int f()
{
    std::cout << "f called" << std::endl;
    return 12;
}

struct F
{
    int invariant() {
        std::cout << "F::invariant() called" << std::endl;
        return 21;
    }

    int fff() {
        return ff([this]{ return invariant(); });
    }
};

int main()
{
    const int fval = ff(f);
    std::cout << "f()=" << fval << std::endl;

    F g;

    const int gval = g.invariant();
    std::cout << "g.invariant()=" << gval << std::endl;

    const int fffval = g.fff();
    std::cout << "fffval()=" << fffval << std::endl;

    return ff([&g]() ->int { return g.invariant(); });
}
