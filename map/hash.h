#if !defined(MAP)
#define MAP

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mapkeys {
	int count;
	char **keys;
};

struct mapkey {
	unsigned char *name;
	void *value;
	struct mapkey *next;
};

struct map {
	int size;
	struct mapkey **keys;
};

unsigned long get_hashkey_str(const unsigned char *str, int size);
struct map *map_new(int size);
void map_free(struct map *m);
bool map_set(struct map *m, const unsigned char *key, void *value);
void *map_get(struct map *m, const unsigned char *key);
struct mapkeys *map_getkeys(struct map *);
void map_mapkeys_free(struct mapkeys *);
void print_map(struct map *m);

#endif
