#ifndef PLIC_H
#define PLIC_H
void plic_write(uint32_t reg, uint32_t data);
uint32_t plic_read(uint32_t reg);

void plic_complete(uint32_t irq);
uint32_t plic_claim(void);
void plic_init(void);
#endif
