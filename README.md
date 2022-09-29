# libramnet - ramnet's C++ utility library

This is a utility library filled with useful vector, string, math, etc functions.
Most functions in this library follow the design style and pattern of the traditional PHP standard library, 
so if you are familiar with that you will feel right at home using this library.

This is a procedural C-style library. There are no classes or objects here.

I have not checked if this library is thread safe, so you should assume it isn't.

See the source code for function prototypes and usage.

Requires at least a C++11 compiler or better to build.

Requires a POSIX compliant system. We currently define _POSIX_C_SOURCE 200809L

Requires [libcurl] built with HTTP support against an ssl/tls library such as OpenSSL.

I develop and test with GCC/G++ and Linux, YMMV on other platforms.

To use, simply do:

git clone https://github.com/rmarder/libramnet

./configure && make && make test && make install

[libcurl]: <https://curl.se/libcurl/>
