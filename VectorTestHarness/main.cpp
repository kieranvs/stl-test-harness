#include "tests_vector.h"
#include "tests_unique_ptr.h"

template <typename T>
struct my_vec
{
};

int main()
{
	run_tests<my_vec>();
	return 0;
}