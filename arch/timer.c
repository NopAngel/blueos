#include <include/timer.h>
#include <include/string/string.h>
#include <include/printk.h>

extern int cursor_y;

static timer_info_t timer_info = {0};
static uint64_t timer_ticks = 0;
static uint64_t timer_ms = 0;

#define MAX_ALARMS 32
static timer_alarm_t alarms[MAX_ALARMS];
static int next_alarm_id = 0;

static uint64_t calibration_start = 0;
static uint32_t calibrated_frequency = 0;

void timer_handler(void) {
    timer_ticks++;
    timer_ms += TIMER_MS_PER_TICK;

    timer_info.ticks = timer_ticks;
    timer_info.uptime_ms = timer_ms;

    timer_check_alarms();

    outb(0x20, 0x20);
}

void timer_init(uint32_t frequency_hz) {
    uint32_t divisor = TIMER_FREQUENCY_HZ / frequency_hz;

    outb(TIMER_COMMAND_PORT, TIMER_RATE_GENERATOR);

    outb(TIMER_CHANNEL0_PORT, divisor & 0xFF);
    outb(TIMER_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

    timer_info.ticks = 0;
    timer_info.uptime_ms = 0;
    timer_info.frequency_hz = frequency_hz;
    timer_info.initialized = 1;

    for (int i = 0; i < MAX_ALARMS; i++) {
        alarms[i].active = 0;
    }

    timer_calibrate();

    char msg[50];
    strcpy(msg, "[TIMER] Initialized at ");

    char freq_str[10];
    uint32_t temp = frequency_hz;
    int idx = 0;
    if (temp == 0) {
        freq_str[idx++] = '0';
    } else {
        while (temp > 0) {
            freq_str[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = freq_str[i];
            freq_str[i] = freq_str[idx - 1 - i];
            freq_str[idx - 1 - i] = t;
        }
    }
    freq_str[idx] = '\0';

    strcpy(msg + 22, freq_str);
    strcpy(msg + 22 + idx, " Hz");

    printk(msg, cursor_y++, GREEN);
}

void timer_set_frequency(uint32_t hz) {
    if (hz < 20) hz = 20;
    if (hz > 10000) hz = 10000;

    uint32_t divisor = TIMER_FREQUENCY_HZ / hz;

    outb(TIMER_COMMAND_PORT, TIMER_RATE_GENERATOR);
    outb(TIMER_CHANNEL0_PORT, divisor & 0xFF);
    outb(TIMER_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

    timer_info.frequency_hz = hz;
}

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

uint64_t timer_get_ms(void) {
    return timer_ms;
}

uint32_t timer_get_frequency(void) {
    return timer_info.frequency_hz;
}

void timer_sleep(uint64_t milliseconds) {
    uint64_t target = timer_ms + milliseconds;

    while (timer_ms < target) {
        asm volatile("pause");
    }
}

void timer_delay_us(uint64_t microseconds) {
    uint64_t loops = microseconds * 3;

    for (volatile uint64_t i = 0; i < loops; i++) {
        asm volatile("nop");
    }
}

void timer_delay_ms(uint64_t milliseconds) {
    timer_delay_us(milliseconds * 1000);
}

void timer_delay_ticks(uint64_t ticks) {
    uint64_t target = timer_ticks + ticks;

    while (timer_ticks < target) {
        asm volatile("pause");
    }
}

int timer_set_alarm(uint64_t ms_from_now, void (*callback)(void*), void* data) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].active) {
            alarms[i].trigger_time = timer_ms + ms_from_now;
            alarms[i].callback = callback;
            alarms[i].data = data;
            alarms[i].active = 1;
            alarms[i].repeat = 0;
            alarms[i].interval = 0;

            next_alarm_id++;
            return next_alarm_id - 1;
        }
    }

    return -1;
}

int timer_set_repeating_alarm(uint64_t interval_ms, void (*callback)(void*), void* data) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (!alarms[i].active) {
            alarms[i].trigger_time = timer_ms + interval_ms;
            alarms[i].callback = callback;
            alarms[i].data = data;
            alarms[i].active = 1;
            alarms[i].repeat = 1;
            alarms[i].interval = interval_ms;

            next_alarm_id++;
            return next_alarm_id - 1;
        }
    }

    return -1;
}

void timer_cancel_alarm(int alarm_id) {
    if (alarm_id >= 0 && alarm_id < MAX_ALARMS) {
        alarms[alarm_id].active = 0;
    }
}

