#pragma once

#include <stddef.h>

struct allocator {
	size_t size;
	void* data;
	void* book;
};

struct allocator make_alloc(size_t data_size);

void* balloc(struct allocator* allocator, size_t size);

void bfree(struct allocator* allocator, void* ptr);
