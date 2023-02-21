#pragma once

#include <cstdio>
#include <typeinfo>
#include <stdexcept>

#include "memory_correctness_item.h"
#include "counted_malloc.h"
#include "tests_common.h"

template <typename UP, typename T> concept has_constructor = requires(T * tp) { UP{ tp }; };

template <template <typename> class UniquePtr>
TestResult test_basic()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
void run_tests_unique_ptr()
{
	printf("\n%s\n-------------------------------\n", typeid(UniquePtr).name());

	printf("Class methods:\n");

	if constexpr (has_constructor<UniquePtr<int>, int>)
		output_result("basic", test_basic<UniquePtr>());
	else
		output_warning("basic", "not implemented");

	printf("\n");
}