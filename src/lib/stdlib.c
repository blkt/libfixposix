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

#include <lfp/stdlib.h>
#include <lfp/string.h>

#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

int lfp_mkstemp(char *template)
{
    return mkstemp(template);
}

static
char* _lfp_getenv(const char *name, unsigned short len, char *const envp[])
{
    if (envp == NULL) return NULL;
    do {
        if (strlen(*envp) > len && strncmp(name, *envp, len) == 0)
            return (char*)*envp+len;
    } while(++envp);
    return NULL;
}

// FIXME: add autoconf check that confstr(_CS_PATH) returns sane values
static
char* _lfp_default_path(void)
{
    size_t default_path_size = confstr(_CS_PATH, NULL, 0);
    char *default_path = malloc(default_path_size);
    confstr(_CS_PATH, default_path, default_path_size);
    return default_path;
}

char* lfp_getpath(char *const envp[])
{
    char *envpath = _lfp_getenv("PATH=", sizeof("PATH=")-1, envp);
    if (envpath != NULL) {
        return strdup(envpath);
    } else {
        return _lfp_default_path();
    }
}

static pthread_mutex_t ptsname_mutex = PTHREAD_MUTEX_INITIALIZER;

char *lfp_ptsname(int masterfd)
{
    pthread_mutex_lock(&ptsname_mutex);

    char *_name = ptsname(masterfd);
    if (_name != NULL) {
        _name = lfp_strndup(_name, PATH_MAX);
    }

    pthread_mutex_unlock(&ptsname_mutex);

    return _name;
}