void timer_check_alarms(void) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].active && timer_ms >= alarms[i].trigger_time) {
            if (alarms[i].callback) {
                alarms[i].callback(alarms[i].data);
            }

            if (alarms[i].repeat) {
                alarms[i].trigger_time = timer_ms + alarms[i].interval;
            } else {
                alarms[i].active = 0;
            }
        }
    }
}

uint64_t timer_get_elapsed_ms(uint64_t start_ticks) {
    uint64_t current_ticks = timer_get_ticks();
    uint64_t elapsed_ticks = current_ticks - start_ticks;

    return (elapsed_ticks * 1000) / timer_info.frequency_hz;
}

uint64_t timer_get_elapsed_us(uint64_t start_ticks) {
    uint64_t current_ticks = timer_get_ticks();
    uint64_t elapsed_ticks = current_ticks - start_ticks;

    return (elapsed_ticks * 1000000) / timer_info.frequency_hz;
}

uint64_t timer_calculate_elapsed(uint64_t start_ticks, uint64_t end_ticks) {
    return end_ticks - start_ticks;
}

void timer_get_uptime_str(char* buffer, uint32_t buffer_size) {
    if (buffer_size < 20) return;

    uint64_t ms = timer_ms;

    uint64_t days = ms / TIMER_DAY_MS;
    ms %= TIMER_DAY_MS;

    uint64_t hours = ms / TIMER_HOUR_MS;
    ms %= TIMER_HOUR_MS;

    uint64_t minutes = ms / TIMER_MINUTE_MS;
    ms %= TIMER_MINUTE_MS;

    uint64_t seconds = ms / TIMER_SECOND_MS;
    ms %= TIMER_SECOND_MS;

    char days_str[10], hours_str[3], mins_str[3], secs_str[3];

    uint64_t temp = days;
    int idx = 0;
    if (temp == 0) {
        days_str[idx++] = '0';
    } else {
        while (temp > 0) {
            days_str[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = days_str[i];
            days_str[i] = days_str[idx - 1 - i];
            days_str[idx - 1 - i] = t;
        }
    }
    days_str[idx] = '\0';

    temp = hours;
    if (temp < 10) {
        hours_str[0] = '0';
        hours_str[1] = '0' + temp;
    } else {
        hours_str[0] = '0' + (temp / 10);
        hours_str[1] = '0' + (temp % 10);
    }
    hours_str[2] = '\0';

    temp = minutes;
    if (temp < 10) {
        mins_str[0] = '0';
        mins_str[1] = '0' + temp;
    } else {
        mins_str[0] = '0' + (temp / 10);
        mins_str[1] = '0' + (temp % 10);
    }
    mins_str[2] = '\0';

    temp = seconds;
    if (temp < 10) {
        secs_str[0] = '0';
        secs_str[1] = '0' + temp;
    } else {
        secs_str[0] = '0' + (temp / 10);
        secs_str[1] = '0' + (temp % 10);
    }
    secs_str[2] = '\0';

    buffer[0] = '\0';

    if (days > 0) {
        strcpy(buffer, days_str);
        strcpy(buffer + strlen(days_str), "d ");
    }

    char temp_buf[20];
    strcpy(temp_buf, hours_str);
    strcpy(temp_buf + 2, ":");
    strcpy(temp_buf + 3, mins_str);
    strcpy(temp_buf + 5, ":");
    strcpy(temp_buf + 6, secs_str);

    strcpy(buffer + strlen(buffer), temp_buf);
}

void timer_get_time_str(char* buffer, uint32_t buffer_size) {
    if (buffer_size < 15) return;

    uint64_t ms = timer_ms;

    uint64_t hours = (ms / TIMER_HOUR_MS) % 24;
    ms %= TIMER_HOUR_MS;

    uint64_t minutes = ms / TIMER_MINUTE_MS;
    ms %= TIMER_MINUTE_MS;

    uint64_t seconds = ms / TIMER_SECOND_MS;
    ms %= TIMER_SECOND_MS;

    char hours_str[3], mins_str[3], secs_str[3], ms_str[4];

    if (hours < 10) {
        hours_str[0] = '0';
        hours_str[1] = '0' + hours;
    } else {
        hours_str[0] = '0' + (hours / 10);
        hours_str[1] = '0' + (hours % 10);
    }
    hours_str[2] = '\0';

    if (minutes < 10) {
        mins_str[0] = '0';
        mins_str[1] = '0' + minutes;
    } else {
        mins_str[0] = '0' + (minutes / 10);
        mins_str[1] = '0' + (minutes % 10);
    }
    mins_str[2] = '\0';

    if (seconds < 10) {
        secs_str[0] = '0';
        secs_str[1] = '0' + seconds;
    } else {
        secs_str[0] = '0' + (seconds / 10);
        secs_str[1] = '0' + (seconds % 10);
    }
    secs_str[2] = '\0';

    if (ms < 10) {
        ms_str[0] = '0';
        ms_str[1] = '0';
        ms_str[2] = '0' + ms;
    } else if (ms < 100) {
        ms_str[0] = '0';
        ms_str[1] = '0' + (ms / 10);
        ms_str[2] = '0' + (ms % 10);
    } else {
        ms_str[0] = '0' + (ms / 100);
        ms_str[1] = '0' + ((ms / 10) % 10);
        ms_str[2] = '0' + (ms % 10);
    }
    ms_str[3] = '\0';

    buffer[0] = hours_str[0];
    buffer[1] = hours_str[1];
    buffer[2] = ':';
    buffer[3] = mins_str[0];
    buffer[4] = mins_str[1];
    buffer[5] = ':';
    buffer[6] = secs_str[0];
    buffer[7] = secs_str[1];
    buffer[8] = '.';
    buffer[9] = ms_str[0];
    buffer[10] = ms_str[1];
    buffer[11] = ms_str[2];
    buffer[12] = '\0';
}

void timer_print_info(void) {
    cursor_y++;
    printk("=== TIMER INFORMATION ===", cursor_y++, CYAN);

    char buffer[50];

    strcpy(buffer, "Frequency: ");
    uint32_t freq = timer_get_frequency();
    char freq_str[10];
    int idx = 0;
    if (freq == 0) {
        freq_str[idx++] = '0';
    } else {
        uint32_t temp = freq;
        while (temp > 0) {
            freq_str[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = freq_str[i];
            freq_str[i] = freq_str[idx - 1 - i];
            freq_str[idx - 1 - i] = t;
        }
    }
    freq_str[idx] = '\0';
    strcpy(buffer + 12, freq_str);
    strcpy(buffer + 12 + idx, " Hz");
    printk(buffer, cursor_y++, WHITE);

    strcpy(buffer, "Ticks: ");
    uint64_t ticks = timer_get_ticks();
    char ticks_str[20];
    idx = 0;
    uint64_t temp64 = ticks;
    if (temp64 == 0) {
        ticks_str[idx++] = '0';
    } else {
        while (temp64 > 0) {
            ticks_str[idx++] = '0' + (temp64 % 10);
            temp64 /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = ticks_str[i];
            ticks_str[i] = ticks_str[idx - 1 - i];
            ticks_str[idx - 1 - i] = t;
        }
    }
    ticks_str[idx] = '\0';
    strcpy(buffer + 7, ticks_str);
    printk(buffer, cursor_y++, WHITE);

    strcpy(buffer, "Uptime: ");
    char uptime_str[30];
    timer_get_uptime_str(uptime_str, sizeof(uptime_str));
    strcpy(buffer + 8, uptime_str);
    printk(buffer, cursor_y++, GREEN);

    strcpy(buffer, "Time: ");
    char time_str[15];
    timer_get_time_str(time_str, sizeof(time_str));
    strcpy(buffer + 6, time_str);
    printk(buffer, cursor_y++, BLUE);

    int active_alarms = 0;
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].active) active_alarms++;
    }

    strcpy(buffer, "Active alarms: ");
    char alarms_str[3];
    if (active_alarms < 10) {
        alarms_str[0] = '0' + active_alarms;
        alarms_str[1] = '\0';
    } else {
        alarms_str[0] = '0' + (active_alarms / 10);
        alarms_str[1] = '0' + (active_alarms % 10);
        alarms_str[2] = '\0';
    }
    strcpy(buffer + 15, alarms_str);
    printk(buffer, cursor_y++, YELLOW);

    cursor_y++;
}

void timer_calibrate(void) {
    calibrated_frequency = timer_info.frequency_hz;

    char msg[40];
    strcpy(msg, "[TIMER] Calibrated at ");

    char freq_str[10];
    uint32_t temp = calibrated_frequency;
    int idx = 0;
    if (temp == 0) {
        freq_str[idx++] = '0';
    } else {
        while (temp > 0) {
            freq_str[idx++] = '0' + (temp % 10);
            temp /= 10;
        }
        for (int i = 0; i < idx / 2; i++) {
            char t = freq_str[i];
            freq_str[i] = freq_str[idx - 1 - i];
            freq_str[idx - 1 - i] = t;
        }
    }
    freq_str[idx] = '\0';

    strcpy(msg + 22, freq_str);
    strcpy(msg + 22 + idx, " Hz");

    printk(msg, cursor_y++, CYAN);
}
