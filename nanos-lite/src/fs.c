#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))


void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
	//Log("filename:  %s\n", pathname);
	for (int fd = 0; fd < NR_FILES; fd++)
		if (strcmp(file_table[fd].name, pathname) == 0)
			return fd;
	assert(0);
	return 0;  // never be here
}

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

ssize_t fs_read(int fd, void *buf, size_t len) {
	switch (fd) {
		case FD_STDOUT:
		case FD_STDERR:
		case FD_STDIN:
			return 0;
		case FD_EVENTS:
			len = events_read(buf, len);
			break;
		case FD_DISPINFO:
			if (file_table[FD_DISPINFO].open_offset == file_table[FD_DISPINFO].size)
				return 0;
			if (file_table[FD_DISPINFO].open_offset + len > file_table[FD_DISPINFO].size)
				len = file_table[FD_DISPINFO].size - file_table[FD_DISPINFO].open_offset;
			dispinfo_read(buf, file_table[FD_DISPINFO].open_offset, len);
			file_table[FD_DISPINFO].open_offset += len;
			break;
		default:
			if (file_table[fd].open_offset == file_table[fd].size)
				return 0;
			else if (file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

extern void _putc(char ch);
extern void fb_write(const void *buf, off_t offset, size_t len);

ssize_t fs_write(int fd, const void *buf, size_t len) {
	switch (fd) {
		case FD_STDOUT:
		case FD_STDERR:
			for (int i = 0; i < len; i++)
				_putc(((char *)buf)[i]);
			break;
		case FD_FB:
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		case FD_DISPINFO:
			break;
		default:
			if (file_table[fd].open_offset == file_table[fd].size)
				return 0;
			else if (file_table[fd].open_offset + len > file_table[fd].size)
				len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	switch (fd) {
		case FD_STDOUT:
		case FD_STDERR:
		case FD_STDIN:
			return 0;
		default:	
			switch (whence) {
				case SEEK_SET:
					if (offset < 0 || offset > file_table[fd].size)
						return -1;
					file_table[fd].open_offset = offset;
					break;
				case SEEK_CUR:
					if (offset + file_table[fd].open_offset < 0 || offset + file_table[fd].open_offset > file_table[fd].size)
						return -1;
					file_table[fd].open_offset += offset;
					break;
				case SEEK_END:
					if (offset > 0 || offset + file_table[fd].size < 0)
						return -1;
					file_table[fd].open_offset = file_table[fd].size + offset;
					break;
			}
	}
	return file_table[fd].open_offset;
}

int fs_close(int fd) {
	return 0;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}
