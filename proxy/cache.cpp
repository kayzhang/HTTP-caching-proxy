#include "cache.h"

#include <iostream>

Cache::CACHE * init_cache() {
  Cache::CACHE * CA = NULL;
  CA->head = NULL;
  CA->tail = NULL;
  CA->size = 0;
  return CA;
}

void add_to_cache(char * data, char * uri, int size, Cache::CACHE * CA) {
  int cache_size = sizeof(Cache::cache_list);
  if (size + cache_size > MAX_OBJECT_SIZE) {
    Cache::cache_list * info = new Cache::cache_list;

    CA->tail = info;

    info->data = data;
    info->uri = uri;
    info->size = size;
    info->next = NULL;
    CA->size += size + cache_size;
  }
}

void remove(Cache::cache_list * c, Cache::CACHE * CA) {
  CA->size = CA->size - c->size - sizeof(Cache::cache_list);
  if (c->prev != NULL) {
    c->prev->next = c->next;
  }
  if (c->next != NULL) {
    c->next->prev = c->prev;
  }
  //set pointers to NULL;
  c->prev = NULL;
  c->next = NULL;
  //delete item to prevent memory leak;
  delete c;
}

Cache::cache_list * check_cache(char * uri, Cache::CACHE * CA) {
  Cache::cache_list * curr = NULL;
  curr = CA->head;
  if (CA->head != NULL) {
    while (curr != NULL) {
      if (strcmp(curr->uri, uri) == 0) {
        std::cout << "Hit!" << std::endl;
        return curr;
      }
    }
  }
  //curr ==NULL ||head==NULL;
  std::cout << "Miss!" << std::endl;
  return NULL;
}
