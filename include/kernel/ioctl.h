#ifndef IOCTL_H
#define IOCTL_H

#define _IOC_NRBITS   8
#define _IOC_TYPEBITS 8
#define _IOC_SIZEBITS 14
#define _IOC_DIRBITS  2

#define _IOC_WRITE    1U
#define _IOC_READ     2U

#define _IOC(dir,type,nr,size) \
    (((dir)  << 30) | \
     ((type) << 8)  | \
     ((nr)   << 0)  | \
     ((size) << 16))

#define _IOW(type,nr,size)  _IOC(_IOC_WRITE, (type), (nr), sizeof(size))
#define _IOWR(type,nr,size) _IOC(_IOC_READ|_IOC_WRITE, (type), (nr), sizeof(size))

#endif
