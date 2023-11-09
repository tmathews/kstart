#if !defined(LIST)
#define LIST

#include <stdlib.h>

struct list {
	int capacity;
	int length;
	void **items;
};

struct list *list_new();
void list_free(struct list *);
void list_empty(struct list *);
void *list_get(struct list *, int);
void list_append(struct list *, void *);
void list_remove(struct list *, int);
void list_iter(struct list *, void(*)(void *));

#endif
