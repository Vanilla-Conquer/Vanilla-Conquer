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
#include "stdlib.h"
#include "ini.h"
#include "rawfile.h"
#include <string>

PathsClass& PathsClass::Instance()
{
    static PathsClass _instance;

    return _instance;
}

void PathsClass::Init(const char* suffix, const char* ini_name)
{
    if (suffix != nullptr) {
        Suffix = suffix;
    }

    // First we will look for the config in the same directory as the binary, if found, flag for possible portable mode.
    std::string path(Program_Path());
    path += SEP;
    path += ini_name;

    // Use rawfile here as we want to make sure we hit the exact path.
    RawFileClass file(path.c_str());
    INIClass ini;
    bool use_program_path = false;

    // Check the default locations for the config file, if none is present at all, then all paths will remain default.
    if (file.Is_Available()) {
        ini.Load(file);
        use_program_path = true;
    } else {
        // If we fail the exe path, try the default data path which may be different on posix and shipped by maintainer.
        path = Data_Path();
        path += SEP + Suffix + SEP;
        path += ini_name;

        if (file.Is_Available()) {
            ini.Load(file);
        } else {
            // Finally try the default user data path incase the user has pointed us to an alternative data source.
            path = User_Path();
            path += SEP + Suffix + SEP;
            path += ini_name;

            if (file.Is_Available()) {
                ini.Load(file);
            }
        }
    }

    const char* section = "Paths";
    char buffer[128]; // TODO max ini line size.

    // Even if the config was found with the binary, we still check to see if it gives use alternative paths.
    // If not, assume we are in portable mode and point the DataPath to ProgramPath.
    if (ini.Get_String(section, "DataPath", "", buffer, sizeof(buffer)) < sizeof(buffer) && buffer[0] != '\0') {
        DataPath = buffer;
    } else if (use_program_path) {
        DataPath = ProgramPath;
    }

    // Same goes for the UserPath.
    if (ini.Get_String(section, "UserPath", "", buffer, sizeof(buffer)) < sizeof(buffer) && buffer[0] != '\0') {
        UserPath = buffer;
    } else if (use_program_path) {
        UserPath = ProgramPath;
    }
}

const char* PathsClass::Program_Path()
{
    if (ProgramPath.empty()) {
        Init_Program_Path();
    }

    return ProgramPath.c_str();
}

const char* PathsClass::Data_Path()
{
    if (DataPath.empty()) {
        Init_Data_Path();
    }

    return DataPath.c_str();
}

const char* PathsClass::User_Path()
{
    if (UserPath.empty()) {
        Init_User_Path();
    }

    return UserPath.c_str();
}
