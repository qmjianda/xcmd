#ifndef XCMD_IO_H
#define XCMD_IO_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    XCMD_IO_OK      =  0,
    XCMD_IO_ERROR   = -1,
    XCMD_IO_AGAIN   = -2,
    XCMD_IO_BUSY    = -3,
} xcmd_io_result_t;

typedef struct xcmd_io xcmd_io_t;

typedef int (*xcmd_io_read_fn)(xcmd_io_t *io, void *buf, size_t len);
typedef int (*xcmd_io_write_fn)(xcmd_io_t *io, const void *buf, size_t len);
typedef int (*xcmd_io_flush_fn)(xcmd_io_t *io);
typedef int (*xcmd_io_close_fn)(xcmd_io_t *io);
typedef int (*xcmd_io_readable_fn)(xcmd_io_t *io);

struct xcmd_io {
    xcmd_io_read_fn     read;
    xcmd_io_write_fn    write;
    xcmd_io_flush_fn    flush;
    xcmd_io_close_fn    close;
    xcmd_io_readable_fn readable;
    void               *user_data;
};

static inline int xcmd_io_read(xcmd_io_t *io, void *buf, size_t len) {
    if (!io || !io->read) return XCMD_IO_ERROR;
    return io->read(io, buf, len);
}

static inline int xcmd_io_write(xcmd_io_t *io, const void *buf, size_t len) {
    if (!io || !io->write) return XCMD_IO_ERROR;
    return io->write(io, buf, len);
}

static inline int xcmd_io_flush(xcmd_io_t *io) {
    if (!io) return XCMD_IO_ERROR;
    return io->flush ? io->flush(io) : XCMD_IO_OK;
}

static inline int xcmd_io_close(xcmd_io_t *io) {
    if (!io) return XCMD_IO_ERROR;
    return io->close ? io->close(io) : XCMD_IO_OK;
}

static inline int xcmd_io_can_read(xcmd_io_t *io) {
    if (!io) return XCMD_IO_ERROR;
    if (!io->readable) return 1;
    return io->readable(io);
}

static inline int xcmd_io_putc(xcmd_io_t *io, uint8_t c) {
    return xcmd_io_write(io, &c, 1);
}

static inline int xcmd_io_getc(xcmd_io_t *io, uint8_t *c) {
    if (!io || !io->read) return XCMD_IO_ERROR;
    return io->read(io, c, 1);
}

static inline int xcmd_io_puts(xcmd_io_t *io, const char *str) {
    if (!str) return XCMD_IO_ERROR;
    return xcmd_io_write(io, str, strlen(str));
}

int xcmd_io_copy(xcmd_io_t *src, xcmd_io_t *dst, void *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_IO_H */
