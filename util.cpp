#include "util.h"
void *
_bench_mmap(size_t size, const char *name, int line)
{
	    void *p;

           /*
	    ** On success, mmap() returns a pointer to the mapped area.  On error,
	    ** the value MAP_FAILED (that is, (void *) -1) is returned, and errno
	    ** is set appropriately.
	    **/
	    p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (p == ((void *) -1)) {
	        //printf("mmap %zu bytes @ %s:%d failed: %s", size, name, line, strerror(errno));
		        return NULL;
	    }

	    return p;
}
