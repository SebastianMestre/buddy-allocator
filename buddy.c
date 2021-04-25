#include "buddy.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static inline void set_bit(uint8_t* buf, int idx) {
	buf[idx/8] |= (1 << (idx%8));
}
static inline void reset_bit(uint8_t* buf, int idx) {
	buf[idx/8] &= ~(1 << (idx%8));
}
static inline void flip_bit(uint8_t* buf, int idx) {
	buf[idx/8] ^= (1 << (idx%8));
}
static inline int get_bit(uint8_t* buf, int idx) {
	return (buf[idx/8] >> (idx%8)) & 1;
}

static inline int left_child(int idx) {
	return idx * 2 + 1;
}
static inline int right_child(int idx) {
	return idx * 2 + 2;
}
static inline int parent(int idx) {
	return (idx-1) / 2;
}
static inline int sibling(int idx) {
	return ((idx - 1) ^ 1) + 1;
}

// TODO: use a larger minimum allocation size
// TODO: take advantage of this to save space in the book
#define MIN_ALLOC 1

struct allocator make_alloc(size_t data_size) {
	assert(__builtin_popcountll(data_size) == 1);
	assert(data_size >= 8);

	size_t book_size = data_size / 8;
	size_t total_size = data_size + book_size;

	void* buffer = malloc(total_size);

	void* data = buffer;
	void* book = data + data_size;
	memset(book, 0, book_size);

	return (struct allocator){ data_size, data, book };
}

static inline void* split_to_fit(struct allocator* allocator, size_t size, size_t idx, int l, int r) {
	size_t n = r - l;

	assert(n >= size);
	assert(0 < size);
	assert(__builtin_popcountll(n) == 1);

	assert(get_bit(allocator->book, idx) == 0);
	set_bit(allocator->book, idx);

	for (size_t h = n / 2; h >= size; h /= 2) {
		idx = left_child(idx);
		r -= h;

		assert(get_bit(allocator->book, idx) == 0);
		set_bit(allocator->book, idx);
	}

	return allocator->data + l;
}

static inline void* find_free_and_allocate(struct allocator* allocator, size_t size, size_t idx, int l, int r) {
	if (r - l < size) {
		return NULL;
	}

	if (get_bit(allocator->book, idx) == 0) {
		return split_to_fit(allocator, size, idx, l, r);
	}

	if (left_child(idx) >= allocator->size ||
	    (get_bit(allocator->book, left_child(idx)) == 0 &&
	     get_bit(allocator->book, right_child(idx)) == 0)) {
		return NULL;
	}

	int m = (l + r) / 2;

	void* attempt_l = find_free_and_allocate(allocator, size, left_child(idx), l, m);
	if (attempt_l) return attempt_l;

	void* attempt_r = find_free_and_allocate(allocator, size, right_child(idx), m, r);
	if (attempt_r) return attempt_r;

	return NULL;
}

void* balloc(struct allocator* allocator, size_t size) {
	if (size == 0) {
		return NULL;
	}

	if (size < MIN_ALLOC) {
		size = MIN_ALLOC;
	}

	return find_free_and_allocate(allocator, size, 0, 0, allocator->size);
}

void bfree(struct allocator* allocator, void* ptr) {
	size_t idx = ptr - allocator->data;

	assert(get_bit(allocator->book, idx) == 1);
	reset_bit(allocator->book, idx);

	while (idx != 0 && get_bit(allocator->book, sibling(idx)) == 0) {
		idx = parent(idx);

		assert(get_bit(allocator->book, idx) == 1);
		reset_bit(allocator->book, idx);
	}
}
