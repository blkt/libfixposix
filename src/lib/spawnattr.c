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

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include <libfixposix.h>
#include "utils.h"
#include "spawn.h"

struct __lfp_spawnattr {
    uint32_t flags;
    sigset_t sigdefault;
    sigset_t sigmask;
    pid_t    pgroup;
    uid_t    uid;
    gid_t    gid;
};

#define LFP_SPAWN_ALLFLAGS ( LFP_SPAWN_SETSIGMASK    | \
                             LFP_SPAWN_SETSIGDEFAULT | \
                             LFP_SPAWN_SETPGROUP     | \
                             LFP_SPAWN_RESETIDS      | \
                             LFP_SPAWN_SETUID        | \
                             LFP_SPAWN_SETGID          )

int lfp_spawnattr_init(lfp_spawnattr_t *attr)
{
    SYSCHECK(EINVAL, attr == NULL);
    memset(attr, 0, sizeof(lfp_spawnattr_t));
    sigemptyset(&attr->sigdefault);
    return 0;
}

int lfp_spawnattr_destroy(lfp_spawnattr_t *attr)
{
    SYSCHECK(EINVAL, attr == NULL);
    return 0;
}

int lfp_spawnattr_getflags(lfp_spawnattr_t *attr, uint32_t *flags)
{
    SYSCHECK(EINVAL, attr == NULL || flags == NULL);
    *flags = attr->flags;
    return 0;
}

int lfp_spawnattr_setflags(lfp_spawnattr_t *attr, const uint32_t flags)
{
    SYSCHECK(EINVAL, attr == NULL || (flags & ~LFP_SPAWN_ALLFLAGS) != 0);
    attr->flags = flags;
    return 0;
}

int lfp_spawnattr_getsigmask(lfp_spawnattr_t *attr, sigset_t *sigmask)
{
    SYSCHECK(EINVAL, attr == NULL || sigmask == NULL);
    *sigmask = attr->sigmask;
    return 0;
}

int lfp_spawnattr_setsigmask(lfp_spawnattr_t *attr, const sigset_t *sigmask)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETSIGMASK;
    attr->sigmask = *sigmask;
    return 0;
}

int lfp_spawnattr_getsigdefault(lfp_spawnattr_t *attr, sigset_t *sigdefault)
{
    SYSCHECK(EINVAL, attr == NULL || sigdefault == NULL);
    *sigdefault = attr->sigdefault;
    return 0;
}

int lfp_spawnattr_setsigdefault(lfp_spawnattr_t *attr, const sigset_t *sigdefault)
{
    SYSCHECK(EINVAL, attr == NULL || sigdefault == NULL);
    attr->flags |= LFP_SPAWN_SETSIGDEFAULT;
    attr->sigdefault = *sigdefault;
    return 0;
}

int lfp_spawnattr_getpgroup(lfp_spawnattr_t *attr, pid_t *pgroup)
{
    SYSCHECK(EINVAL, attr == NULL || pgroup == NULL);
    *pgroup = attr->pgroup;
    return 0;
}

int lfp_spawnattr_setpgroup(lfp_spawnattr_t *attr, const pid_t pgroup)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETPGROUP;
    attr->pgroup = pgroup;
    return 0;
}

int lfp_spawnattr_getuid(lfp_spawnattr_t *attr, uid_t *uid)
{
    SYSCHECK(EINVAL, attr == NULL || uid == NULL);
    *uid = attr->uid;
    return 0;
}

int lfp_spawnattr_setuid(lfp_spawnattr_t *attr, const uid_t uid)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETUID;
    attr->uid = uid;
    return 0;
}

int lfp_spawnattr_getgid(lfp_spawnattr_t *attr, gid_t *gid)
{
    SYSCHECK(EINVAL, attr == NULL || gid == NULL);
    *gid = attr->gid;
    return 0;
}

int lfp_spawnattr_setgid(lfp_spawnattr_t *attr, const gid_t gid)
{
    SYSCHECK(EINVAL, attr == NULL);
    attr->flags |= LFP_SPAWN_SETGID;
    attr->gid = gid;
    return 0;
}



int lfp_spawn_apply_attributes(const lfp_spawnattr_t *attr)
{
    if(attr == NULL)
        return 0;

    SYSCHECK(EINVAL, (attr->flags & LFP_SPAWN_RESETIDS) && \
                     ((attr->flags & LFP_SPAWN_SETUID)  || \
                      (attr->flags & LFP_SPAWN_SETUID)));

    if (attr->flags & LFP_SPAWN_SETSIGMASK)
        if (sigprocmask(SIG_SETMASK, &attr->sigmask, NULL) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETSIGMASK:sigprocmask");
#endif
            return lfp_errno();
        }

    if (attr->flags & LFP_SPAWN_SETSIGDEFAULT) {
        struct sigaction sa = { .sa_flags   = 0,
                                .sa_handler = SIG_DFL };
        for (int i = 1; i <= LFP_NSIG; i++)
            if (sigismember(&attr->sigdefault, i))
                if (sigaction(i, &sa, NULL) < 0) {
#if !defined(NDEBUG)
                    perror("LFP_SPAWN_APPLY_ATTR:SETSIGDEFAULT:sigaction");
#endif
                    return lfp_errno();
                }
    }

    if (attr->flags & LFP_SPAWN_SETPGROUP)
        if (setpgid(0, attr->pgroup) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETPGROUP:setpgid");
#endif
            return lfp_errno();
        }

    if (attr->flags & LFP_SPAWN_RESETIDS) {
        if (seteuid(getuid()) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:RESETIDS:seteuid");
#endif
            return lfp_errno();
        }
        if (setegid(getgid()) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:RESETIDS:setegid");
#endif
            return lfp_errno();
        }
    }

    if (attr->flags & LFP_SPAWN_SETUID)
        if (seteuid(attr->uid) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETUID:seteuid");
#endif
            return lfp_errno();
        }

    if (attr->flags & LFP_SPAWN_SETGID)
        if (setegid(attr->gid) < 0) {
#if !defined(NDEBUG)
            perror("LFP_SPAWN_APPLY_ATTR:SETGID:setegid");
#endif
            return lfp_errno();
        }

    return 0;
}
