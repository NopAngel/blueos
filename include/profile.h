#ifndef PROFILE_H
#define PROFILE_H

#include <include/printk.h>


#define PROF_SHIFT 2  
extern unsigned int *prof_buffer;
extern unsigned int prof_len;

void profile_init(unsigned int start_addr, unsigned int end_addr);
void profile_tick(unsigned int pc);
void profile_display();

#endif