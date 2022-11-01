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

#ifndef _RAMNET_C_
#define _RAMNET_C_

#include "ramnet.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <climits>
#include <vector>
#include <map>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <cstring>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

// this can be found in "apk add curl-dev"
#include <curl/curl.h>

// this comes from either LibreSSL or the LibreTLS libtls for OpenSSL project.
// the latter can be found in "apk add libretls-dev"
#include <tls.h>

// this is deliberate, we want to build base64.cpp and base64.h directly into libramnet
#include "base64.cpp"

// most of the functions in this library mirror the useful functions contained in the PHP standard library.

namespace ramnet {

/***********************
 * internal curl stuff *
************************
*/

namespace {

struct http
{
	long status;
	std::string body;
};

static size_t CURL_WriteCallback(const void *contents, const size_t size, const size_t nmemb, const void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

http http_fetch(const std::string url)
{
	CURL *curl;
	CURLcode res;
	std::string read_buffer;
	long response_code;
	http result;

	// default result returned on failure
	result.body = "";
	result.status = 499;

	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L); // 30 seconds
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L); // follow up to 10 redirects
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURL_WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		}
		else
		{
			std::cerr << "CURL failure. terminating..." << std::endl;
			exit(EXIT_FAILURE);
		}
		result.body = read_buffer;
		result.status = response_code;
		curl_easy_cleanup(curl);
	}
	else
	{
		std::cerr << "CURL failure. terminating..." << std::endl;
		exit(EXIT_FAILURE);
	}
	return result;
}

} // end anonymous namespace

/****************
 * base64 stuff *
 ****************
*/

std::string __base64_encode(const std::string &str)
{
	return base64_encode(str);
}

std::string __base64_decode(const std::string &str)
{
	return base64_decode(str);
}

/*********************
 * process functions *
**********************
*/

struct popen2
{
	pid_t child_pid;
	int from_child, to_child;
};

struct shell_exec
{
	int status;
	std::string output;
};

int popen2(const char *cmdline, struct popen2 *childinfo)
{
	pid_t p;
	int pipe_stdin[2], pipe_stdout[2];

	if(pipe(pipe_stdin)) return -1;
	if(pipe(pipe_stdout)) return -1;

	p = fork();
	if(p < 0) return p; /* fork failed */
	if(p == 0)
	{
		/* child */
		close(pipe_stdin[1]);
		dup2(pipe_stdin[0], 0);
		close(pipe_stdout[0]);
		dup2(pipe_stdout[1], 1);
		execl("/bin/sh", "sh", "-c", cmdline, NULL);
		perror("execl");
		exit(EXIT_FAILURE);
	}
	childinfo->child_pid = p;
	childinfo->to_child = pipe_stdin[1];
	childinfo->from_child = pipe_stdout[0];
	return 0;
}

// simple wrapper around popen2()
// ensures process terminatation after timeout seconds. timeout 0 runs forever.
std::string shell_exec(const std::string &cmd, const std::string &input, int &status, int timeout)
{
	char buf[8192];
	struct popen2 kid;
	popen2(cmd.c_str(), &kid);
	write(kid.to_child, input.c_str(), input.length());
	close(kid.to_child);
	memset(buf, 0, 8192);

	if(timeout > 0)
	{
		timeout = timeout * 10;
		for(int i = 0; i < timeout; i++)
		{
			if(waitpid(kid.child_pid, &status, WNOHANG) != 0)
			{
				// process finished before timeout.
				break;
			}
			usleep(100000); // 0.1 second
		}
		if(waitpid(kid.child_pid, &status, WNOHANG) == 0)
		{
			// process still running. kill it.
			kill(kid.child_pid, SIGTERM);
			usleep(100000);
			if(waitpid(kid.child_pid, &status, WNOHANG) == 0)
			{
				kill(kid.child_pid, SIGKILL);
			}
		}
	}

	// wait for and reap the child
	waitpid(kid.child_pid, &status, 0);

	// collect all of the output from child, do not block
	fcntl(kid.from_child, F_SETFL, O_NONBLOCK);
	read(kid.from_child, buf, 8192);
	close(kid.from_child);

	std::string output = buf;
	status = WEXITSTATUS(status);

	return output;
}

