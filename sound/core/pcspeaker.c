#include <include/ports.h>
#include <include/types.h>


static void play_sound(uint32_t nFrequence);
static void nosound(void);
void beep(void);
void timer_wait(int ticks);

static void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;

 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t) (Div) );
 	outb(0x42, (uint8_t) (Div >> 8));

 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(0x61, tmp | 3);
 	}
 }

 static void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;

 	outb(0x61, tmp);
 }

 void beep() {
 	 play_sound(1000);
 	 timer_wait(10);
 	 nosound();
          //set_PIT_2(old_frequency);
 }

 void timer_wait(int ticks) {

    for (int i = 0; i < ticks; i++) {
        for (volatile int j = 0; j < 10000; j++);
    }
}
