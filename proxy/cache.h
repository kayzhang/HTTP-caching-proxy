#ifndef __CACHE_H__
#define __CACHE_H__

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

class Cache
{
 public:
  struct cache_t {
    int size;
    char * data;
    char * uri;
    struct cache_t * prev;
    struct cache_t * next;
  };
  typedef struct cache_t cache_list;
  
  
  struct CACHE_T {
  int size;
  cache_list *head;
  cache_list *tail;
};
typedef struct CACHE_T CACHE;
  
  CACHE *CA;
  void create_item(cache_list * c);
  void add_to_cache(char* data,char* uri,int size,CACHE*CA);
  void remove(cache_list * c,CACHE* CA);
  void check_cache( char* uri,CACHE*CA);
  CACHE* init_cache();
};

#endif
