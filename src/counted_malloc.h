#pragma once

extern size_t counted_malloc_allocations;
extern size_t counted_malloc_deallocations;

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