// alternative way of calling shell_exec() with fewer arguments
std::string shell_exec(const std::string &cmd, const std::string &input, int &status)
{
	return shell_exec(cmd, input, status, 0);
}

// alternative way of calling shell_exec() with fewer arguments
std::string shell_exec(const std::string &cmd, const std::string &input)
{
	int status;
	return shell_exec(cmd, input, status);
}

// alternative way of calling shell_exec() with fewer arguments
std::string shell_exec(const std::string &cmd)
{
	return shell_exec(cmd, "");
}

/******************
 * math functions *
 ******************
*/

int rand(const int min /* = 0 */, const int max /* = RAND_MAX */)
{
	/* this is ugly but this is the correct way in C++ to get a random number from a specific range using the standard library only */
	return min + std::rand() / (RAND_MAX / (max - min + 1) + 1);
}

/********************
 * string functions *
 ********************
*/

bool is_int(const std::string &str)
{
	if(str.find_first_not_of("0123456789") == std::string::npos)
	{
		return true;
	}
	return false;
}

std::string ltrim(const std::string &str, const std::string &whitespace /* = " \n\r\t\f\v" */)
{
	size_t start = str.find_first_not_of(whitespace);
	if(start == std::string::npos)
	{
		return "";
	}
	return str.substr(start);
}

std::string rtrim(const std::string &str, const std::string &whitespace /* = " \n\r\t\f\v" */)
{
	size_t end = str.find_last_not_of(whitespace);
	if(end == std::string::npos)
	{
		return "";
	}
	return str.substr(0, end + 1);
}

std::string trim(const std::string &str, const std::string &whitespace /* = " \n\r\t\f\v" */)
{
	return rtrim(ltrim(str, whitespace), whitespace);
}

// C++24 supposedly will have it's own std::string.contains(). In the meantime, this provides str_contains() from PHP 8
bool str_contains(const std::string &haystack, const std::string &needle)
{
	if(haystack.find(needle) != std::string::npos)
	{
		return true;
	}
	return false;
}

std::string strrev(std::string str)
{
	reverse(str.begin(), str.end());
	return str;
}

std::string str_rot13(const std::string &str)
{
	std::string result;
	for(size_t i = 0; i < str.size(); i++)
	{
		if(isalpha(str[i]))
		{
			if((tolower(str[i]) - 'a') < 14)
			{
				result.append(1, str[i] + 13);
			}
			else
			{
				result.append(1, str[i] - 13);
			}
		}
		else
		{
			result.append(1, str[i]);
		}
	}
	return result;
}

std::string str_repeat(const std::string &str, const size_t times)
{
	size_t loop;
	std::string result;

	loop = 0;
	while(loop < times)
	{
		result.append(str);
		loop++;
	}
	return result;
}

std::string str_pad(const std::string &input, size_t length, const std::string pad_str /* = " " */, size_t pad_type /* = STR_PAD_RIGHT */)
{
	std::string result;

	// no length available to do any padding. return input untouched.
	if(input.size() >= length) { return input; }

	if(pad_type == STR_PAD_RIGHT || pad_type == STR_PAD_LEFT || pad_type == STR_PAD_BOTH)
	{
		for(size_t i = input.size(); i < length; i = i + pad_str.size())
		{
			if(input.size() + result.size() + pad_str.size() < length)
			{
				result.append(pad_str);
			}
			else
			{
				result.append(pad_str.substr(0, length - i));
			}
		}
	}

	if(pad_type == STR_PAD_RIGHT)
	{
		result.insert(0, input);
	}
	if(pad_type == STR_PAD_LEFT)
	{
		result.append(input);
	}
	if(pad_type == STR_PAD_BOTH)
	{
		size_t half = (length - input.size()) / 2;
		std::string tmp = result;
		result = "";

		// this prefers padding right, just like PHP does
		result.append(tmp.substr(0, half));
		result.append(input);
		result.append(tmp.substr(half, std::string::npos));
	}
	return result;
}

std::string ucfirst(const std::string &str)
{
	std::string result = str;
	result[0]=toupper(result[0]);
	return result;
}

std::string lcfirst(const std::string &str)
{
	std::string result = str;
	result[0]=tolower(result[0]);
	return result;
}

std::string strtoupper(const std::string &str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::toupper(c); });
	return result;
}

std::string strtolower(const std::string &str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::tolower(c); });
	return result;
}

