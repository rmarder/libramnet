#include "ramnet.hpp"
#include <cassert>
#include <iostream>

using namespace ramnet;

// tests for trim, ltrim, rtrim
void test_trim()
{
	std::cout << "Tesing trim() ltrim() rtrim() single space...";
	assert(trim(" test") == "test");
	assert(trim("test ") == "test");
	assert(trim(" test ") == "test");
	assert(ltrim(" test") == "test");
	assert(rtrim("test ") == "test");
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing trim() ltrim() rtrim() multiple spaces...";
	assert(trim("  test") == "test");
	assert(trim("test  ") == "test");
	assert(trim("  test  ") == "test");
	assert(ltrim("  test") == "test");
	assert(rtrim("test  ") == "test");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing trim() ltrim() rtrim() middle spaces...";
	assert(trim("  t t") == "t t");
	assert(trim("t t  ") == "t t");
	assert(trim("  t t  ") == "t t");
	assert(ltrim("  t t") == "t t");
	assert(rtrim("t t  ") == "t t");
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing ltrim() rtrim() spaces at wrong end...";
	assert(rtrim("  t t  ") == "  t t");
	assert(ltrim("  t t  ") == "t t  ");
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing trim() ltrim() rtrim() mixed whitespace...";
	assert(trim("\n\r\t t t") == "t t");
	assert(trim("t t \v\r\n\t") == "t t");
	assert(trim("\r\n\v\t t t \r\n\v\t") == "t t");
	assert(ltrim("\r\n\v\t t t \r\n\v\t") == "t t \r\n\v\t");
	assert(rtrim("\r\n\v\t t t \r\n\v\t") == "\r\n\v\t t t");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing trim() ltrim() rtrim() custom characters...";
	assert(trim("  this is a test  ", "is") == "  this is a test  ");
	assert(trim(" this is a test ", "t") == " this is a test ");
	assert(trim(" this is a test ", " t") == "his is a tes");
	assert(trim("  this is a test  ", "ish ") == "this is a test");
	assert(trim("  this is a test  ", "isht ") == "a te");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
}

void test_net()
{
	std::cout << "Tesing gethostbyname() on localhost...";
	assert(gethostbyname("localhost") != "localhost");
	std::cout << "\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing url_get_contents() on http://www.example.com...";
	assert(url_get_contents("http://www.example.com") != "");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Tesing url_get_contents() on https://www.example.com...";
	assert(url_get_contents("https://www.example.com") != "");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing sopen() on www.example.com port 80...";
	int sock = -1;
	sock = sopen("www.example.com", 80);
	assert(sock != -1);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing write_line() on www.example.com port 80...";
	assert(write_line(sock, "HEAD / HTTP/1.0\r\nHost: www.example.com\r\n") != false);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing read_line() on www.example.com port 80...";
	assert(read_line(sock) == "HTTP/1.0 200 OK");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
	close(sock);
}

void test_tls()
{
	int sock = -1;
	int sock2 = -1;
	std::cout << "Testing ssl_sopen() on www.example.com port 443...";
	sock = ssl_sopen("www.example.com", 443, true);
	assert(sock != -1);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing ssl_sopen() on www.google.com port 443...";
	sock2 = ssl_sopen("www.google.com", 443, true);
	assert(sock2 != -1);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing ssl_write_line() on www.example.com port 443...";
	assert(ssl_write_line(sock, "HEAD / HTTP/1.0\r\nHost: www.example.com\r\n") != false);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing ssl_write_line() on www.google.com port 443...";
	assert(ssl_write_line(sock2, "HEAD / HTTP/1.0\r\nHost: www.google.com\r\n") != false);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing ssl_read_line() on www.example.com port 443...";
	assert(ssl_read_line(sock) == "HTTP/1.0 200 OK");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing ssl_read_line() on www.google.com port 443...";
	assert(ssl_read_line(sock2) == "HTTP/1.0 200 OK");
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	ssl_close(sock);
	ssl_close(sock2);
}

