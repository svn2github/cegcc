/* realpath.c - Return the canonicalized absolute pathname */

/* Written 2000 by Werner Almesberger */
/* Adapted for WINCE */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

char *realpath(const char *path, char *resolved_path) {

    char *path_copy;
    char *aptr;
    int res;
    int alloced = 0;

    if (!path) {
	errno = EINVAL;
	return NULL;
    } else if (!*path) {
        errno = ENOENT;
        return NULL;
    }

    path_copy = strdup(path);
    if (!path_copy) {
        errno = ENOMEM;
        return NULL;
    }
    
    // Since this is wince, there is no way we can have '\' characters
    // in file/dir names. So we might as well replace them with '/', won't
    // change nothing.

    aptr = path_copy;
    while (aptr = strchr(aptr, '\\')) { *(aptr++) = '/'; }

    if (*path_copy != '/') {

        // then we need current directory
        char * cwd = getcwd(0,0);
        if (!cwd) {
            // this should only be ENOENT, or ENOMEM
            free(path_copy);
            return NULL;
        }

        aptr = malloc(strlen(cwd) + strlen(path_copy) + 2);

        if (aptr) {
            strcpy(aptr, cwd);
            strcat(aptr, "/");
            strcpy(aptr, path_copy);
        }

        free(path_copy);
        free(cwd);
        if (!aptr) {
            errno = ENOMEM;
            return NULL;
        }
        path_copy = aptr;
    }

    // remove any trailing slashes
    aptr = path_copy + strlen(path_copy) - 1;
    while (aptr > path_copy && *aptr == '/') { *(aptr--) = 0; }

    if (!resolved_path) {
        // since there are no symlinks on WinCE, we are lucky,
        // the resolved path will never exceed this path
        resolved_path = malloc(strlen(path_copy)+1);
        alloced = 1;
    }

    *resolved_path = '/';
    resolved_path[1] = 0; // ought to be faster than strcpy

    aptr = path_copy;
    struct stat st;

    while (1) {

        while (*aptr == '/') { aptr++; }

        // are we done?
        if (!*aptr) { break; }

        // find next slash
        char * slash = strchr(aptr, '/');
        if (slash) {
            *slash = 0;
        }

        if (*aptr == '.') {
            if (!aptr[1]) {
                // encountered "."
                // nothing to do
                if (!slash) { break; }
                aptr = slash + 1;
                continue;
            } 
            if (aptr[1] == '.' && !aptr[2]) {
                // encountered ".."
                // so rewind the path one element back
                char * prev_slash = strrchr(resolved_path, '/');
                if (prev_slash == resolved_path) {
                    // if the only thing in the path now is the 
                    // root, then the root just remains there
                    prev_slash++;
                }
                *prev_slash = 0;

                if (!slash) { break; }
                aptr = slash + 1;
                continue;
            }
        }

        if (resolved_path[1]) {
            strcat(resolved_path, "/");
        }

        strcat(resolved_path, aptr);

        if (slash) {

            if (lstat(resolved_path, &st)) {
                
                if (alloced) {
                    free(resolved_path);
                }
                free(path_copy);

                errno = EACCES;
                return NULL;

            }

            if (!(st.st_mode & S_IFDIR)) {
                // there are more entries ahead, but this one is
                // not a directory. tsk-tsk
                if (alloced) { free(resolved_path); }
                free(path_copy);
                errno = ENOTDIR;
                return NULL;
            }

        } else {
            break;
        }

        aptr = slash + 1;
        continue;

    }

    free(path_copy);
    return resolved_path;
    
}

