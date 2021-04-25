#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "buddy.h"

void print_buf(void* buf, int n) {
	char const* sep = "";
	for (int i = 0; i < n; i++) {
		printf("%s%02x", sep, ((uint8_t*)buf)[i] & 0xff);
		sep = " ";
	}
	puts("");
}

int main() {

	const int n = 32;

	struct allocator ac = make_alloc(n);

	void* p0 = balloc(&ac, 32);
	memset(p0, 0xee, 32);

	print_buf(ac.data, n);

	bfree(&ac, p0);

	void* p1 = balloc(&ac, 16);
	void* p2 = balloc(&ac, 8);
	memset(p1, 0xcc, 16);
	memset(p2, 0xdd, 8);

	print_buf(ac.data, n);
}