void test_base64()
{
	std::string testdata;
	std::string encoded;
	std::string decoded;
	std::string control;

	std::cout << "Testing base64_encode...";
	control = "PCFkb2N0eXBlIGh0bWw+PGh0bWw+PGhlYWQ+PHRpdGxlPkV4YW1wbGUgRG9tYWluPC90aXRsZT48bWV0YSBjaGFyc2V0PSJ1dGYtOCIgLz48bWV0YSBodHRwLWVxdWl2PSJDb250ZW50LXR5cGUiIGNvbnRlbnQ9InRleHQvaHRtbDsgY2hhcnNldD11dGYtOCIgLz48bWV0YSBuYW1lPSJ2aWV3cG9ydCIgY29udGVudD0id2lkdGg9ZGV2aWNlLXdpZHRoLCBpbml0aWFsLXNjYWxlPTEiIC8+PHN0eWxlIHR5cGU9InRleHQvY3NzIj5ib2R5IHtiYWNrZ3JvdW5kLWNvbG9yOiAjZjBmMGYyO21hcmdpbjogMDtwYWRkaW5nOiAwO2ZvbnQtZmFtaWx5OiAtYXBwbGUtc3lzdGVtLCBzeXN0ZW0tdWksIEJsaW5rTWFjU3lzdGVtRm9udCwgIlNlZ29lIFVJIiwgIk9wZW4gU2FucyIsICJIZWx2ZXRpY2EgTmV1ZSIsIEhlbHZldGljYSwgQXJpYWwsIHNhbnMtc2VyaWY7fQ==";
	testdata = "<!doctype html><html><head><title>Example Domain</title><meta charset=\"utf-8\" /><meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /><style type=\"text/css\">body {background-color: #f0f0f2;margin: 0;padding: 0;font-family: -apple-system, system-ui, BlinkMacSystemFont, \"Segoe UI\", \"Open Sans\", \"Helvetica Neue\", Helvetica, Arial, sans-serif;}";
	assert(base64_encode(testdata) == control);
	std::cout << "\t\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing base64_decode...";
	assert(base64_decode(control) == testdata);
	std::cout << "\t\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing base64 with binary data...";
	testdata = file_get_contents("/bin/sh");
	encoded = base64_encode(testdata);
	decoded = base64_decode(encoded);
	assert(decoded == testdata);
	std::cout << "\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
}

void test_process()
{
	std::string cmd;
	std::string input;
	std::string output;
	int status;

	std::cout << "Testing shell_exec() on script ./test.sh...";
	cmd = "/bin/sh ./test.sh";
	input = "hello world";
	output = "";
	status = -1;
	output = shell_exec(cmd, input, status);
	assert(output == "hello world");
	assert(status == 5);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing shell_exec() with stdin and stdout on process tr...";
	cmd = "tr a-z A-Z";
	input = "hello world";
	output = "";
	status = -1;
	output = shell_exec(cmd, input, status);
	assert(output == "HELLO WORLD");
	assert(status == 0);
	std::cout << "\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing shell_exec() on process ping using timeout...";
	cmd = "ping localhost";
	input = "";
	output = "";
	status = -1;
	output = shell_exec(cmd, input, status, 1);
	assert(status == 0);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing shell_exec() on process that doesn't exist...";
	cmd = "strjrdthytbytdtydjdytytfyytjuyjkfyj5ejur5";
	input = "";
	output = "";
	status = -1;
	output = shell_exec(cmd, input, status);
	assert(status == 127);
	std::cout << "\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
}

void test_filesystem()
{
	std::cout << "Testing file_exists() on nonexisting file...";
	assert(file_exists("test.tmp") == false);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing is_readable() on nonexisting file...";
	assert(is_readable("test.tmp") == false);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing is_writable() on nonexisting file...";
	assert(is_writable("test.tmp") == false);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing file_put_contents() new file...";
	assert(file_put_contents("test.tmp", "test line\n") > 0);
	std::cout << "\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing file_put_contents() append file...";
	assert(file_put_contents("test.tmp", "second line\n", FILE_APPEND) > 0);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing file_get_contents() ...";
	assert(file_get_contents("test.tmp") == "test line\nsecond line\n");
	std::cout << "\t\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing file_put_contents() overwrite file...";
	assert(file_put_contents("test.tmp", "test") > 0);
	assert(file_get_contents("test.tmp") == "test");
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing file_exists() on existing file...";
	assert(file_exists("test.tmp") == true);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing is_readable() on existing file...";
	assert(is_readable("test.tmp") == true);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing is_writable() on existing file...";
	assert(is_writable("test.tmp") == true);
	std::cout << "\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing unlink() on existing file...";
	assert(unlink("test.tmp") == true);
	std::cout << "\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;

	std::cout << "Testing unlink() on nonexisting file...";
	assert(unlink("test.tmp") == true);
	std::cout << "\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
}

void test_misc()
{
	std::cout << "Testing sleep()...";
	assert(sleep(1) == 0);
	std::cout << "\t\t\t\t\t\t[\033[1;32mPASSED\033[0m]" << std::endl;
}

int main(void)
{
	test_trim();
	test_net();
	test_tls();
	test_base64();
	test_process();
	test_filesystem();
	test_misc();
	return 0;
}
