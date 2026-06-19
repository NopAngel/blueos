#ifndef _KERNEL_SYSINIT_H_
#define _KERNEL_SYSINIT_H_

struct sysinit {
  void (*func)(void);
  int order;
};

#define SYSINIT(name, order)                                                   \
  static void name(void);                                                      \
  __attribute__((used, section(".sysinit"))) static const struct sysinit       \
      _sysinit_##name = {.func = name, .order = order};                        \
  static void name(void)

void sysinit_run(void);

#endif /* _KERNEL_SYSINIT_H_ */
