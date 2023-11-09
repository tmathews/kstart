#include "list.h"

struct list *list_new() {
	// TODO crash if can't alloc
	struct list *l = malloc(sizeof(struct list));
	l->capacity = 16;
	l->length = 0;
	l->items = calloc(l->capacity, sizeof(char *));
	return l;
}

void list_free(struct list *l) {
	l->capacity = 0;
	l->length = 0;
	free(l->items);
	free(l);
}

void list_empty(struct list *l) {
	l->length = 0;
}

void *list_get(struct list *l, int index) {
	if (index < 0 || index >= l->length) {
		return NULL;
	}
	return l->items[index];
}

void list_append(struct list *l, void *item) {
	if (l == NULL) {
		return;
	}
	// Resize list if it's too small.
	if (l->length + 1 >= l->capacity) {
		l->capacity *= 2;
		l->items = realloc(l->items, l->capacity * sizeof(char *));
		if (l->items == NULL) {
			// TODO crash if can't realloc
		}
	}
	l->items[l->length] = item;
	l->length++;
}

void list_remove(struct list *l, int index) {
	if (index < 0 || index >= l->length)
		return;
	for (int i = index; i < l->length-1; i++) {
		l->items[i] = l->items[i+1];
	}
}

void list_iter(struct list *l, void (*fn)(void *ptr)) {
	for (int i = 0; i < l->length; i++) {
		fn(l->items[i]);
	}
}
