/*
 * libramnet - ramnet's C++ utility library.
 * https://github.com/rmarder/libramnet
 *
 * Copyright (C) 2022 Robert Alex Marder (ram@robertmarder.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef _RAMNET_H_
#define _RAMNET_H_

#define _POSIX_C_SOURCE 200809L

#include <string>
#include <climits>
#include <vector>

namespace ramnet {

// php constants
// these can be set to anything as long as they are unique
// we only test == we never test >< etc
const size_t STR_PAD_RIGHT = 1;
const size_t STR_PAD_LEFT = 2;
const size_t STR_PAD_BOTH = 3;

// math functions
int rand(const int min = 0, const int max = RAND_MAX);
bool is_int(const std::string &str);

// filesystem functions
std::string file_get_contents(const std::string &str);

// string functions
std::string ltrim(const std::string &str, const std::string &whitespace = " \n\r\t\f\v");
std::string rtrim(const std::string &str, const std::string &whitespace = " \n\r\t\f\v");
std::string trim(const std::string &str, const std::string &whitespace = " \n\r\t\f\v");
bool str_contains(const std::string &haystack, const std::string &needle);
std::string strrev(std::string str);
std::string str_rot13(const std::string &str);
std::string str_rot13(const std::string &str);
std::string str_rot13(const std::string &str);
std::string str_repeat(const std::string &str, const size_t times);
std::string str_pad(const std::string &input, size_t length, const std::string pad_str = " ", size_t pad_type = STR_PAD_RIGHT);
std::string ucfirst(std::string str);
std::string lcfirst(std::string str);
std::string str_replace(const std::string &search, const std::string &replace, const std::string &subject);

// array functions
std::vector<std::string> explode(std::string const &search, std::string const &subject, int limit = INT_MAX);
std::vector<std::string> str_split(const std::string &str, size_t length = 1);
std::string implode(const std::string &separator, const std::vector<std::string> &array);

// network functions
std::string dnslookup(const std::string &input);

}

#endif
