struct kvm_eventfd {
    int fd;
    unsigned long long count;
    int flags;
};

#define KVM_IOEVENTFD_FLAG_DATAMATCH (1 << 1)
#define KVM_IOEVENTFD_FLAG_PIO       (1 << 2)
#define KVM_IOEVENTFD_FLAG_DEASSIGN  (1 << 3)

struct kvm_ioeventfd {
    unsigned long long addr;
    unsigned long long datamatch;
    unsigned int len;
    int fd;
    unsigned int flags;
    unsigned char pad[36];
};

#define KVMIO 0xAE
#define KVM_IOEVENTFD _IOW(KVMIO, 0x79, struct kvm_ioeventfd)

int kvm_create_eventfd(unsigned long long addr, unsigned long long datamatch, unsigned int len, int flags) {
    int kvm_fd = open("/dev/kvm", O_RDWR);
    if(kvm_fd < 0) return -1;

    int vm_fd = ioctl(kvm_fd, 0xAE01, 0);
    if(vm_fd < 0) {
        close(kvm_fd);
        return -1;
    }

    int event_fd = eventfd(0, 0);
    if(event_fd < 0) {
        close(vm_fd);
        close(kvm_fd);
        return -1;
    }

    struct kvm_ioeventfd ioevent;
    ioevent.addr = addr;
    ioevent.datamatch = datamatch;
    ioevent.len = len;
    ioevent.fd = event_fd;
    ioevent.flags = flags;

    for(int i=0;i<36;i++) ioevent.pad[i] = 0;

    int ret = ioctl(vm_fd, KVM_IOEVENTFD, &ioevent);
    if(ret < 0) {
        close(event_fd);
        close(vm_fd);
        close(kvm_fd);
        return -1;
    }

    close(vm_fd);
    close(kvm_fd);

    return event_fd;
}

void kvm_signal_event(int event_fd) {
    unsigned long long val = 1;
    write(event_fd, &val, sizeof(val));
}

unsigned long long kvm_read_event(int event_fd) {
    unsigned long long val;
    read(event_fd, &val, sizeof(val));
    return val;
}

void kvm_clear_event(int event_fd) {
    unsigned long long val;
    read(event_fd, &val, sizeof(val));
}

int kvm_deassign_eventfd(unsigned long long addr, unsigned long long datamatch, unsigned int len) {
    int kvm_fd = open("/dev/kvm", O_RDWR);
    if(kvm_fd < 0) return -1;

    int vm_fd = ioctl(kvm_fd, 0xAE01, 0);
    if(vm_fd < 0) {
        close(kvm_fd);
        return -1;
    }

    struct kvm_ioeventfd ioevent;
    ioevent.addr = addr;
    ioevent.datamatch = datamatch;
    ioevent.len = len;
    ioevent.fd = -1;
    ioevent.flags = KVM_IOEVENTFD_FLAG_DEASSIGN;

    for(int i=0;i<36;i++) ioevent.pad[i] = 0;

    int ret = ioctl(vm_fd, KVM_IOEVENTFD, &ioevent);

    close(vm_fd);
    close(kvm_fd);

    return ret;
}

struct kvm_userspace_memory_region {
    unsigned int slot;
    unsigned int flags;
    unsigned long long guest_phys_addr;
    unsigned long long memory_size;
    unsigned long long userspace_addr;
};

#define KVM_SET_USER_MEMORY_REGION _IOW(KVMIO, 0x46, struct kvm_userspace_memory_region)

void kvm_setup_memory(int vm_fd, void *mem, unsigned long long size, unsigned long long guest_addr) {
    struct kvm_userspace_memory_region region;
    region.slot = 0;
    region.flags = 0;
    region.guest_phys_addr = guest_addr;
    region.memory_size = size;
    region.userspace_addr = (unsigned long long)mem;

    ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region);
}

int kvm_create_vcpu(int vm_fd) {
    int vcpu_fd = ioctl(vm_fd, 0xAE41, 0);
    return vcpu_fd;
}

void kvm_run_vcpu(int vcpu_fd) {
    struct kvm_run *run = mmap(0, 0x1000, 3, 0x20, vcpu_fd, 0);

    while(1) {
        ioctl(vcpu_fd, 0xAE80, 0);

        switch(run->exit_reason) {
            case 2:
                if(run->io.port == 0x3F8 && run->io.direction == 1) {
                    putchar(run->io.data[0]);
                }
                break;
            case 100:
                if(run->internal.suberror == 0) {
                    unsigned long long *regs = (unsigned long long*)((char*)run + 0x200);
                    regs[16] += 2;
                }
                break;
        }
    }
}

int kvm_setup_eventfd_for_port(unsigned int port, int event_fd) {
    unsigned long long addr = (unsigned long long)port;
    addr |= 0x8000000000000000ULL;

    return kvm_create_eventfd(addr, 0, 1, KVM_IOEVENTFD_FLAG_PIO);
}

void kvm_wait_for_event(int event_fd) {
    unsigned long long val;
    read(event_fd, &val, sizeof(val));
}
