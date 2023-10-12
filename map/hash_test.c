#include "hash.h"
#include <assert.h>

void test_map();
void test_map_loop();

void main() {
	test_map();
	test_map_loop();
}

void test_map() {
	printf("test_map:\n");
	int a = 1;
	bool b = true;
	float c = 13.37;
	struct map *m = map_new(2);
	assert(m != NULL);
	printf("\tmap_new: OK\n");
	map_set(m, "a", &a);
	map_set(m, "b", &b);
	map_set(m, "c", &c);
	printf("\tmap_set: OK\n");
	
	int *x = map_get(m, "a");
	bool *y = map_get(m, "b");
	float *z = map_get(m, "c");
	//printf("\tx:%p a:%p\n", x, &a);
	assert(x == &a);
	assert(*x == a);
	assert(y == &b);
	assert(*y == b);
	assert(z == &c);
	assert(*z == c);
	printf("\tmap_get: OK\n");

	map_free(m);
	printf("\tmap_free: OK\n");
}

void test_map_loop() {
	printf("test_map_loop:\n");
	struct map *m = map_new(16);
	assert(m != NULL);
	printf("\tmap_new: OK\n");
	int i = 0;
	for (; i < 1000; i++) {
		map_set(m, "a", &i);
	}
	printf("\tmap_set: OK\n");
	
	int *a = map_get(m, "a");
	//printf("%d %p\n", *a, a);
	assert(a == &i);
	assert(*a == i);
	printf("\tmap_get: OK\n");

	map_free(m);
	printf("\tmap_free: OK\n");
}
