#pragma once

#include <cstdio>
#include <typeinfo>
#include <stdexcept>
#include <optional>

#include "memory_correctness_item.h"
#include "counted_malloc.h"
#include "tests_common.h"

template <typename UP, typename T> concept has_constructor_ptr = requires(T * tp) { UP{ tp }; };
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

template <typename UP, typename T> concept has_use_count = requires(UP u) { { u.use_count() } -> std::same_as<long>; };

template <template <typename> class SharedPtr>
TestResult test_constructor_default()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	SharedPtr<MemoryCorrectnessItem> p{};

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_constructor_ptr()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

	if (MemoryCorrectnessItem::count_alive() != 1)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_destructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_move_constructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		SharedPtr<MemoryCorrectnessItem> q(std::move(p));

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			SharedPtr<MemoryCorrectnessItem> q(std::move(p));

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

template <template <typename> class SharedPtr>
TestResult test_copy_constructor()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		SharedPtr<MemoryCorrectnessItem> q(p);

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	{
		std::optional<SharedPtr<MemoryCorrectnessItem>> p;
		p.emplace(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		SharedPtr<MemoryCorrectnessItem> q(*p);

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		p.reset();

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_move_assignment()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			SharedPtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

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
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			SharedPtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

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

	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		SharedPtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

		{
			auto pa = p;
			auto qa = q;

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;

			pa = std::move(qa);

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;
		}

		if (MemoryCorrectnessItem::count_alive() != 2)
			return TestResult::IncorrectObjectHandling;
	}

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_copy_assignment()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			SharedPtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;

			p = q;

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
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());
		{
			SharedPtr<MemoryCorrectnessItem> q(new MemoryCorrectnessItem());

			if (MemoryCorrectnessItem::count_alive() != 2)
				return TestResult::IncorrectObjectHandling;

			q = p;

			if (MemoryCorrectnessItem::count_alive() != 1)
				return TestResult::IncorrectObjectHandling;
		}

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;
	}

	if (MemoryCorrectnessItem::count_alive() != 0)
		return TestResult::IncorrectObjectHandling;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_reset()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

		if (MemoryCorrectnessItem::count_alive() != 1)
			return TestResult::IncorrectObjectHandling;

		p.reset();

		if (MemoryCorrectnessItem::count_alive() != 0)
			return TestResult::IncorrectObjectHandling;
	}

	{
		SharedPtr<MemoryCorrectnessItem> p(new MemoryCorrectnessItem());

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

template <template <typename> class SharedPtr>
TestResult test_get()
{
	MemoryCorrectnessItem::reset();
	counted_malloc_reset();

	{
		MemoryCorrectnessItem* raw = new MemoryCorrectnessItem();
		SharedPtr<MemoryCorrectnessItem> p(raw);

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

template <template <typename> class SharedPtr>
TestResult test_operator_star()
{
	auto ptr = new int;

	SharedPtr<int> p(ptr);

	if (&(*p) != ptr)
		return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_operator_arrow()
{
	struct Foo
	{
		int magic() { return 42; }
	};

	SharedPtr<Foo> p(new Foo());

	if (p->magic() != 42)
		return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
TestResult test_use_count()
{
	SharedPtr<int> p(new int);

	if (p.use_count() != 1)
		return TestResult::IncorrectResults;

	SharedPtr<int> q(p);

	if (p.use_count() != 2)
		return TestResult::IncorrectResults;

	if (q.use_count() != 2)
		return TestResult::IncorrectResults;

	return TestResult::Pass;
}

template <template <typename> class SharedPtr>
void run_tests_shared_ptr()
{
	printf("\n%s\n-------------------------------\n", typeid(SharedPtr).name());

	printf("Class methods:\n");

	if constexpr (has_constructor_default<SharedPtr<int>, int>)
		output_result("constructor (default)", test_constructor_default<SharedPtr>());
	else
		output_warning("constructor (default)", "not implemented");

	if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
		output_result("constructor (pointer)", test_constructor_ptr<SharedPtr>());
	else
		output_warning("constructor (pointer)", "not implemented");

	if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
		output_result("destructor", test_destructor<SharedPtr>());
	else
		output_warning("destructor", "can't test, missing requirements: constructor (pointer)");

	if constexpr (std::copy_constructible<SharedPtr<int>>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("copy constructor", test_copy_constructor<SharedPtr>());
		else
			output_warning("copy constructor", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("copy constructor", "not implemented");

	if constexpr (std::assignable_from<SharedPtr<int>&, SharedPtr<int>&> || std::assignable_from<SharedPtr<int>&, const SharedPtr<int>&> || std::assignable_from<SharedPtr<int>&, const SharedPtr<int>>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("copy assignment", test_copy_assignment<SharedPtr>());
		else
			output_warning("copy assignment", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("copy assignment", "not implemented");

	if constexpr (std::is_move_constructible_v<SharedPtr<int>>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("move constructor", test_move_constructor<SharedPtr>());
		else
			output_warning("move constructor", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("move constructor", "not implemented");

	if constexpr (std::is_move_assignable_v<SharedPtr<int>>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("move assignment", test_move_assignment<SharedPtr>());
		else
			output_warning("move assignment", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("move assignment", "not implemented");

	if constexpr (has_reset<SharedPtr<int>, int> && has_reset_empty<SharedPtr<int>, int>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("reset", test_reset<SharedPtr>());
		else
			output_warning("reset", "can't test, missing requirements: constructor (pointer)");
	else
		if constexpr (has_reset_empty<SharedPtr<int>, int>)
			output_warning("reset", "only 0 arg version implemented");
		else if constexpr (has_reset<SharedPtr<int>, int>)
			output_warning("reset", "only 1 arg version implemented");
		else
			output_warning("reset", "not implemented");

	if constexpr (has_get<SharedPtr<int>, int>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("get", test_get<SharedPtr>());
		else
			output_warning("get", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("get", "not implemented");

	if constexpr (has_operator_star<SharedPtr<int>, int>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("operator*", test_operator_star<SharedPtr>());
		else
			output_warning("operator*", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("operator*", "not implemented");

	if constexpr (has_operator_arrow<SharedPtr>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("operator->", test_operator_arrow<SharedPtr>());
		else
			output_warning("operator->", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("operator->", "not implemented");

	if constexpr (has_use_count<SharedPtr<int>, int>)
		if constexpr (has_constructor_ptr<SharedPtr<int>, int>)
			output_result("use_count", test_use_count<SharedPtr>());
		else
			output_warning("use_count", "can't test, missing requirements: constructor (pointer)");
	else
		output_warning("use_count", "not implemented");

	printf("\n");
}