// this depends on array behavior, so it's down here
std::string str_replace(const std::string &search, const std::string &replace, const std::string &subject)
{
	std::vector<std::string> vect;
	vect = explode(search, subject);
	return implode(replace, vect);
}

/******************
* array functions *
*******************
*/

namespace {

// explode implementation helper. we don't want people caling this, thus the anonymous namespace.
// given a search delimited string subject, return the segment requested.
// eg:  _explode_segment("||", "this||is||a||string", result, 2) // result == "a"
// if segment was found return true, if segment was not found return false
bool _explode_segment(const std::string &search, const std::string &subject, std::string &result, size_t segment = 0)
{
	//std::string result;
	std::string looptest;
	size_t found_segment;

	// segment 0 is a special case because we won't find search immediately before it.
	// instead, just return from the start of string until we encounter search, not including search itself.
	if(segment == 0)
	{
		found_segment = subject.find(search, 0);
		result = subject.substr(0, found_segment);
		//std::cout << "found segment: 0 " << " result: " << result << std::endl;
		// we have a good result
		return true;
	}

	found_segment = 0;
	result = "";

	// loop until we find the segment we want
	for(size_t i = 0; i < subject.size(); i++)
	{
		// have we found a segment?
		if(subject.substr(i, search.size()) == search)
		{
			found_segment++;
			result = subject.substr(i + search.size(), std::string::npos);
			//std::cout << "found segment: [" << result << "] | ";
			result = result.substr(0, result.find(search, 0));
			//std::cout << "found segment: [" << result << "]" << std::endl;
			if(found_segment == segment)
			{
				// we have a good result.
				return true;
			}
			i = i + search.size() - 1;
		}
	}
	// we didn't find any segments. signal to caller that result is garbage.
	return false;
}

} //end anonymous namespace

// This does not handle limit like php's explode() does.
// 1) Negative limit is undefined behavior.
// 2) If limit is set and positive, the returned array will contain a maximum of limit elements. The last element will NOT contain the rest of string.
std::vector<std::string> explode(std::string const &search, std::string const &subject, int limit /* = INT_MAX */)
{
	std::vector<std::string> result_v;
	std::string result;
	int loop = 0;
	bool found = false;
	if(limit == 0)
	{
		// PHP: if the limit parameter is zero, then this is treated as 1.
		limit = 1;
	}
	while(1)
	{
		found = _explode_segment(search, subject, result, loop);
		if(found == false || limit == loop)
		{
			// no more segments to get
			break;
		}
		result_v.push_back(result);
		loop++;
	}
	return result_v;
}

std::vector<std::string> str_split(const std::string &str, size_t length /* = 1 */)
{
	std::vector<std::string> result_v;
	std::string result;
	size_t loop;

	if(length > str.size())
	{
		length = str.size();
	}

	loop = 0;
	while(loop < str.size())
	{
		result = str.substr(loop, length);
		result_v.push_back(result);
		loop = loop + length;
	}

	return result_v;
}

std::string implode(const std::string &separator, const std::vector<std::string> &array)
{
	std::string result;
	for(size_t i = 0; i < array.size(); i++)
	{
		result.append(array[i]);
		if(i < array.size() - 1)
		{
			// this prevents a tailing seperator going on the end during the last loop cycle
			result.append(separator);
		}
	}
	return result;
}

/*********************
 * Network functions *
 *********************
*/

// performs a dns lookup on input and returns an IP address
// returns input unmodified on failure.
std::string __gethostbyname(const std::string &input)
{
	struct hostent *h = gethostbyname(input.c_str());
	struct in_addr a;
	if(h == NULL)
	{
		//herror("gethostbyname");
		return input;
	}
	if(h->h_addrtype == AF_INET)
	{
		//printf("name: %s\n", h->h_name);
		//while(*h->h_aliases) printf("alias: %s\n", *h->h_aliases++);
		while(*h->h_addr_list)
		{
			memmove((char *) &a, *h->h_addr_list++, sizeof(a));
			//printf("address: %s\n", inet_ntoa(a));
			return inet_ntoa(a);
		}
	}
	return input;
}

