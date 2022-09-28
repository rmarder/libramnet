# libramnet - ramnet's C++ utility library

This is a utility library filled with useful vector, string, math, etc functions.
Most functions in this library follow the design style and pattern of the traditional PHP standard library, 
so if you are familiar with that you will feel right at home using this library.

This is a procedural C-style library. There are no classes or objects here.

I have not checked if this library is thread safe, so you should assume it isn't.

This library collides with the C++ library namespace. You must not declare "using namespace std;" in any programs using this library.

See the source code for function prototypes and usage.

This has no dependencies aside from the standard C++ library.
Requires at least a C++11 compiler or better to build.

I develop and test with GCC/G++ and Linux, YMMV on other platforms.

To use, simply git clone https://github.com/rmarder/libramnet and then ./configure && make && make install
