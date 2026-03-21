#ifndef IOCTL_H
#define IOCTL_H

#define _IOC_SIZEBITS   14
#define _IOC_DIRBITS    2

#define _IOC_NONE   0U
#define _IOC_WRITE  1U
#define _IOC_READ   2U

#define _IOWR(type, nr, size_type) \
    ((_IOC_READ | _IOC_WRITE) << 30) | ((type) << 8) | (nr) | (sizeof(size_type) << 16)

#endif