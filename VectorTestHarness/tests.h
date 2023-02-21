#pragma once

#include <cstdio>
#include <typeinfo>
#include <stdexcept>

#include "memory_correctness_item.h"
#include "counted_malloc.h"

template <typename Vec, typename T> concept has_push_back = requires(Vec v) { v.push_back(T{}); };
template <typename Vec> concept has_size = requires(Vec v) { { v.size() } -> std::same_as<size_t>; };
template <typename Vec> concept has_capacity = requires(Vec v) { { v.capacity() } -> std::same_as<size_t>; };
template <typename Vec> concept has_empty = requires(Vec v) { { v.empty() } -> std::same_as<bool>; };
template <typename Vec> concept has_reserve = requires(Vec v) { v.reserve(42); };
template <typename Vec> concept has_resize = requires(Vec v) { v.resize(42); };
template <typename Vec> concept has_clear = requires(Vec v) { v.clear(); };
template <typename Vec, typename T> concept has_operator_sq_bk = requires(Vec v) { { v[0] } -> std::convertible_to<T>; };
template <typename Vec, typename T> concept has_at = requires(Vec v) { { v.at(0) } -> std::convertible_to<T>; };
template <typename Vec, typename T> concept has_front = requires(Vec v) { { v.front() } -> std::same_as<T&>; };
template <typename Vec, typename T> concept has_back = requires(Vec v) { { v.back() } -> std::same_as<T&>; };

enum class TestResult
{
	IncorrectResults,
	LeaksMemory,
	IncorrectObjectHandling,
	SuboptimalObjectHandling,
	Pass
};

template <template <typename> class Vec>
TestResult test_push_back()
{
	{
		Vec<int> v;
		if (v.size() != 0) return TestResult::IncorrectResults;

		v.push_back(0);
		if (v.size() != 1) return TestResult::IncorrectResults;

		v.push_back(0);
		if (v.size() != 2) return TestResult::IncorrectResults;

		v.push_back(0);
		if (v.size() != 3) return TestResult::IncorrectResults;
	}

	{
		MemoryCorrectnessItem::reset();
		counted_malloc_reset();	
		
		Vec<MemoryCorrectnessItem> v;
		v.push_back(MemoryCorrectnessItem{});

		auto allocs_before = counted_malloc_allocations;
		int count_made = 1;

		while (counted_malloc_allocations == allocs_before)
		{
			v.push_back(MemoryCorrectnessItem{});
			count_made += 1;

			if (count_made > 10000)
				break;
		}

		if (counted_malloc_allocations - counted_malloc_deallocations != 1)
			return TestResult::LeaksMemory;

		if (MemoryCorrectnessItem::count_alive() != count_made)
			return TestResult::IncorrectObjectHandling;
			
		if (MemoryCorrectnessItem::count_constructed_copy > count_made)
			return TestResult::SuboptimalObjectHandling;
	}

	return TestResult::Pass;
}

template <typename Vec>
TestResult test_size()
{
	Vec v;
	return v.size() == 0 ? TestResult::Pass : TestResult::IncorrectResults;
}

template <typename Vec>
TestResult test_capacity()
{
	Vec v;
	auto init_capacity = v.capacity();

	for (int i = 0; i <= init_capacity; i++)
		v.push_back(0);

	auto new_capacity = v.capacity();

	return new_capacity > init_capacity ? TestResult::Pass : TestResult::IncorrectResults;
}

