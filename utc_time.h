#ifndef __UTC_TIME_H
#define __UTC_TIME_H

#include <time.h>
#include <sys/time.h>

void current_utc_time(struct timespec *ts);
int delta_t(struct timespec *interval, struct timespec *begin, struct timespec *end);

#endif
