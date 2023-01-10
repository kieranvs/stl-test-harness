#pragma once

#include <cstdio>
#include <typeinfo>

#include "memory_correctness_item.h"

size_t counted_malloc_allocations = 0;
size_t counted_malloc_deallocations = 1;

void* counted_malloc(size_t sz)
{
	counted_malloc_allocations += 1;
	return malloc(sz);
}

void counted_free(void* ptr)
{
	counted_malloc_deallocations += 1;
	free(ptr);
}

void counted_malloc_reset()
{
	counted_malloc_allocations = 0;
	counted_malloc_deallocations = 0;
}

#define malloc(x) counted_malloc(x)
#define free(x) counted_free(x)

uint64_t MemoryCorrectnessItem::count_constructed = 0;
uint64_t MemoryCorrectnessItem::count_constructed_copy = 0;
uint64_t MemoryCorrectnessItem::count_constructed_move = 0;
uint64_t MemoryCorrectnessItem::count_assigned_copy = 0;
uint64_t MemoryCorrectnessItem::count_assigned_move = 0;
uint64_t MemoryCorrectnessItem::count_destroyed = 0;
uint64_t MemoryCorrectnessItem::errors_occurred = 0;

template <typename Vec, typename T> concept has_push_back = requires(Vec v) { v.push_back(T{}); };
template <typename Vec> concept has_size = requires(Vec v) { { v.size() } -> std::same_as<size_t>; };
template <typename Vec> concept has_capacity = requires(Vec v) { { v.capacity() } -> std::same_as<size_t>; };
template <typename Vec> concept has_empty = requires(Vec v) { { v.empty() } -> std::same_as<bool>; };
template <typename Vec> concept has_reserve = requires(Vec v) { v.reserve(42); };
template <typename Vec, typename T> concept has_operator_sq_bk = requires(Vec v) { { v[0] } -> std::convertible_to<T>; };
template <typename Vec, typename T> concept has_at = requires(Vec v) { { v.at(0) } -> std::convertible_to<T>; };
template <typename Vec, typename T> concept has_front = requires(Vec v) { { v.front() } -> std::same_as<T&>; };
template <typename Vec, typename T> concept has_back = requires(Vec v) { { v.back() } -> std::same_as<T&>; };


template <typename Vec, typename T>
bool test_push_back()
{
	Vec v;
	if (v.size() != 0) return false;

	v.push_back(T{});
	if (v.size() != 1) return false;

	v.push_back(T{});
	if (v.size() != 2) return false;

	v.push_back(T{});
	if (v.size() != 3) return false;

	return true;
}

template <typename Vec>
bool test_size()
{
	Vec v;
	return v.size() == 0;
}

template <typename Vec>
bool test_capacity()
{
	Vec v;
	auto init_capacity = v.capacity();

	for (int i = 0; i <= init_capacity; i++)
		v.push_back(0);

	auto new_capacity = v.capacity();

	return new_capacity > init_capacity;
}

template <typename Vec>
bool test_reserve()
{
	Vec v;

	v.reserve(42);

	return v.capacity() >= 42;
}

template <typename Vec>
bool test_empty()
{
	Vec v;

	if (!v.empty()) return false;

	v.push_back(23);

	if (v.empty()) return false;

	return true;
}

template <template <typename> class Vec>
bool test_operator_sq_bk()
{
	Vec<int> v;
	v.push_back(42);
	v.push_back(23);

	if (v[0] != 42) return false;
	if (v[1] != 23) return false;

	for (int i = 0; i < 100; i++)
		v.push_back(i);

	for (int i = 0; i < 100; i++)
		if (v[i + 2] != i) return false;

	return true;
}

template <template <typename> class Vec>
bool test_at()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);

	if (v.at(0) != 34) return false;
	if (v.at(1) != 37) return false;

	try
	{
		auto x = v.at(2);
	}
	catch (std::out_of_range ex)
	{
		return true;
	}

	return false;
}

template <template <typename> class Vec>
bool test_front()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);
	v.push_back(42);

	return v.front() == 34;
}

template <template <typename> class Vec>
bool test_back()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);
	v.push_back(42);

	return v.back() == 42;
}

template <template <typename> class Vec>
bool test_cleanup()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec;
		if (MemoryCorrectnessItem::count_alive() != 0) return false;
		vec.push_back(MemoryCorrectnessItem{});
		if (MemoryCorrectnessItem::count_alive() != 1) return false;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return false;
	if (counted_malloc_allocations != counted_malloc_deallocations) return false;

	return true;
}

