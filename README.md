# libramnet - ramnet's C++ utility library

This is a utility library filled with useful vector, string, math, etc functions.
Most functions in this library follow the design style and pattern of the traditional PHP standard library, 
so if you are familiar with that you will feel right at home using this library.

This is a procedural C-style library. There are no classes or objects here.

This library is not thread safe. You have been warned.

See the source code for function prototypes and usage. test.cpp contains example usage of everything.

Requires at least a C++11 compiler or better to build.

Requires a POSIX compliant system. We currently define _POSIX_C_SOURCE 200809L

Requires [libcurl] built with HTTP support against an ssl/tls library such as OpenSSL.

Requires [base64] base64 encoding and decoding with c++, which is bundled in this project.

Requires [libtls] libtls, we test against libretls for openssl

I develop and test with GCC/G++ and Linux, YMMV on other platforms.

To use, simply do:

git clone https://github.com/rmarder/libramnet

cd libramnet

sh configure && make && make test && sudo make install

[libcurl]: <https://curl.se/libcurl/>
[base64]: <https://github.com/ReneNyffenegger/cpp-base64>
[libtls]: <https://git.causal.agency/libretls/>
