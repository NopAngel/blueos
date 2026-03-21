#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQUENCY_HZ     1193182  
#define TIMER_CHANNEL0_PORT    0x40
#define TIMER_COMMAND_PORT     0x43
#define TIMER_RATE_GENERATOR   0x36     
#define TIMER_DEFAULT_HZ       100
#define TIMER_MS_PER_TICK      (1000 / TIMER_DEFAULT_HZ)
#define MAX_ALARMS             32

#define TIMER_SECOND_MS        1000ULL
#define TIMER_MINUTE_MS        (60ULL * TIMER_SECOND_MS)
#define TIMER_HOUR_MS          (60ULL * TIMER_MINUTE_MS)
#define TIMER_DAY_MS           (24ULL * TIMER_HOUR_MS)

typedef struct {
    uint64_t ticks;
    uint64_t uptime_ms;
    uint32_t frequency_hz;
    int initialized;
} timer_info_t;

typedef struct {
    uint64_t trigger_time;     
    void (*callback)(void*);   
    void* data;                
    int active;                
    int repeat;                
    uint64_t interval;         
} timer_alarm_t;


void     timer_init(uint32_t frequency_hz);
void     timer_handler(void);
void     timer_check_alarms(void);
void     timer_calibrate(void);

uint64_t timer_get_ticks(void);
uint64_t timer_get_ms(void);
uint32_t timer_get_frequency(void);

void     timer_sleep(uint64_t milliseconds);
void     timer_delay_us(uint64_t microseconds);
void     timer_delay_ms(uint64_t milliseconds);
void     timer_delay_ticks(uint64_t ticks);

int      timer_set_alarm(uint64_t ms_from_now, void (*callback)(void*), void* data);
int      timer_set_repeating_alarm(uint64_t interval_ms, void (*callback)(void*), void* data);
void     timer_cancel_alarm(int alarm_id);

void     timer_get_uptime_str(char* buffer, uint32_t buffer_size);
void     timer_get_time_str(char* buffer, uint32_t buffer_size);
void     timer_print_info(void);
uint64_t timer_get_elapsed_ms(uint64_t start_ticks);
uint64_t timer_get_elapsed_us(uint64_t start_ticks);
uint64_t timer_calculate_elapsed(uint64_t start_ticks, uint64_t end_ticks);

#endif