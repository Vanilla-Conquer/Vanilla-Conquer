// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection
#include "paths.h"
#include "debugstring.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <pwd.h>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#if defined(__linux__)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#if defined(__APPLE__)
#define _DARWIN_BETTER_REALPATH
#include <mach-o/dyld.h>
#include <TargetConditionals.h>
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
#include <sys/sysctl.h>
#endif

namespace
{
    const std::string& User_Home()
    {
        static std::string _path;

        if (_path.empty()) {
            int uid = getuid();
            const char* home_env = std::getenv("HOME");

            if (uid != 0 && home_env) {
                // We only acknowlegde HOME if not root.
                _path = home_env;
            } else {
                struct passwd* pw = nullptr;
                struct passwd pwd;
                long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

                if (bufsize < 0) {
                    bufsize = 16384;
                }

                std::vector<char> buffer;
                buffer.resize(bufsize);
                int error_code = getpwuid_r(uid, &pwd, buffer.data(), buffer.size(), &pw);

                if (error_code) {
                    // TODO: handle error "Unable to get passwd struct.";
                }

                const char* tmp = pw->pw_dir;

                if (!tmp) {
                    // TODO: handle error "User has no home directory?;
                }

                _path = tmp;
            }
        }

        return _path;
    }

    std::string Get_Posix_Default(const char* env_var, const char* relative_path)
    {
        const char* tmp = std::getenv(env_var);

        if (tmp != nullptr && tmp[0] != '\0') {
            if (tmp[0] != '/') {
                char buffer[200];
                std::snprintf(buffer,
                              sizeof(buffer),
                              "'%s' should start with '/' as per XDG specification that the value must be absolute. "
                              "The current value is: "
                              "'%s'.",
                              tmp,
                              env_var);
                // TODO: handle error
            }

            return tmp;
        }

        return User_Home() + "/" + relative_path;
    }
} // namespace

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

void PathsClass::Init_Program_Path()
{
    /*
    ** Adapted from https://github.com/gpakosz/whereami
    ** dual licensed under the WTFPL v2 and MIT licenses without any warranty. by Gregory Pakosz (@gpakosz)
    */
    std::string tmp;
    char buffer[PATH_MAX];
    char* resolved = nullptr;
#if defined(__linux__) || defined(__CYGWIN__) || defined(__sun)
    resolved = realpath(PROC_SELF_EXE, buffer);
#elif defined(__APPLE__)
    char buffer1[PATH_MAX];
    char* path = buffer1;

    uint32_t size = (uint32_t)sizeof(buffer1);
    if (_NSGetExecutablePath(path, &size) == -1) {
        path = (char*)malloc(size);
        if (!_NSGetExecutablePath(path, &size)) {
            free(path);
            return;
        }
    }

    resolved = realpath(path, buffer);

    if (path != buffer1) {
        free(path);
    }
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
    char buffer1[PATH_MAX];
    char* path = buffer1;

#if defined(__NetBSD__)
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
#else
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
#endif
    size_t size = sizeof(buffer1);

    if (sysctl(mib, (u_int)(sizeof(mib) / sizeof(mib[0])), path, &size, NULL, 0) != 0) {
        return;
    }

    resolved = realpath(path, buffer);
#endif
    if (!resolved) {
        return;
    }

    tmp = resolved;
    ProgramPath = tmp.substr(0, tmp.find_last_of("/"));
}

void PathsClass::Init_Data_Path()
{
    if (ProgramPath.empty()) {
        Init_Program_Path();
    }

    DataPath = ProgramPath.substr(0, ProgramPath.find_last_of("/")) + SEP + "share";

    if (!Suffix.empty()) {
        DataPath += SEP + Suffix;
    }
}

void PathsClass::Init_User_Path()
{
    if (UserPath.empty()) {
#ifdef TARGET_OS_MAC
        UserPath = User_Home() + "/Library/Application Support/Vanilla-Conquer";
#else
        UserPath = Get_Posix_Default("XDG_CONFIG_HOME", ".config") + "/vanilla-conquer";
#endif

        if (!Suffix.empty()) {
            UserPath += SEP + Suffix;
        }

        Create_Directory(UserPath.c_str());
    }
}

bool PathsClass::Create_Directory(const char* dirname)
{
    bool ret = true;

    if (dirname == nullptr) {
        return ret;
    }

    std::string temp = dirname;
    size_t pos = 0;
    do {
        pos = temp.find_first_of("/", pos + 1);
        if (mkdir(temp.substr(0, pos).c_str(), 0700) != 0) {
            if (errno != EEXIST) {
                ret = false;
                break;
            }
        }
    } while (pos != std::string::npos);

    return ret;
}

bool PathsClass::Is_Absolute(const char* path)
{
    return path != nullptr && path[0] == '/';
}

std::string PathsClass::Argv_Path(const char* cmd_arg)
{
    std::string ret(PATH_MAX, '\0');
    char arg_dir[PATH_MAX];
    strncpy(arg_dir, cmd_arg, sizeof(arg_dir));

    // remove the executable from path, if any
    char *dir = strrchr(arg_dir, '/');
    if (dir != nullptr) {
        *dir = '\0';
    }

    if (realpath(arg_dir, &ret[0]) == nullptr) {
        DBG_WARN("PathsClass::Argv_Path: realpath() failed");
        ret = arg_dir;
    } else {
        ret.resize(strlen(&ret[0]));
    }
    return ret;
}
