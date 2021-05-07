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
#include "mixnamedb.h"
#include "debugstring.h"
#include "fastini.h"
#include "gmixdb.h"
#include "rawfile.h"
#include "ramfile.h"
#include "crc.h"
#include "crc32.h"
#include <algorithm>
#include <cstdio>

void MixNameDatabase::Read_From_XCC(const char *dbfile)
{
    RawFileClass fc(dbfile);
    fc.Open();

    // Get the count of the first tranch of filenames, the TD/Sole names
    Read_XCC_Tranch(fc, HASH_RACRC);
    // Get the count of the second tranch of filenames, the RA names
    Read_XCC_Tranch(fc, HASH_RACRC);
    // Get the count of the third tranch of filenames, the TS names
    Read_XCC_Tranch(fc, HASH_CRC32);
    // Get the count of the second tranch of filenames, the RA2/YR names
    Read_XCC_Tranch(fc, HASH_CRC32);

    Regenerate_Hash_Maps();
}

bool MixNameDatabase::Read_From_Ini(const char *ini_file)
{
    if (ini_file != nullptr) {
        m_saveName = ini_file;
    }

    if (ini_file == nullptr && m_saveName.empty()) {
        return false;
    }

    FastINIClass ini;
    RawFileClass fc(m_saveName.c_str());

    if (ini.Load(fc) == 0) {
        return false;
    }

    auto &list = ini.Section_List();

    for (auto node = list.First(); node != nullptr; node = node->Next()) {
        char buff[FastINIClass::MAX_LINE_LENGTH];
        DataEntry tmp = { node->Get_Name(), "", 0, 0 };
        ini.Get_String(node->Get_Name(), "Comment", "", buff, sizeof(buff));
        tmp.file_desc = buff;
        tmp.ra_crc = ini.Get_Hex(node->Get_Name(), "CnCHash");
        tmp.ts_crc = ini.Get_Hex(node->Get_Name(), "CRC32Hash");

        auto nameit = m_nameMap.find(tmp.file_name);

        // If the file name exists, just ignore it and move on
        if (nameit == m_nameMap.end()) {
            m_nameMap[tmp.file_name] = tmp;
            m_isMapDirty = true;
        }
    }

    if (m_isMapDirty) {
        Regenerate_Hash_Maps();
        m_isMapDirty = false;
    }

    return true;
}

const MixNameDatabase::NameEntry &MixNameDatabase::Get_Entry(int32_t hash, HashMethod crc_type)
{
    static const NameEntry _empty;

    // If additional name data has been added, the maps key'd on the filename hash need updating too.
    if (m_isMapDirty) {
        Regenerate_Hash_Maps();
        m_isMapDirty = false;
    }

    if (crc_type != HASH_ANY) {
        auto it = m_hashMaps[crc_type].find(hash);

        if (it != m_hashMaps[crc_type].end()) {
            return it->second;
        }
    } else {
        for (int i = 0; i < HASH_COUNT; ++i) {
            auto it = m_hashMaps[i].find(hash);

            if (it != m_hashMaps[i].end()) {
                return it->second;
            }
        }
    }

    return _empty;
}

bool MixNameDatabase::Add_Entry(const char *file_name, const char *comment, HashMethod crc_type)
{
    DataEntry tmp = { file_name, comment, 0, 0 };
    std::string name = tmp.file_name;
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    auto it = m_nameMap.find(tmp.file_name);

    if (it == m_nameMap.end()) {
        // If the name wasn't found then just add what we have for it.
        m_nameMap[tmp.file_name] = tmp;
        m_isMapDirty = true;
        it = m_nameMap.find(tmp.file_name);
    }

    switch (crc_type) {
        case HASH_RACRC: // Just add the C&C hash
            if (it->second.ra_crc != 0) {
                return false;
            }

            it->second.ra_crc = Calculate_CRC<CRCEngine>(name.c_str(), (unsigned)name.size());
            m_isMapDirty = true;
            break;
        case HASH_CRC32: // Just add the CRC32 hash.
            if (it->second.ts_crc != 0) {
                return false;
            }

            it->second.ts_crc = Calculate_CRC<CRC32Engine>(name.c_str(), (unsigned)name.size());
            m_isMapDirty = true;
            break;
        case HASH_ANY: // add for all known hash methods.
            if (it->second.ra_crc != 0 && it->second.ts_crc != 0) {
                return false;
            }
            if (it->second.ra_crc == 0) {
                it->second.ra_crc = Calculate_CRC<CRCEngine>(name.c_str(), (unsigned)name.size());
                m_isMapDirty = true;
            }

            if (it->second.ts_crc == 0) {
                it->second.ts_crc = Calculate_CRC<CRC32Engine>(name.c_str(), (unsigned)name.size());
                m_isMapDirty = true;
            }

            break;
        default:
            DBG_LOG("Requested unhandled hash method.");
            return false;
    }

    return true;
}

