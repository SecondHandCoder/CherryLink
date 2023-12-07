/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Note: Eliminating warnings about undefined functions when using newlib-nano in gcc12
 * 
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-11     SecondHandCoder       first version.
 */

#if defined(__GNUC__)
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;

    return len;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void)file;

    return len;
}

__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;
    
    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;
    
    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;

    errno = EBADF;
    return -1;
}
#endif
