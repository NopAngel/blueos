#ifndef _UAPI_BLUEOS_SIGNAL_H
#define _UAPI_BLUEOS_SIGNAL_H

#define SIGHUP    1  
#define SIGINT    2  
#define SIGQUIT   3  
#define SIGILL    4 
#define SIGTRAP   5 
#define SIGABRT   6  
#define SIGKILL   9  
#define SIGSEGV  11  
#define SIGTERM  15  
#define SIG_DFL  ((void (*)(int))0)  
#define SIG_IGN  ((void (*)(int))1)  

typedef void (*sig_handler_t)(int);

#endif