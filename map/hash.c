#include "hash.h"

unsigned long get_hashkey_str(const unsigned char *str, int size) {
	unsigned long hash = 5381;
	int c;
	while (c = *str++)
		hash = ((hash << 5) + hash) + c;
	return hash % size;
}

struct map *map_new(int size) {
	struct map *m = malloc(sizeof(struct map));
	if (m == NULL) {
		return NULL;
	}
	m->size = size;
	m->keys = calloc(size, sizeof(struct mapkey *));
	if (m->keys == NULL) {
		free(m);
		return NULL;
	}
	return m;
}

void map_free(struct map *m) {
	struct mapkey *n, *tmp;
	for (int i = 0; i < m->size; i++) {
		n = m->keys[i];
		for (; n != NULL;) {
			tmp = n->next;
			free(n->name);
			free(n);
			n = tmp;
		}
	}
	free(m->keys);
	free(m);
}

bool map_set(struct map *m, const unsigned char *key, void *value) {
	unsigned long hash = get_hashkey_str(key, m->size);
	struct mapkey *node = m->keys[hash];
	struct mapkey *parent = NULL;
	//print_map(m);
	//printf("map_set mapp(%p) key(%s) hash(%d), nodep(%p)\n", m, key, hash, node);
	for (;node != NULL; node = node->next) {
		//printf("nodep(%p)", node); printf(" name: %s\n", node->name);
		if (strcmp(node->name, key) == 0) {
			break;
		}
		parent = node;
	}
	if (node == NULL) {
		//printf("new node!\n");
		node = malloc(sizeof(struct mapkey));
		if (node == NULL)
			return false;
		node->next = NULL;
		node->name = malloc(strlen(key)+1);
		if (node->name == NULL) {
			free(node);
			return false;
		}
		strcpy(node->name, key);
		// assign to parent or root
		if (parent != NULL) {
			parent->next = node;
		} else {
			m->keys[hash] = node;
		}
	}
	node->value = value;
	//printf("assign valuep(%p) to nodep(%p)\n", node->value, node);
	//printf("assigned %p, %p\n", value, node->value);
	return true;
}

void *map_get(struct map *m, const unsigned char *key) {
	unsigned long hash = get_hashkey_str(key, m->size);
	struct mapkey *node = m->keys[hash];
	//printf("map_get: %s -> %d -> %p\n", key, hash, node);
	for (;node != NULL;) {
		//printf("finding key(%s), hash(%d): nodep(%p) valuep(%p)\n", key, hash, node, node->value);
		if (strcmp(node->name, key) == 0) {
			return node->value;
		}
		node = node->next;
	}	
	return NULL;
}

struct mapkeys *map_getkeys(struct map *m) {
	int size = 16;
	struct mapkeys *k = malloc(sizeof(struct mapkeys));
	k->count = 0,
	k->keys = calloc(size, sizeof(char *));
	if (k->keys == NULL) {
		return NULL;
	}
	for (int i = 0; i < m->size; i++) {
		if (m->keys[i] == NULL) {
			continue;
		}
		struct mapkey *n = m->keys[i];
		while(n != NULL) {
			if (k->count >= size) {
				size *= 2;
				k->keys = realloc(k->keys, sizeof(char *) * size);
				if (k->keys == NULL) {
					return NULL;
				}
			}
			k->keys[k->count++] = n->name;
			n = n->next;
		}
	}
	return k;
}

void map_mapkeys_free(struct mapkeys *keys) {
	free(keys->keys);
	free(keys);
}

void print_map(struct map *m) {
	for (int i = 0; i < m->size; i++) {
		printf("[%d] addr(%p)\n", i, m->keys[i]);
	}
}
