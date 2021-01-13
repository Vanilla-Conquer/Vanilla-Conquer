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
#define _WIN32_WINNT 0x0600
#include "paths.h"
#include "utf.h"
#include <winerror.h>
#include <shlobj.h>

namespace
{
    class FreeCoTaskMemory
    {
        LPWSTR pointer = nullptr;

    public:
        explicit FreeCoTaskMemory(LPWSTR pointer)
            : pointer(pointer){};
        ~FreeCoTaskMemory()
        {
            CoTaskMemFree(pointer);
        }
    };
} // namespace

void PathsClass::Init_Program_Path()
{
    /*
    ** Adapted from https://github.com/gpakosz/whereami
    ** dual licensed under the WTFPL v2 and MIT licenses without any warranty. by Gregory Pakosz (@gpakosz)
    */
    wchar_t buffer1[MAX_PATH];
    wchar_t buffer2[MAX_PATH];
    wchar_t* path = NULL;
    int length = -1;

    while (true) {
        DWORD size;

        size = GetModuleFileNameW(nullptr, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));

        if (size == 0) {
            break;
        } else if (size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0]))) {
            DWORD size_ = size;
            do {
                wchar_t* path_;

                path_ = (wchar_t*)realloc(path, sizeof(wchar_t) * size_ * 2);

                if (!path_) {
                    break;
                }

                size_ *= 2;
                path = path_;
                size = GetModuleFileNameW(nullptr, path, size_);
            } while (size == size_);

            if (size == size_) {
                break;
            }
        } else {
            path = buffer1;
        }

        if (!_wfullpath(buffer2, path, MAX_PATH)) {
            break;
        }

        std::string tmp(static_cast<const char*>(UTF16To8(buffer2)));
        ProgramPath = tmp.substr(0, tmp.find_last_of("\\/"));

        break;
    }

    if (path != buffer1) {
        free(path);
    }
}

void PathsClass::Init_Data_Path()
{
    if (ProgramPath.empty()) {
        Init_Program_Path();
    }

    DataPath = ProgramPath.substr(0, ProgramPath.find_last_of("\\/")) + SEP + "share";

    if (!Suffix.empty()) {
        DataPath += SEP + Suffix;
    }
}

void PathsClass::Init_User_Path()
{
    if (UserPath.empty()) {
        LPWSTR path = nullptr;
        HRESULT hr;
        hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &path);
        FreeCoTaskMemory scope(path);

        if (!SUCCEEDED(hr)) {
            // TODO: handle error
        }

        UserPath = static_cast<const char*>(UTF16To8(path));
        UserPath += "\\Vanilla-Conquer";

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
        pos = temp.find_first_of("\\/", pos + 1);
        if (CreateDirectoryW(UTF8To16(temp.substr(0, pos).c_str()), nullptr) == FALSE) {
            if (GetLastError() != ERROR_ALREADY_EXISTS) {
                ret = false;
                break;
            }
        }
    } while (pos != std::string::npos);

    return ret;
}

bool PathsClass::Is_Absolute(const char* path)
{
    if (strlen(path) < 2) {
        return false;
    }

    return path != nullptr && (path[1] == ':' || (path[0] == '\\' && path[1] == '\\'));
}
