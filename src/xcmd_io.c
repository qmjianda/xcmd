#include "xcmd_io.h"
#include <stdlib.h>

int xcmd_io_copy(xcmd_io_t *src, xcmd_io_t *dst, void *buf, size_t buf_size) {
    if (!src || !dst || !buf || buf_size == 0) return XCMD_IO_ERROR;
    
    int total = 0;
    int n;
    
    while ((n = xcmd_io_read(src, buf, buf_size)) > 0) {
        int written = xcmd_io_write(dst, buf, n);
        if (written < 0) return XCMD_IO_ERROR;
        total += written;
        if (written < n) break;
    }
    
    return (n < 0) ? XCMD_IO_ERROR : total;
}