template <template <typename> class Vec>
TestResult test_reserve()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	Vec<MemoryCorrectnessItem> v;
	v.push_back(MemoryCorrectnessItem{});
	v.push_back(MemoryCorrectnessItem{});

	auto count_alive_pre = MemoryCorrectnessItem::count_alive();
	v.reserve(42);

	if (v.capacity() < 42) return TestResult::IncorrectResults;

	if (counted_malloc_allocations - counted_malloc_deallocations != 1)
		return TestResult::LeaksMemory;

	if (MemoryCorrectnessItem::count_alive() != count_alive_pre)
		return TestResult::IncorrectObjectHandling;

	if (MemoryCorrectnessItem::count_constructed_copy > 2)
		return TestResult::SuboptimalObjectHandling;		

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_resize()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	Vec<MemoryCorrectnessItem> v;
	v.push_back(MemoryCorrectnessItem{});
	v.push_back(MemoryCorrectnessItem{});

	v.resize(42);

	if (v.size() != 42) return TestResult::IncorrectResults;

	if (counted_malloc_allocations - counted_malloc_deallocations != 1)
		return TestResult::LeaksMemory;

	if (MemoryCorrectnessItem::count_alive() != 42)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_clear()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	Vec<MemoryCorrectnessItem> v;
	v.push_back(MemoryCorrectnessItem{});
	v.push_back(MemoryCorrectnessItem{});

	auto cap_pre = v.capacity();
	v.clear();

	if (v.size() != 0) return TestResult::IncorrectResults;
	if (v.capacity() != cap_pre) return TestResult::IncorrectResults;

	if (counted_malloc_allocations - counted_malloc_deallocations > 1)
		return TestResult::LeaksMemory;

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <typename Vec>
TestResult test_empty()
{
	Vec v;

	if (!v.empty()) return TestResult::IncorrectResults;

	v.push_back(23);

	if (v.empty()) return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_operator_sq_bk()
{
	Vec<int> v;
	v.push_back(42);
	v.push_back(23);

	if (v[0] != 42) return TestResult::IncorrectResults;
	if (v[1] != 23) return TestResult::IncorrectResults;

	for (int i = 0; i < 100; i++)
		v.push_back(i);

	for (int i = 0; i < 100; i++)
		if (v[i + 2] != i) return TestResult::IncorrectResults;

	// Make sure we're returning it by reference
	auto&& x = v[0];
	x = 69;
	if (v[0] != 69) return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_at()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);

	if (v.at(0) != 34) return TestResult::IncorrectResults;
	if (v.at(1) != 37) return TestResult::IncorrectResults;

	// Make sure we're returning it by reference
	auto&& x = v[0];
	x = 69;
	if (v[0] != 69) return TestResult::IncorrectResults;

	try
	{
		auto x = v.at(2);
	}
	catch (std::out_of_range ex)
	{
		return TestResult::Pass;
	}

	return TestResult::IncorrectResults;
}

template <template <typename> class Vec>
TestResult test_front()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);
	v.push_back(42);

	if (v.front() != 34) return TestResult::IncorrectResults;

	// Make sure we're returning it by reference
	auto&& x = v.front();
	x = 69;
	if (v.front() != 69) return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_back()
{
	Vec<int> v;

	v.push_back(34);
	v.push_back(37);
	v.push_back(42);

	if (v.back() != 42) return TestResult::IncorrectResults;

	// Make sure we're returning it by reference
	auto&& x = v.back();
	x = 69;
	if (v.back() != 69) return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_destructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec;
		vec.push_back(MemoryCorrectnessItem{});
	}

	if (counted_malloc_allocations != counted_malloc_deallocations) return TestResult::LeaksMemory;
	if (MemoryCorrectnessItem::count_alive() != 0) return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

// template <template <typename> class Vec>
// bool test_cleanup_during_growth()
// {
// 	MemoryCorrectnessItem::reset();
// 	counted_malloc_reset();

// 	{
// 		Vec<MemoryCorrectnessItem> vec;
// 		if (MemoryCorrectnessItem::count_alive() != 0) return false;

// 		for (int i = 0; i < 1024; i++)
// 			vec.push_back(MemoryCorrectnessItem{});

// 		if (MemoryCorrectnessItem::count_alive() != 1024) return false;
// 	}

// 	if (MemoryCorrectnessItem::count_alive() != 0) return false;
// 	if (counted_malloc_allocations != counted_malloc_deallocations) return false;

// 	return true;
// }

