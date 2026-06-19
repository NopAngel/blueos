#ifndef _BLUEOS_VMCOREINFO_H
#define _BLUEOS_VMCOREINFO_H

#define VMCOREINFO_SYMBOL(name)                                                \
  printk("VMCOREINFO: SYMBOL(%s)=%p\n", #name, &name)
#define VMCOREINFO_SIZE(name)                                                  \
  printk("VMCOREINFO: SIZE(%s)=%zu\n", #name, sizeof(name))
#define VMCOREINFO_OFFSET(name, field)                                         \
  printk("VMCOREINFO: OFFSET(%s.%s)=%zu\n", #name, #field,                     \
         offsetof(struct name, field))
#define VMCOREINFO_CONFIG(name) printk("VMCOREINFO: CONFIG_%s=y\n", #name)
#define VMCOREINFO_OFFSET(type, field)                                         \
  printk("VMCOREINFO: OFFSET(%s.%s)=%zu\n", #type, #field,                     \
         offsetof(type, field))

void arch_vmcoreinfo_init(void);
void dump_vmcoreinfo(void);

#endif