// similar to file_get_contents, but for the network
// returns empty string on failure
std::string url_get_contents(const std::string &input)
{
	if(input.substr(0, 7) == "http://" || input.substr(0, 8) == "https://")
	{
		http fetch = http_fetch(input);

		if(fetch.status != 200)
		{
			return "";
		}
		return fetch.body;
	}
	return "";
}

// open tcp socket connection to hostname on port
// returns a socket fd, or -1 on failure
int sopen(const std::string &hostname, int port)
{
	int sock;
	struct sockaddr_in serv_addr;
	struct timeval timeout;
	sock = 0;

	std::string host = __gethostbyname(hostname);
	if(host == "")
	{
		std::cerr << "hostname lookup failure!" << std::endl;
		return -1;
	}
	//std::cerr << "gethostbyname: " << host << std::endl;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		std::cerr << "socket creation failed!" << std::endl;
		return -1;
	}

	// timeout after 10 minutes
	timeout.tv_sec = 600;
	timeout.tv_usec = 0;

	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0
	|| setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		std::cerr << "setsockopt failed!" << std::endl;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0)
	{
		std::cerr << "invalid address!" << std::endl;
		return -1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "connection failed!" << std::endl;
		return -1;
	}
	return sock;
}

// read a line from socket and return it.
std::string read_line(int sock)
{
	char tmp[1];
	char buf[8192];

	for(int i = 0; i < 8192; i++)
	{
		if(read(sock, tmp, 1) != 1)
		{
			std::cerr << "socket failure. read failed." << std::endl;
			return "";
		}
		buf[i] = tmp[0];
		if(tmp[0] == '\n' || tmp[0] == '\r')
		{
			// do not break if the first byte we read is a newline character.
			// this prevents returning an empty string for no good reason.
			if(i > 0)
			{
				buf[i] = '\0';
				break;
			}
		}
	}
	std::string result = buf;
	return trim(result);
}

// returns true on success, false on failure
bool write_line(int sock, const std::string &line)
{
	if(write(sock, line.c_str(), line.length()) != (ssize_t)line.length())
	{
		std::cerr << "socket failure. write failed." << std::endl;
		return false;
	}
	if(write(sock, "\r\n", 2) != 2)
	{
		std::cerr << "socket failure. write failed." << std::endl;
		return false;
	}
	return true;
}

// close socket
void __close(int sock)
{
	close(sock);
}

/*****************
 * TLS functions *
 *****************
*/

// this keeps a mapping of tls sockets to the tls session state
// which allows the external api for this to be the same as for the non-ssl function versions
// this also properly handles having multiple ssl sockets open at the same time

// pack the tls structs into a common struct
struct sslstruct {
	struct tls_config *tlscfg = NULL;
	struct tls *tlsctx = NULL;
};

// create a mapping between the tls sockets and our common ssl struct
std::map<int, struct sslstruct> sslmap;

// open a tls connection to hostname on port
// set verify false to disable all tls validation and certificate checking
// returns true on success or false on failure
int ssl_sopen(const std::string &hostname, int port, bool verify)
{
	int tlssock = sopen(hostname, port);
	if(tlssock == -1)
	{
		std::cerr << "error opening socket for ssl." << std::endl;
		return -1;
	}
	struct sslstruct ssl;
	ssl.tlscfg = NULL;
	ssl.tlsctx = NULL;
	if(tls_init() != 0)
	{
		std::cerr << "tls_init() failed!" << std::endl;
		return -1;
	}
	if((ssl.tlscfg = tls_config_new()) == NULL)
	{
		std::cerr << "tls_config_new() failed!" << std::endl;
		return -1;
	}
	if((ssl.tlsctx = tls_client()) == NULL)
	{
		std::cerr << "tls_client() failed!" << std::endl;
		return -1;
	}
	if(tls_configure(ssl.tlsctx, ssl.tlscfg) != 0)
	{
		std::cerr << "tls_configure(): " << tls_error(ssl.tlsctx) << std::endl;
		return -1;
	}
	if(verify == false)
	{
		// disable certificate & oscp validation
		tls_config_insecure_noverifycert(ssl.tlscfg);

		// disable server name validation
		tls_config_insecure_noverifyname(ssl.tlscfg);

		// disable checking certificate expiration
		tls_config_insecure_noverifytime(ssl.tlscfg);
	}
	// if(tls_connect(tlsctx, hostname.c_str(), std::to_string(port).c_str()) != 0)
	if(tls_connect_socket(ssl.tlsctx, tlssock, hostname.c_str()) != 0)
	{
		std::cerr << "tls_connect(): " << tls_error(ssl.tlsctx) << std::endl;
		return -1;
	}
	if(tls_handshake(ssl.tlsctx) != 0)
	{
		std::cerr << "tls_handshake(): " << tls_error(ssl.tlsctx) << std::endl;
		return -1;
	}
	sslmap[tlssock] = ssl;
	return tlssock;
}

