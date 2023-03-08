#include "tests_vector.h"
#include "tests_unique_ptr.h"
#include "tests_shared_ptr.h"

template <typename T>
struct my_vector
{

};

template <typename T>
struct my_unique_ptr
{

};

template <typename T>
struct my_shared_ptr
{

};

int main()
{
	tests_vector::run<my_vector>();
	tests_unique_ptr::run<my_unique_ptr>();
	tests_shared_ptr::run<my_shared_ptr>();
	return 0;
}