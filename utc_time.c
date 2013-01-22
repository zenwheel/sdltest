#include "utc_time.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void current_utc_time(struct timespec *ts) {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

/* Elapsed time between begin and end, using data type timespec.
 * Return values simply to indicate return point */
int delta_t(struct timespec *interval, struct timespec *begin, struct timespec *end) {
  interval->tv_nsec = end->tv_nsec - begin->tv_nsec; /* Subtract 'decimal fraction' first */
  if(interval->tv_nsec < 0 ){
    interval->tv_nsec += 1000000000; /* Borrow 1sec from 'tv_sec' if subtraction -ve */
    interval->tv_sec = end->tv_sec - begin->tv_sec - 1; /* Subtract whole number of seconds and return 1 */
    return (1);
  } else {
    interval->tv_sec = end->tv_sec - begin->tv_sec; /* Subtract whole number of seconds and return 0 */
    return (0);
  }
}
