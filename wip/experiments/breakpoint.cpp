// g++ --std=c++20 -Og -ggdb breakpoint.cpp -o breakpoint

namespace gioppler {
    // forcing a function to always be inlined
    // https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
    // https://clang.llvm.org/docs/AttributeReference.html
    // https://docs.microsoft.com/en-us/cpp/cpp/attributes

    // setting a breakpoint for different compilers and processors
    // https://stackoverflow.com/a/49079078
    // https://github.com/nemequ/portable-snippets/blob/master/debug-trap/debug-trap.h
    // https://github.com/scottt/debugbreak/blob/master/debugbreak.h
    // https://clang.llvm.org/docs/LanguageExtensions.html
    // https://llvm.org/docs/LangRef.html#llvm-debugtrap-intrinsic
    // https://docs.microsoft.com/en-us/cpp/intrinsics/debugbreak
    // https://gitlab.gnome.org/GNOME/glib/-/blob/main/glib/gbacktrace.h
    // https://web.archive.org/web/20210114140648/https://processors.wiki.ti.com/index.php/Software_Breakpoints_in_the_IDE
    [[gnu::always_inline]][[clang::always_inline]][[msvc::forceinline]] inline
    void set_breakpoint()
    {
    #if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
        __builtin_debugtrap();   // CLang
    #elif defined(__has_builtin) && __has_builtin(__debugbreak)
        __debugbreak();
    #elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
        __debugbreak();   // Microsoft C Compiler and Intel Compiler Collection
    #elif defined(__i386__) || defined(__x86_64__)
        __asm__ __volatile__("int3");   // x86/x86_64 processors
    #else
        #error Unsupported platform or compiler.
    #endif
    }
}

int test()
{
    gioppler::set_breakpoint();
    int result = 2+2;
    return result;
}

int main()
{
    return test();
}
