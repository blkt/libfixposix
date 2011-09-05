/*******************************************************************************/
/* Permission is hereby granted, free of charge, to any person or organization */
/* obtaining a copy of the software and accompanying documentation covered by  */
/* this license (the "Software") to use, reproduce, display, distribute,       */
/* execute, and transmit the Software, and to prepare derivative works of the  */
/* Software, and to permit third-parties to whom the Software is furnished to  */
/* do so, all subject to the following:                                        */
/*                                                                             */
/* The copyright notices in the Software and this entire statement, including  */
/* the above license grant, this restriction and the following disclaimer,     */
/* must be included in all copies of the Software, in whole or in part, and    */
/* all derivative works of the Software, unless such copies or derivative      */
/* works are solely in the form of machine-executable object code generated by */
/* a source language processor.                                                */
/*                                                                             */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    */
/* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT   */
/* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE   */
/* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, */
/* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER */
/* DEALINGS IN THE SOFTWARE.                                                   */
/*******************************************************************************/

#include <config.h>

#include <lfp/time.h>
#include <lfp/errno.h>
#include <lfp/unistd.h>

#if defined(__APPLE__)
# include <mach/mach.h>
# include <mach/clock.h>
#endif

#include "utils.h"

/*******************/
/* clock_settime() */
/*******************/

#if defined(__APPLE__) && !HAVE_CLOCK_GETTIME
static
int _lfp_clock_getres(clock_id_t clk_id, struct timespec *tp)
{
    int ret = 0;
    kern_return_t kr;
    clock_serv_t clk_serv;

    host_t host_self = mach_host_self();
    kr = host_get_clock_service(host_self, clk_id, &clk_serv);
    MACH_SYSCHECK(EINVAL, kr != KERN_SUCCESS);

    natural_t attributes[4];
    mach_msg_type_number_t count = sizeof(attributes) / sizeof(natural_t);
    kr = clock_get_attributes(clk_serv, CLOCK_GET_TIME_RES, attributes, &count);
    MACH_SYSCHECK(EINVAL, kr != KERN_SUCCESS);

    tp->tv_sec  = attributes[0] / 10^9;
    tp->tv_nsec = attributes[0] % 10^9;

  cleanup:
    mach_port_deallocate(mach_task_self(), host_self);
    mach_port_deallocate(mach_task_self(), clk_serv);

    return ret;
}
#endif

int lfp_clock_getres(lfp_clockid_t clk_id, struct timespec *res)
{
#if HAVE_CLOCK_GETTIME
    return clock_getres((clockid_t)clk_id, res);
#elif defined(__APPLE__)
    SYSCHECK(EINVAL, res == NULL);

    switch (clk_id) {
    case CLOCK_REALTIME:
        return _lfp_clock_getres(CALENDAR_CLOCK, res);
    case CLOCK_MONOTONIC:
        return _lfp_clock_getres(REALTIME_CLOCK, res);
    default:
        SYSERR(EINVAL);
    }
#else
# error "BUG! This point should not be reached"
#endif
}


/*******************/
/* clock_gettime() */
/*******************/

#if defined(__APPLE__) && !HAVE_CLOCK_GETTIME
static inline
int _lfp_clock_gettime(clock_id_t clk_id, struct timespec *tp)
{
    int ret = 0;
    kern_return_t kr;
    clock_serv_t clk_serv;
    mach_timespec_t mtp;

    host_t host_self = mach_host_self();
    kr = host_get_clock_service(host_self, clk_id, &clk_serv);
    MACH_SYSCHECK(EINVAL, kr != KERN_SUCCESS);

    kr = clock_get_time(clk_serv, &mtp);
    MACH_SYSCHECK(EINVAL, kr != KERN_SUCCESS);
    _lfp_mach_timespec_t_to_timespec(&mtp, tp);

  cleanup:
    mach_port_deallocate(mach_task_self(), host_self);
    mach_port_deallocate(mach_task_self(), clk_serv);

    return ret;
}
#endif

int lfp_clock_gettime(lfp_clockid_t clk_id, struct timespec *tp)
{
#if HAVE_CLOCK_GETTIME
    return clock_gettime((clockid_t)clk_id, tp);
#elif defined(__APPLE__)
    SYSCHECK(EINVAL, tp == NULL);

    switch (clk_id) {
    case CLOCK_REALTIME:
        return _lfp_clock_gettime(CALENDAR_CLOCK, tp);
    case CLOCK_MONOTONIC:
        return _lfp_clock_gettime(REALTIME_CLOCK, tp);
    default:
        SYSERR(EINVAL);
    }
#else
# error "BUG! This point should not be reached"
#endif
}


/*******************/
/* clock_gettime() */
/*******************/

#if defined(__APPLE__) && !HAVE_CLOCK_GETTIME
static inline
int _lfp_clock_settime_realtime(struct timespec *tp)
{
    struct timeval tv;
    _lfp_timespec_to_timeval(tp, &tv);

    SYSGUARD(settimeofday(&tv, NULL));
    return 0;
}
#endif

int lfp_clock_settime(lfp_clockid_t clk_id, struct timespec *tp)
{
#if HAVE_CLOCK_GETTIME
    return clock_settime((clockid_t)clk_id, tp);
#elif defined(__APPLE__)
    SYSCHECK(EINVAL, tp == NULL);

    switch (clk_id) {
    case CLOCK_REALTIME:
        return _lfp_clock_settime_realtime(tp);
    case CLOCK_MONOTONIC:
        SYSERR(EPERM);
    default:
        SYSERR(EINVAL);
    }
#else
# error "BUG! This point should not be reached"
#endif
}