template <template <typename> class Vec>
TestResult test_copy_construct()
{
	{
		Vec<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		Vec<int> v2 = v;
		
		if (v2.size() != 3) return TestResult::IncorrectResults;

		if (v2[0] != 1) return TestResult::IncorrectResults;
		if (v2[1] != 2) return TestResult::IncorrectResults;
		if (v2[2] != 3) return TestResult::IncorrectResults;
	}

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec0;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		auto vec1 = vec0;

		if (MemoryCorrectnessItem::count_alive() != 16) return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return TestResult::IncorrectObjectHandling;
	if (counted_malloc_allocations != counted_malloc_deallocations) return TestResult::LeaksMemory;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_move_construct()
{
	{
		Vec<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		Vec<int> v2 = std::move(v);
		
		if (v2.size() != 3) return TestResult::IncorrectResults;

		if (v2[0] != 1) return TestResult::IncorrectResults;
		if (v2[1] != 2) return TestResult::IncorrectResults;
		if (v2[2] != 3) return TestResult::IncorrectResults;
	}

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec0;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		auto vec1 = std::move(vec0);

		if (MemoryCorrectnessItem::count_alive() != 8) return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return TestResult::IncorrectObjectHandling;
	if (counted_malloc_allocations != counted_malloc_deallocations) return TestResult::LeaksMemory;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_copy_assignment()
{
	{
		Vec<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		Vec<int> v2;
		v2 = v;
		
		if (v2.size() != 3) return TestResult::IncorrectResults;

		if (v2[0] != 1) return TestResult::IncorrectResults;
		if (v2[1] != 2) return TestResult::IncorrectResults;
		if (v2[2] != 3) return TestResult::IncorrectResults;
	}

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec0;
		Vec<MemoryCorrectnessItem> vec1;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		vec1 = vec0;

		if (MemoryCorrectnessItem::count_alive() != 16) return TestResult::IncorrectObjectHandling;
	}

	{
		Vec<MemoryCorrectnessItem> vec0;
		Vec<MemoryCorrectnessItem> vec1;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		for (int i = 0; i < 3; i++)
			vec1.push_back(MemoryCorrectnessItem{});

		vec1 = vec0;

		if (MemoryCorrectnessItem::count_alive() != 16) return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return TestResult::IncorrectObjectHandling;
	if (counted_malloc_allocations != counted_malloc_deallocations) return TestResult::LeaksMemory;

	return TestResult::Pass;
}

template <template <typename> class Vec>
TestResult test_move_assignment()
{
	{
		Vec<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		Vec<int> v2;
		v2 = std::move(v);
		
		if (v2.size() != 3) return TestResult::IncorrectResults;

		if (v2[0] != 1) return TestResult::IncorrectResults;
		if (v2[1] != 2) return TestResult::IncorrectResults;
		if (v2[2] != 3) return TestResult::IncorrectResults;
	}

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		Vec<MemoryCorrectnessItem> vec0;
		Vec<MemoryCorrectnessItem> vec1;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		vec1 = std::move(vec0);

		if (MemoryCorrectnessItem::count_alive() != 8) return TestResult::IncorrectObjectHandling;
	}

	{
		Vec<MemoryCorrectnessItem> vec0;
		Vec<MemoryCorrectnessItem> vec1;

		for (int i = 0; i < 8; i++)
			vec0.push_back(MemoryCorrectnessItem{});

		for (int i = 0; i < 3; i++)
			vec1.push_back(MemoryCorrectnessItem{});

		vec1 = std::move(vec0);

		if (MemoryCorrectnessItem::count_alive() != 8) return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0) return TestResult::IncorrectObjectHandling;
	if (counted_malloc_allocations != counted_malloc_deallocations) return TestResult::LeaksMemory;

	return TestResult::Pass;
}

void output_result(const char* name, TestResult result)
{
	if (result == TestResult::Pass)
		printf("  %s: \033[32mpass\033[0m\n", name);
	else if (result == TestResult::SuboptimalObjectHandling)
		printf("  %s: \033[32mpass, suboptimal copies/moves\033[0m\n", name);
	else if (result == TestResult::IncorrectObjectHandling)
		printf("  %s: \033[33mincorrect object handling\033[0m\n", name);
	else if (result == TestResult::LeaksMemory)
		printf("  %s: \033[33mleaks memory\033[0m\n", name);
	else if (result == TestResult::IncorrectResults)
		printf("  %s: \033[31mfail\033[0m\n", name);
}

void output_warning(const char* name, const char* warning)
{
	printf("  %s: \033[33m%s\033[0m\n", name, warning);
}

template <template <typename> class Vec>
void run_tests()
{
	using VecInt = Vec<int>;

	printf("\n%s\n-------------------------------\n", typeid(Vec).name());

	printf("Class methods:\n");

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
		if constexpr (has_capacity<VecInt> && has_push_back<VecInt, int>)
			output_result("reserve", test_reserve<Vec>());
		else
			output_warning("reserve", "can't test, missing requirements: push_back, capacity");
	}
	else
		output_warning("reserve", "not implemented");

	if constexpr (has_resize<VecInt>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt>)
			output_result("resize", test_resize<Vec>());
		else
			output_warning("resize", "can't test, missing requirements: push_back, size");
	}
	else
		output_warning("resize", "not implemented");

	if constexpr (has_push_back<VecInt, int>)
	{
		if constexpr (has_size<VecInt>)
			output_result("push_back", test_push_back<Vec>());
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

	if constexpr (has_clear<VecInt>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt> && has_capacity<VecInt>)
			output_result("clear", test_clear<Vec>());
		else
			output_warning("clear", "can't test, missing requirements: push_back, size, capacity");
	}
	else
		output_warning("clear", "not implemented");

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

	if constexpr (std::constructible_from<VecInt, const VecInt&>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt> && has_operator_sq_bk<VecInt, int>)
			output_result("(constructor) (copy)", test_copy_construct<Vec>());
		else
			output_warning("(constructor) (copy)", "can't test, missing requirements: push_back, size, operator[]");
	}
	else
		output_warning("(constructor) (copy)", "not implemented");

	if constexpr (std::constructible_from<VecInt, VecInt&&>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt> && has_operator_sq_bk<VecInt, int>)
			output_result("(constructor) (move)", test_move_construct<Vec>());
		else
			output_warning("(constructor) (move)", "can't test, missing requirements: push_back, size, operator[]");
	}
	else
		output_warning("(constructor) (move)", "not implemented");

	if constexpr (std::is_assignable_v<VecInt, const VecInt&>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt> && has_operator_sq_bk<VecInt, int>)
			output_result("operator=(T&) (copy assignment)", test_copy_assignment<Vec>());
		else
			output_warning("operator=(T&) (copy assignment)", "can't test, missing requirements: push_back, size, operator[]");
	}
	else
		output_warning("operator=(T&) (copy assignment)", "not implemented");

	if constexpr (std::is_assignable_v<VecInt, VecInt&&>)
	{
		if constexpr (has_push_back<VecInt, int> && has_size<VecInt> && has_operator_sq_bk<VecInt, int>)
			output_result("operator=(T&&) (move assignment)", test_move_assignment<Vec>());
		else
			output_warning("operator=(T&&) (move assignment)", "can't test, missing requirements: push_back, size, operator[]");
	}
	else
		output_warning("operator=(T&&) (move assignment)", "not implemented");


	if constexpr (has_push_back<VecInt, int>)
		output_result("(destructor)", test_destructor<Vec>());
	else
		output_warning("(destructor)", "can't test, missing requirements: push_back");

	// if constexpr (has_push_back<VecInt, int>)
	// 	output_result("clean up (growth)", test_cleanup_during_growth<Vec>());
	// else
	// 	output_warning("clean up (growth)", "can't test, missing requirements: push_back");

	printf("\n");
}