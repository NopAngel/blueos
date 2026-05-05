#ifndef _BLUEOS_VMCOREINFO_H
#define _BLUEOS_VMCOREINFO_H


#define VMCOREINFO_SYMBOL(name) \
	printk(YELLOW, "VMCOREINFO: SYMBOL(%s)=%p\n", #name, &name)
#define VMCOREINFO_SIZE(name) \
	printk(CYAN, "VMCOREINFO: SIZE(%s)=%zu\n", #name, sizeof(name))
#define VMCOREINFO_OFFSET(name, field) \
	printk(CYAN, "VMCOREINFO: OFFSET(%s.%s)=%zu\n", #name, #field, offsetof(struct name, field))
#define VMCOREINFO_CONFIG(name) \
	printk(CYAN, "VMCOREINFO: CONFIG_%s=y\n", #name)
#define VMCOREINFO_OFFSET(type, field) \
    printk(CYAN, "VMCOREINFO: OFFSET(%s.%s)=%zu\n", #type, #field, offsetof(type, field))

void arch_vmcoreinfo_init(void);
void dump_vmcoreinfo(void);

#endif