void MixNameDatabase::Read_XCC_Tranch(FileClass &fc, HashMethod crc_type)
{
    uint32_t entry_count;
    DataEntry tmp = { "", "", 0, 0 };

    // Get the count of the tranch of filenames.
    if (fc.Read(&entry_count, sizeof(entry_count)) != sizeof(entry_count)) {
        return;
    }

    // Retrieve and store the entries
    for (unsigned i = 0; i < entry_count; ++i) {
        // Get the filename.
        Get_String(fc, tmp.file_name);

        // Get the description or comment.
        Get_String(fc, tmp.file_desc);

        std::string name = tmp.file_name;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        int32_t crc = 0;

        if (crc_type == HASH_CRC32) {
            crc = Calculate_CRC<CRC32Engine>(name.c_str(), (unsigned)name.size());
        } else {
            crc = Calculate_CRC<CRCEngine>(name.c_str(), (unsigned)name.size());
        }

        auto nameit = m_nameMap.find(tmp.file_name);

        // If the file name doesn't exist in the map, add it, otherwise we update the missing crc for it.
        if (nameit == m_nameMap.end()) {
            switch (crc_type) {
                default:
                case HASH_RACRC:
                    tmp.ra_crc = crc;
                    break;
                case HASH_CRC32:
                    tmp.ts_crc = crc;
                    break;
            }

            m_nameMap[tmp.file_name] = tmp;
        } else {
            switch (crc_type) {
                default:
                case HASH_RACRC:
                    nameit->second.ra_crc = crc;
                    break;
                case HASH_CRC32:
                    nameit->second.ts_crc = crc;
                    break;
            }
        }
    }
}

void MixNameDatabase::Regenerate_Hash_Maps()
{
    for (auto it = m_nameMap.begin(); it != m_nameMap.end(); ++it) {
        if (it->second.ra_crc != 0) {
            // Check for collisions and null out the crc and remove the entry from the respective map.
            auto rait = m_hashMaps[HASH_RACRC].find(it->second.ra_crc);

            if (rait != m_hashMaps[HASH_RACRC].end()) {
                DBG_LOG(
                    "Hash collision, '%s' hashes to same value as '%s' with HashMethod C&C Hash. File name ignored.\n",
                    it->second.file_name.c_str(),
                    rait->second.file_name.c_str());
                it->second.ra_crc = 0;
            } else {
                m_hashMaps[HASH_RACRC][it->second.ra_crc] = { it->second.file_name, it->second.file_desc };
            }
        }

        if (it->second.ts_crc != 0) {
            // Check for collisions and null out the crc and remove the entry from the respective map.
            auto tsit = m_hashMaps[HASH_CRC32].find(it->second.ts_crc);

            if (tsit != m_hashMaps[HASH_CRC32].end()) {
                DBG_LOG(
                    "Hash collision, '%s' hashes to same value as '%s' with HashMethod CRC32. File name ignored.\n",
                    it->second.file_name.c_str(),
                    tsit->second.file_name.c_str());
                it->second.ts_crc = 0;
            } else {
                m_hashMaps[HASH_CRC32][it->second.ts_crc] = { it->second.file_name, it->second.file_desc };
            }
        }
    }
}

void MixNameDatabase::Get_String(FileClass &fc, std::string &dst)
{
    char c;
    dst = "";

    // Get the string.
    while (true) {
        if (fc.Read(&c, sizeof(c)) != sizeof(c)) {
            return;
        }

        if (c == '\0') {
            return;
        }

        dst += c;
    }
}

void MixNameDatabase::Read_Internal()
{
    RAMFileClass fc((void *)g_mixDB, g_mixDB_length);
    fc.Open();

    // Get the count of the first tranch of filenames, the TD/Sole names
    Read_XCC_Tranch(fc, HASH_RACRC);
    // Get the count of the second tranch of filenames, the RA names
    Read_XCC_Tranch(fc, HASH_RACRC);
    // Get the count of the third tranch of filenames, the TS names
    Read_XCC_Tranch(fc, HASH_CRC32);
    // Get the count of the second tranch of filenames, the RA2/YR names
    Read_XCC_Tranch(fc, HASH_CRC32);

    Regenerate_Hash_Maps();
}

void MixNameDatabase::Save_To_Ini(const char *ini_file)
{
    if (ini_file != nullptr) {
        m_saveName = ini_file;
    }

    if (ini_file == nullptr && m_saveName.empty()) {
        return;
    }

    // faster to skip building the ini class and then saving and just write the file direct.
    FILE *fp = std::fopen(m_saveName.c_str(), "w");

    if (fp == nullptr) {
        return;
    }

    // TODO: This currently saves in file name order, we may want alternate sorts.
    for (auto it = m_nameMap.begin(); it != m_nameMap.end(); ++it) {
        fprintf(fp, "[%s]\n", it->second.file_name.c_str());

        if (!it->second.file_desc.empty()) {
            fprintf(fp, "Comment=%s\n", it->second.file_desc.c_str());
        }

        if (it->second.ra_crc != 0) {
            fprintf(fp, "CnCHash=%08X\n", it->second.ra_crc);
        }

        if (it->second.ts_crc != 0) {
            fprintf(fp, "CRC32Hash=%08X\n", it->second.ts_crc);
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}
