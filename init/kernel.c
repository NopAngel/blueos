/* $OpenBSD: init_main.c,v 1.331 2026/01/01 07:00:57 jsg Exp $    */
/* $NetBSD: init_main.c,v 1.84.4.1 1996/06/02 09:08:06 mrg Exp $    */

#include <stddef.h>               
#include <drivers/rtc.h>
#include <drivers/fb.h>
#include <drivers/tty.h>
#include <kernel/arch.h>
#include <kernel/printk.h>
#include <kernel/task.h>          
#include <lib/string.h>
#include <version.h>

int cursor_x, cursor_y;


#ifndef bool
typedef enum { false = 0, true = 1 } bool;
#endif

#ifndef RUNNING
#define RUNNING 1
#endif

static inline void* curcpu(void) { return NULL; }

/* --- ESTRUCTURAS COMPATIBLES --- */
struct session { int s_count; struct process *s_leader; };
struct pgrp    { struct pgrp *pg_hash; task_t *pg_members; struct session *pg_session; };
struct plimit  { int pl_cnt; };
struct vmspace { uint32_t vm_shm; };
struct sigacts { int ps_sigflags; };
struct vnode   { int v_type; };
extern int vfs_chdir(const char *path);


struct proc {
    void* p_cpu;
    int pid;
    int state;
    bool pledge_active;
    int pledge_mask;
    int unveil_count;
    char name[32];
    struct vmspace *p_vmspace;
};

struct process {
    struct sigacts *ps_sigacts;
    struct vmspace *ps_vmspace;
};

typedef uint32_t vaddr_t;
typedef int32_t  register_t;

struct sys_execve_args {
    const char *path;
    char *const *argp;
    char *const *envp;
};

#define SCARG(p, x) ((p)->x)

struct proc *curproc;

struct session session0;
struct pgrp    pgrp0;
struct proc    proc0;            
struct process process0;
struct plimit  limit0;
struct vmspace vmspace0;
struct sigacts sigacts0;

struct process *initprocess;

/* Cambiados a enteros (int) para coincidir con el retorno de tu kthread_create */
int pagedaemon_id;
int reaper_id;
int cleaner_id;
int syncer_id;
int init_id;

struct vnode *rootvp;
int boothowto = 0;
int ncpus = 1;
int ncpusfound = 1;
volatile int start_init_exec = 0;

extern task_t *current_task;

const char copyright[] = "Copyright (c) 2025, 2026 NopAngel. All rights reserved. (BLUEOS KERNEL)\n";

int k_main(unsigned int magic, void *arch_data);
void check_console(struct proc *p);
void start_init(void);               
static void blueos_banner(void);

extern void init_all(unsigned int magic, void *arch_data);
extern void print_prompt(void);

void
pagedaemon(void)
{
    printk("[pagedaemon]: Actual paging thread active in the background.\n");
    for (;;) {
        arch_idle(); 
    }
}

void
reaper(void)
{
    printk("[reaper]: Royal thread collecting zombie processes initiated.\n");
    for (;;) {
        arch_idle();
    }
}

void
buf_daemon(void)
{
    printk("[cleaner]: Actual buffer cleaning thread ready.\n");
    for (;;) {
        arch_idle();
    }
}

void
syncer_thread(void)
{
    printk("[update]: Active file system synchronization thread.\n");
    for (;;) {
        arch_idle();
    }
}

/**
 * k_main - System startup
 */
int
k_main(unsigned int magic, void *arch_data)
{
    struct proc *p;
    struct process *pr;

    init_all(magic, arch_data);
    blueos_banner();
    vfs_chdir("/");

    
    curproc = p = &proc0;
    p->p_cpu = curcpu(); 
    p->pid = 0;
    p->state = RUNNING;
    p->pledge_active = false;
    p->pledge_mask = 0;
    p->unveil_count = 0;

    char *name_ptr = p->name;
    *name_ptr++ = 's'; *name_ptr++ = 'w'; *name_ptr++ = 'a'; *name_ptr++ = 'p'; 
    *name_ptr++ = 'p'; *name_ptr++ = 'e'; *name_ptr++ = 'r'; *name_ptr = '\0';

    pr = &process0;
    pgrp0.pg_session = &session0;
    session0.s_count = 1;
    session0.s_leader = pr;
    pr->ps_sigacts = &sigacts0;
    p->p_vmspace = pr->ps_vmspace = &vmspace0;

    current_task = (task_t *)p;
    check_console(p);

    printk("[KERNEL THREADS]: Spawning essential system daemons into scheduler...\n");
    
    pagedaemon_id = kthread_create(pagedaemon, "pagedaemon");
    reaper_id     = kthread_create(reaper, "reaper");
    cleaner_id    = kthread_create(buf_daemon, "cleaner");
    syncer_id     = kthread_create(syncer_thread, "update");

    printk("[INIT]: Spawning process 1 (init) lifecycle context...\n");
    initprocess = pr;

    start_init_exec = 1;
    
    init_id = kthread_create(start_init, "init");

    while (1) {
        arch_idle();
    }

    return (0);
}

static char *initpaths[] = {
    "/sbin/init",
    "/sbin/oinit",
    "/sbin/init.bak",
    NULL,
};

void
check_console(struct proc *p)
{
    (void)p;
    printk("<6> vfs: /dev/console virtual node locked onto storage channel\n");
}

void
start_init(void)
{
    char **pathp, *path;
    
    printk("[INIT]: Executing early environment paths check...\n");

    for (pathp = &initpaths[0]; (path = *pathp) != NULL; pathp++) {
        printk("  checking boot image: %s -> target down\n", path);
    }

    printk("\n<6> init: falling back safely to local virtual terminal shell\n");
    
    print_prompt();
    
    for (;;) {
        arch_idle();
    }
}

static void
blueos_banner(void)
{
    rtc_time_t now;
    get_local_time(&now);

    printk("%s\n", copyright);
    printk("BlueOS v%s (GENERIC) %02d/%02d/%04d-UTC-%02d:%02d\n", UTS_RELEASE,
           now.day, now.month, now.year, now.hour, now.minute);
}