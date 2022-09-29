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
}

int main(void)
{
	test_trim();
	test_net();
	return 0;
}