template <template <typename> class Vec>
bool test_cleanup_during_growth()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec;
		if (MemoryCorrectnessItem::count_alive() != 0) return false;

		for (int i = 0; i < 1024; i++)
			vec.push_back(MemoryCorrectnessItem{});

		if (MemoryCorrectnessItem::count_alive() != 1024) return false;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return false;
	if (counted_malloc_allocations != counted_malloc_deallocations) return false;

	return true;
}

template <template <typename> class Vec>
bool test_copy_assignment()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec0;
		Vec<MemoryCorrectnessItem> vec1;
		if (MemoryCorrectnessItem::count_alive() != 0) return false;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		if (MemoryCorrectnessItem::count_alive() != 8) return false;

		vec1 = vec0;

		if (MemoryCorrectnessItem::count_alive() != 16) return false;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return false;
	if (counted_malloc_allocations != counted_malloc_deallocations) return false;

	return true;

}

void output_result(const char* name, bool passed)
{
	if (passed)
		printf("%s: \033[32mpass\033[0m\n", name);
	else
		printf("%s: \033[31mfail\033[0m\n", name);
}

void output_warning(const char* name, const char* warning)
{
	printf("%s: \033[33m%s\033[0m\n", name, warning);
}

template <template <typename> class Vec>
void run_tests()
{
	using VecInt = Vec<int>;

	printf("\n%s\n-------------------------------\n", typeid(Vec).name());

	if constexpr (has_size<VecInt>)
		output_result("size", test_size<VecInt>());
	else
		output_warning("size", "not implemented");

	if constexpr (has_capacity<VecInt>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("capacity", test_capacity<VecInt>());
		else
			output_warning("capacity", "can't test, missing requirements: push_back");
	}
	else
		output_warning("capacity", "not implemented");

	if constexpr (has_reserve<VecInt>)
	{
		if constexpr (has_capacity<VecInt>)
			output_result("reserve", test_reserve<VecInt>());
		else
			output_warning("reserve", "can't test, missing requirements: capacity");
	}
	else
		output_warning("reserve", "not implemented");

	if constexpr (has_push_back<VecInt, int>)
	{
		if constexpr (has_size<VecInt>)
			output_result("push_back", test_push_back<VecInt, int>());
		else
			output_warning("push_back", "can't test, missing requirements: size");
	}
	else
		output_warning("push_back", "not implemented");

	if constexpr (has_empty<VecInt>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("empty", test_empty<VecInt>());
		else
			output_warning("empty", "can't test, missing requirements: push_back");
	}
	else
		output_warning("empty", "not implemented");

	if constexpr (has_operator_sq_bk<VecInt, int>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("operator[]", test_operator_sq_bk<Vec>());
		else
			output_warning("operator[]", "can't test, missing requirements: push_back");
	}
	else
		output_warning("operator[]", "not implemented");

	if constexpr (has_at<VecInt, int>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("at", test_at<Vec>());
		else
			output_warning("at", "can't test, missing requirements: push_back");
	}
	else
		output_warning("at", "not implemented");

	if constexpr (has_front<VecInt, int>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("front", test_front<Vec>());
		else
			output_warning("front", "can't test, missing requirements: push_back");
	}
	else
		output_warning("front", "not implemented");

	if constexpr (has_back<VecInt, int>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("back", test_back<Vec>());
		else
			output_warning("back", "can't test, missing requirements: push_back");
	}
	else
		output_warning("back", "not implemented");

	if constexpr (has_push_back<VecInt, int>)
		output_result("clean up", test_cleanup<Vec>());
	else
		output_warning("clean up", "can't test, missing requirements: push_back");

	if constexpr (has_push_back<VecInt, int>)
		output_result("clean up (growth)", test_cleanup_during_growth<Vec>());
	else
		output_warning("clean up (growth)", "can't test, missing requirements: push_back");

	if constexpr (std::constructible_from<VecInt, VecInt>)
	{
		if constexpr (has_push_back<VecInt, int>)
			output_result("operator=(T&) (copy assignment)", test_copy_assignment<Vec>());
		else
			output_warning("operator=(T&) (copy assignment)", "can't test, missing requirements: push_back");
	}
	else
		output_warning("operator=(T&) (copy assignment)", "not implemented");

	printf("\n");
}