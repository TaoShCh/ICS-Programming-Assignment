#include "common.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
//#define DEFAULT_ENTRY ((void *)0x4000000)

//extern void ramdisk_read(void *, off_t, size_t);
//extern size_t get_ramdisk_size();



extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);


extern void _map(_Protect *p, void *va, void *pa);

uintptr_t loader(_Protect *as, const char *filename) {
  //ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());

	
//  int fd = fs_open(filename, 0, 0);
//  fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));  
//  return (uintptr_t)DEFAULT_ENTRY;
  
  int fd = fs_open(filename, 0, 0);
  int filesize = fs_filesz(fd);
  void *pa;
  void *va = DEFAULT_ENTRY;
  while(filesize > 0) {
	  pa = new_page();
	  _map(as, va, pa);
	  va += PGSIZE;
	  fs_read(fd, pa, PGSIZE);  
	  filesize -= PGSIZE;
  }
  return (uintptr_t)DEFAULT_ENTRY;
}