// read a line from tls socket and return it.
std::string ssl_read_line(int tlssock)
{
	char tmp[1];
	char buf[8192];
	ssize_t state;
	int i = 0;

	struct sslstruct ssl;
	ssl = sslmap[tlssock];

	while(i < 8192)
	{
		state = tls_read(ssl.tlsctx, tmp, 1);
		if(state == -1)
		{
			std::cerr << "socket failure. read failed." << std::endl;
			exit(EXIT_FAILURE);
		}
		if(state == TLS_WANT_POLLIN)
		{
			usleep(100000);
			continue;
		}
		if(state != 1)
		{
			std::cerr << "tls_read() failure." << std::endl;
			exit(EXIT_FAILURE);
		}

		buf[i] = tmp[0];
		if(tmp[0] == '\n' || tmp[0] == '\r')
		{
			// do not break if the first byte we read is a newline character.
			// this prevents returning an empty string for no good reason.
			if(i > 0)
			{
				buf[i] = '\0';
				break;
			}
		}
		i++;
	}
	std::string result = buf;
	return trim(result);
}

// returns true on success, false on failure
bool ssl_write_line(int tlssock, const std::string &line)
{
	struct sslstruct ssl;
	ssl = sslmap[tlssock];

	if(tls_write(ssl.tlsctx, line.c_str(), line.length()) != (ssize_t)line.length())
	{
		std::cerr << "ssl socket failure. write failed." << std::endl;
		return false;
	}
	if(tls_write(ssl.tlsctx, "\r\n", 2) != 2)
	{
		std::cerr << "ssl socket failure. write failed." << std::endl;
		return false;
	}
	return true;
}

void ssl_close(int tlssock)
{
	struct sslstruct ssl;
	ssl = sslmap[tlssock];

	if(tls_close(ssl.tlsctx) != 0)
	{
		std::cerr << "tls_close(): " << tls_error(ssl.tlsctx) << std::endl;
	}
	tls_free(ssl.tlsctx);
	tls_config_free(ssl.tlscfg);
	sslmap.erase(tlssock);
	close(tlssock);
}

/************************
 * filesystem functions *
 ************************
*/

// returns true if file was successfully removed (or never existed), false otherwise
bool __unlink(const std::string &file)
{
	if(file_exists(file) == true)
	{
		std::remove(file.c_str());
	}
	if(file_exists(file) == true)
	{
		return false;
	}
	return true;
}

std::string file_get_contents(const std::string &str)
{
	std::ifstream file;
	file.open(str, std::ios::in | std::ios::binary);
	if(!file.is_open())
	{
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

// this should return the number of bytes written, but it doesn't.
// instead we return 0 if nothing was written, or strlen(data) if we wrote anything
size_t file_put_contents(const std::string &str, const std::string &data, size_t flag /* = 0 */)
{
	std::ofstream file;
	if(flag == 0)
	{
		std::remove(str.c_str());
		file.open(str, std::ios::out | std::ios::binary);
	}
	if(flag == FILE_APPEND)
	{
		file.open(str, std::ios::out | std::ios::binary | std::ios::app);
	}
	if(!file.is_open())
	{
		return 0;
	}
	file << data;
	file.close();
	return data.length();
}

bool file_exists(const std::string &str)
{
	if(access(str.c_str(), F_OK) == 0)
	{
		return true;
	}
	return false;
}

bool is_readable(const std::string &str)
{
	if(access(str.c_str(), R_OK) == 0)
	{
		return true;
	}
	return false;
}

bool is_writable(const std::string &str)
{
	if(access(str.c_str(), W_OK) == 0)
	{
		return true;
	}
	return false;
}

/******************
 * misc functions *
 ******************
*/

unsigned int __sleep(unsigned int seconds)
{
	return sleep(seconds);
}

} // end of namespace
#endif
