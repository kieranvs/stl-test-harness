#pragma once

#include <cstdio>
#include <typeinfo>
#include <stdexcept>

#include "memory_correctness_item.h"
#include "counted_malloc.h"
#include "tests_common.h"

namespace tests_unique_ptr
{

template <typename UP, typename T> concept has_constructor_ptr = requires(T* tp) { UP{ tp }; };
template <typename UP, typename T> concept has_constructor_val = requires(T&& tp) { UP{ std::move(tp) }; };
template <typename UP, typename T> concept has_constructor_default = requires { UP{}; };
template <typename UP, typename T> concept has_reset = requires(UP u, T * t) { u.reset(t); };
template <typename UP, typename T> concept has_reset_empty = requires(UP u) { u.reset(); };
template <typename UP, typename T> concept has_release = requires(UP u) { { u.release() } -> std::same_as<T*>; };
template <typename UP, typename T> concept has_get = requires(UP u) { { u.get() } -> std::same_as<T*>; };
template <typename UP, typename T> concept has_operator_star = requires(UP u) { { *u } -> std::same_as<T&>; };

struct Fooable
{
	void foo() {}
};
template <template <typename> class UP> concept has_operator_arrow = requires(UP<Fooable> u) { u->foo(); };

template <template <typename> class UniquePtr>
TestResult test_constructor_default()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	UniquePtr<MemoryCorrectnessItem> p{};

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_constructor_ptr()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

	if (MemoryCorrectnessItem::count_alive() != 1)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_constructor_val()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	UniquePtr<MemoryCorrectnessItem> p{ MemoryCorrectnessItem() };

	if (MemoryCorrectnessItem::count_alive() != 1)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_destructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_move_constructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		UniquePtr<MemoryCorrectnessItem> q(std::move(p));

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_move_assignment()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			UniquePtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;

			p = std::move(q);

			if (MemoryCorrectnessItem::count_alive() != 1)
				return TestResult::IncorrectObjectHandling;
		}

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;


	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			UniquePtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;

			q = std::move(p);

			if (MemoryCorrectnessItem::count_alive() != 1)
				return TestResult::IncorrectObjectHandling;
		}

		if (MemoryCorrectnessItem::count_alive() != 0)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_reset()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		p.reset();

		if (MemoryCorrectnessItem::count_alive() != 0)
			return TestResult::IncorrectObjectHandling;
	}

	{
		UniquePtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		MemoryCorrectnessItem* q = new MemoryCorrectnessItem();

		p.reset(q);

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_release()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	MemoryCorrectnessItem* new_raw;

	{
		MemoryCorrectnessItem* raw = new MemoryCorrectnessItem();
		UniquePtr<MemoryCorrectnessItem> p(raw);

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		new_raw = p.release();

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		if (raw != new_raw)
			return TestResult::IncorrectResults;
	}

	if (MemoryCorrectnessItem::count_alive() != 1)
		return TestResult::IncorrectObjectHandling;

	delete new_raw;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_get()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		MemoryCorrectnessItem* raw = new MemoryCorrectnessItem();
		UniquePtr<MemoryCorrectnessItem> p(raw);

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		if (p.get() != raw)
			return TestResult::IncorrectResults;

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_operator_star()
{
	auto ptr = new int;

	UniquePtr<int> p(ptr);

	if (&(*p) != ptr)
		return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
TestResult test_operator_arrow()
{
	struct Foo
	{
		int magic() { return 42; }
	};

	UniquePtr<Foo> p(new Foo());

	if (p->magic() != 42)
		return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class UniquePtr>
void run()
{
	printf("\n%s\n-------------------------------\n", typeid(UniquePtr).name());

	printf("Class methods:\n");

	if constexpr (has_constructor_default<UniquePtr<int>, int>)
		output_result("constructor (default)", test_constructor_default<UniquePtr>());
	else
		output_warning("constructor (default)", "not implemented");

	if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
		output_result("constructor (pointer)", test_constructor_ptr<UniquePtr>());
	else
		output_warning("constructor (pointer)", "not implemented");

	if constexpr (has_constructor_val<UniquePtr<int>, int>)
		output_result("constructor (val)", test_constructor_val<UniquePtr>());
	else
		output_warning("constructor (val)", "not implemented");

	if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
		output_result("destructor", test_destructor<UniquePtr>());
	else
		output_warning("destructor", "can't test, missing requirements: constructor (pointer)");

	if constexpr (std::copy_constructible<UniquePtr<int>>)
		output_result("copy constructor", TestResult::IncorrectResults);
	else
		output_result("copy constructor", TestResult::Pass);

	if constexpr (std::assignable_from<UniquePtr<int>&, UniquePtr<int>&> || std::assignable_from<UniquePtr<int>&, const UniquePtr<int>&> || std::assignable_from<UniquePtr<int>&, const UniquePtr<int>>)
		output_result("copy assignment", TestResult::IncorrectResults);
	else
		output_result("copy assignment", TestResult::Pass);

	if constexpr (std::is_move_constructible_v<UniquePtr<int>>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("move constructor", test_move_constructor<UniquePtr>());
		else
			output_warning("move constructor", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("move constructor", "not implemented");

	if constexpr (std::is_move_assignable_v<UniquePtr<int>>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("move assignment", test_move_assignment<UniquePtr>());
		else
			output_warning("move assignment", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("move assignment", "not implemented");

	if constexpr (has_reset<UniquePtr<int>, int> && has_reset_empty<UniquePtr<int>, int>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("reset", test_reset<UniquePtr>());
		else
			output_warning("reset", "can't test, missing requirements: constructor (pointer)");
	else
		if constexpr (has_reset_empty<UniquePtr<int>, int>)
			output_warning("reset", "only 0 arg version implemented");
		else if constexpr (has_reset<UniquePtr<int>, int>)
			output_warning("reset", "only 1 arg version implemented");
		else
			output_warning("reset", "not implemented");

	if constexpr (has_release<UniquePtr<int>, int>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("release", test_release<UniquePtr>());
		else
			output_warning("release", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("release", "not implemented");

	if constexpr (has_get<UniquePtr<int>, int>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("get", test_get<UniquePtr>());
		else
			output_warning("get", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("get", "not implemented");

	if constexpr (has_operator_star<UniquePtr<int>, int>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("operator*", test_operator_star<UniquePtr>());
		else
			output_warning("operator*", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("operator*", "not implemented");

	if constexpr (has_operator_arrow<UniquePtr>)
		if constexpr (has_constructor_ptr<UniquePtr<int>, int>)
			output_result("operator->", test_operator_arrow<UniquePtr>());
		else
			output_warning("operator->", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("operator->", "not implemented");

	printf("\n");
}

}