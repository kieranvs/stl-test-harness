#include "tests_unique_ptr.h"

template <typename T>
struct my_unique_ptr
{

};

int main()
{
	run_tests_unique_ptr<my_unique_ptr>();
	return 0;
}