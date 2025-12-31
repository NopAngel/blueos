// include/timer.h
#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define TIMER_FREQUENCY_HZ 1193180
#define TIMER_DIVISOR 1193      
#define TIMER_MS_PER_TICK 1     


#define TIMER_CHANNEL0_PORT 0x40
#define TIMER_CHANNEL1_PORT 0x41
#define TIMER_CHANNEL2_PORT 0x42
#define TIMER_COMMAND_PORT  0x43


#define TIMER_SQUARE_WAVE   0x36  
#define TIMER_RATE_GENERATOR 0x34 

typedef struct {
    uint64_t ticks;          
    uint64_t uptime_ms;      
    uint32_t frequency_hz;   
    uint8_t  initialized;    
} timer_info_t;


typedef struct {
    uint64_t trigger_time;   
    void (*callback)(void*); 
    void* data;             
    uint8_t active;         
    uint8_t repeat;          
    uint64_t interval;       
} timer_alarm_t;

void timer_init(uint32_t frequency);
void timer_sleep(uint64_t milliseconds);
uint64_t timer_get_ticks(void);
uint64_t timer_get_ms(void);
uint32_t timer_get_frequency(void);
void timer_print_info(void);

int timer_set_alarm(uint64_t ms_from_now, void (*callback)(void*), void* data);
int timer_set_repeating_alarm(uint64_t interval_ms, void (*callback)(void*), void* data);
void timer_cancel_alarm(int alarm_id);
void timer_check_alarms(void);

void timer_delay_us(uint64_t microseconds);
void timer_delay_ms(uint64_t milliseconds);
void timer_delay_ticks(uint64_t ticks);

uint64_t timer_get_elapsed_ms(uint64_t start_ticks);
uint64_t timer_get_elapsed_us(uint64_t start_ticks);
uint64_t timer_calculate_elapsed(uint64_t start_ticks, uint64_t end_ticks);

void timer_get_uptime_str(char* buffer, uint32_t buffer_size);
void timer_get_time_str(char* buffer, uint32_t buffer_size);

void timer_handler(void);
void timer_set_frequency(uint32_t hz);
void timer_calibrate(void);


#define TIMER_SECOND_MS 1000
#define TIMER_MINUTE_MS (60 * TIMER_SECOND_MS)
#define TIMER_HOUR_MS   (60 * TIMER_MINUTE_MS)
#define TIMER_DAY_MS    (24 * TIMER_HOUR_MS)

#endif // TIMER_H