#include <drivers/rtc.h>

static int kernel_tz_offset = 0;

void time_set_offset(int offset) { kernel_tz_offset = offset; }
