//
// Copyright 2020 Electronic Arts Inc.
//
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

/* $Header:   F:\projects\c&c\vcs\code\init.cpv   2.18   16 Oct 1995 16:50:16   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INIT.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : January 20, 1992                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Anim_Init -- Initialize the VQ animation control structure.                               *
 *   Init_Game -- Main game initialization routine.                                            *
 *   Load_Recording_Values -- Loads recording values from recording file                       *
 *   Obfuscate -- Sufficiently transform parameter to thwart casual hackers.                   *
 *   Parse_Command_Line -- Parses the command line parameters.                                 *
 *   Parse_INI_File -- Parses CONQUER.INI for special options                                  *
 *   Play_Intro -- plays the introduction & logo movies                                        *
 *   Save_Recording_Values -- Saves recording values to a recording file                       *
 *   Select_Game -- The game's main menu                                                       *
 *   Version_Number -- Determines the version number.                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "loaddlg.h"
#include "common/gitinfo.h"
#include "listener.h"
#include "soleclient.h"
#include "soleglobals.h"
#include "solehelp.h"
#include "soleparams.h"
#include "solewdt.h"
#include "voicethemes.h"
#include "common/tcpip.h"
#include "common/vqaconfig.h"
#include <time.h>

/****************************************
**	Function prototypes for this module **
*****************************************/
static void Play_Intro(bool for_real = false);
void Init_CDROM_Access(void);

extern unsigned long RandNumb;

extern int SimRandIndex;

extern TimerClass GameTimer;

#define ATTRACT_MODE_TIMEOUT 3600 // timeout for attract mode
#if (0)

long FAR PASCAL _export Start_Game_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
    case WM_CREATE:
        break;

    case WM_COMMAND:
        EndDialog(hwnd, TRUE);
        AllDone = TRUE;
        break;

    case WM_DESTROY:
        EndDialog(hwnd, TRUE);
        break;
    }
    return (DefWindowProc(hwnd, message, wParam, lParam));
}
#endif

extern bool Server_Remote_Connect(void);
extern bool Client_Remote_Connect(void);

/***********************************************************************************************
 * Clear_Team_Scores -- Clear Sole Survivor team scores.                                       *
 *                                                                                             *
 *    Clears all the team scores.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/2020 OB : Implemented.                                                              *
 *=============================================================================================*/
void Clear_Team_Scores()
{
    for (int i = 0; i < 4; ++i) {
        TeamScores[i] = 0;
    }
}

/***********************************************************************************************
 * Init_Game -- Main game initialization routine.                                              *
 *                                                                                             *
 *    Perform all one-time game initializations here. This includes all                        *
 *    allocations and table setups. The intro and other one-time startup                       *
 *    tasks are also performed here.                                                           *
 *                                                                                             *
 * INPUT:   argc,argv   -- Command line arguments.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this ONCE!                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Init_Game(int, char*[])
{
    void const* temp_mouse_shapes;

    CCDebugString("C&C95 - About to load reslib.dll\n");

    /*
    **	Initialize the game object heaps.
    */
    CCDebugString("C&C95 - About to enter Units.Set_Heap\n");
    Units.Set_Heap(UNIT_MAX);
    CCDebugString("C&C95 - About to enter Factories.Set_Heap\n");
    Factories.Set_Heap(FACTORY_MAX);
    CCDebugString("C&C95 - About to enter Terrains.Set_Heap\n");
    Terrains.Set_Heap(TERRAIN_MAX);
    CCDebugString("C&C95 - About to enter Templates.Set_Heap\n");
    Templates.Set_Heap(TEMPLATE_MAX);
    CCDebugString("C&C95 - About to enter Smudges.Set_Heap\n");
    Smudges.Set_Heap(SMUDGE_MAX);
    CCDebugString("C&C95 - About to enter Overlays.Set_Heap\n");
    Overlays.Set_Heap(OVERLAY_MAX);
    CCDebugString("C&C95 - About to enter Infantry.Set_Heap\n");
    Infantry.Set_Heap(INFANTRY_MAX);
    CCDebugString("C&C95 - About to enter Bullets.Set_Heap\n");
    Bullets.Set_Heap(BULLET_MAX);
    CCDebugString("C&C95 - About to enter Buildings.Set_Heap\n");
    Buildings.Set_Heap(BUILDING_MAX);
    CCDebugString("C&C95 - About to enter Anims.Set_Heap\n");
    Anims.Set_Heap(ANIM_MAX);
    CCDebugString("C&C95 - About to enter Aircraft.Set_Heap\n");
    Aircraft.Set_Heap(AIRCRAFT_MAX);
    CCDebugString("C&C95 - About to enter Triggers.Set_Heap\n");
    Triggers.Set_Heap(TRIGGER_MAX);
    CCDebugString("C&C95 - About to enter TeamTypes.Set_Heap\n");
    TeamTypes.Set_Heap(TEAMTYPE_MAX);
    CCDebugString("C&C95 - About to enter Teams.Set_Heap\n");
    Teams.Set_Heap(TEAM_MAX);
    CCDebugString("C&C95 - About to enter Houses.Set_Heap\n");
    Houses.Set_Heap(HOUSE_MAX);

    /*
    **	Initialize all the waypoints to invalid values.
    */
    CCDebugString("C&C95 - About to clear waypoints\n");
    memset(Waypoint, 0xFF, sizeof(Waypoint));

    /*
    **	Setup the keyboard processor in preparation for the game.
    */
    CCDebugString("C&C95 - About to do various keyboard inits\n");
#ifdef FIX_ME_LATER
    Keyboard_Attributes_Off(TRACKEXT | PAUSEON | BREAKON | SCROLLLOCKON | CTRLSON | CTRLCON | PASSBREAKS | FILTERONLY
                            | TASKSWITCHABLE);
#endif // FIX_ME_LATER
    Keyboard->Clear();

    /*
    **	This is the shape staging buffer. It must always be available, so it is
    **	allocated here and never freed. The library sets the globals ShapeBuffer
    **	and ShapeBufferSize to these values, so it can be accessed for other
    **	purposes.
    */
    CCDebugString("C&C95 - About to call Set_Shape_Buffer\n");
    Set_Shape_Buffer(new unsigned char[SHAPE_BUFFER_SIZE], SHAPE_BUFFER_SIZE);

    /*
    **	Bootstrap enough of the system so that the error dialog box can sucessfully
    **	be displayed.
    */
    CCDebugString("C&C95 - About to register LOCAL.MIX\n");

#ifdef DEMO
    new MFCD("DEMOL.MIX");
    MFCD::Cache("DEMOL.MIX");
#else
    int temp = RequiredCD;
    RequiredCD = -2;
    new MFCD("LOCAL.MIX"); // Cached.
    MFCD::Cache("LOCAL.MIX");
    CCDebugString("C&C95 - About to register UPDATE.MIX\n");
    new MFCD("UPDATE.MIX"); // Cached.
    new MFCD("UPDATA.MIX"); // Cached.
    CCDebugString("C&C95 - About to register UPDATEC.MIX\n");
    new MFCD("UPDATEC.MIX"); // Cached.
    MFCD::Cache("UPDATEC.MIX");
#ifdef JAPANESE
    CCDebugString("C&C95 - About to register LANGUAGE.MIX\n");
    new MFCD("LANGUAGE.MIX");
#endif // JAPANESE

    RequiredCD = temp;

#endif
    CCDebugString("C&C95 - About to load fonts\n");
    Green12FontPtr = Load_Alloc_Data(CCFileClass("12GREEN.FNT"));
    Green12GradFontPtr = Load_Alloc_Data(CCFileClass("12GRNGRD.FNT"));
    MapFontPtr = Load_Alloc_Data(CCFileClass("8FAT.FNT"));
    Font8Ptr = MFCD::Retrieve(FONT8);
    FontPtr = (char*)Font8Ptr;
    Set_Font(FontPtr);
    Font3Ptr = MFCD::Retrieve(FONT3);
    //	Font6Ptr = MFCD::Retrieve(FONT6);
    Font6Ptr = Load_Alloc_Data(CCFileClass("6POINT.FNT"));
    // ScoreFontPtr = MFCD::Retrieve("12GRNGRD.FNT");	//GRAD12FN");	//("SCOREFNT.FNT");
    ScoreFontPtr = Load_Alloc_Data(CCFileClass("12GRNGRD.FNT"));
    FontLEDPtr = MFCD::Retrieve("LED.FNT");
    VCRFontPtr = MFCD::Retrieve("VCR.FNT");
    //	GradFont6Ptr = MFCD::Retrieve("GRAD6FNT.FNT");
    GradFont6Ptr = Load_Alloc_Data(CCFileClass("GRAD6FNT.FNT"));
    BlackPalette = new (MEM_CLEAR | MEM_REAL) unsigned char[768];
    GamePalette = new (MEM_CLEAR | MEM_REAL) unsigned char[768];
    OriginalPalette = new (MEM_CLEAR | MEM_REAL) unsigned char[768];
    WhitePalette = new (MEM_CLEAR | MEM_REAL) unsigned char[768];
    memset(WhitePalette, 63, 768);

    CCDebugString("C&C95 - About to set palette\n");
    memset(BlackPalette, 0x01, 768);
    if (!Special.IsFromInstall)
        Set_Palette(BlackPalette);
    memset(BlackPalette, 0, 768);
    if (!Special.IsFromInstall) {
        Set_Palette(BlackPalette);
        CCDebugString("C&C95 - About to clear visible page\n");
        VisiblePage.Clear();
    }

    Set_Palette(GamePalette);

    CCDebugString("C&C95 - About to set the mouse shape\n");
    /*
    ** Since there is no mouse shape currently available we need'
    ** to set one of our own.
    */
#if defined(_WIN32) && !defined(SDL2_BUILD)
    ShowCursor(FALSE);
#endif
    if (MouseInstalled) {
        temp_mouse_shapes = MFCD::Retrieve("MOUSE.SHP");
        if (temp_mouse_shapes) {
            Set_Mouse_Cursor(0, 0, Extract_Shape(temp_mouse_shapes, 0));
            while (Get_Mouse_State() > 1) {
                Show_Mouse();
            }
        }
    }

    CCDebugString("C&C95 - About to enter wait for focus loop\n");
    /*
    ** Process the message loop until we are in focus.
    */
    do {
        CCDebugString("C&C95 - About to call Keyboard->Check\n");
        Keyboard->Check();
    } while (!GameInFocus);
    AllSurfaces.SurfacesRestored = false;

    CCDebugString("C&C95 - About to load the language file\n");
    /*
    **	Fetch the language text from the hard drive first. If it cannot be
    **	found on the hard drive, then look for it in the mixfile.
    */
    if (RawFileClass(Language_Name("CONQUER")).Is_Available()) {
        SystemStrings = (char const*)Load_Alloc_Data(CCFileClass(Language_Name("CONQUER")));
    } else {
        SystemStrings = (char const*)MFCD::Retrieve(Language_Name("CONQUER"));
    }

    /*
    **	Default palette initialization. Uses the desert palette for convenience,
    **	but only the non terrain specific colors matter.
    */
    // Mem_Copy((void *)MFCD::Retrieve("TEMPERAT.PAL"), GamePalette, 768L);
    CCFileClass palfile("TEMPERAT.PAL");
    palfile.Read(GamePalette, 768L);

    if (!MouseInstalled) {
        char buffer[255];
        Set_Palette(GamePalette);
#ifdef GERMAN
        sprintf(buffer, "Sole Survivor kann Ihren Maustreiber nicht finden..");
#else
#ifdef FRENCH
        sprintf(buffer, "Sole Survivor ne peut pas d�tecter votre gestionnaire de souris.");
#else
        sprintf(buffer, "Sole Survivor is unable to detect your mouse driver.");
#endif
#endif
        WWMessageBox().Process(buffer, TXT_OK);
        Prog_End();
        exit(1);
    }

#ifdef DEMO
    /*
    **	Add in any override path specified in the conquer.ini file.
    */
    if (strlen(OverridePath)) {
        CCFileClass::Set_Search_Drives(OverridePath);
    }
#endif
    /*
    **	Initialize access to the CD-ROM and ensure that the CD is inserted. This can, and
    **	most likely will, result in a visible prompt.
    */
#ifndef REMASTER_BUILD
    Init_CDROM_Access();
#endif

#ifndef DEMO
    CCDebugString("C&C95 - About to register addon mixfiles\n");
    /*
    **	Before all else, cache any additional mixfiles.
    */

    /*
    ** Need to search the search paths. ST - 3/15/2019 2:18PM
    */
    const char* path = ".\\";
    char search_path[_MAX_PATH];
    char scan_path[_MAX_PATH];
    Find_File_Data* ffd;
    bool found;

    for (int p = 0; p < 100; p++) {

        strcpy(search_path, path);
        if (search_path[strlen(search_path) - 1] != '\\') {
            strcat(search_path, "\\");
        }

        strcpy(scan_path, search_path);
        strcat(scan_path, "SC*.MIX");
        found = Find_First(scan_path, 0, &ffd);
        while (found) {
            char* ptr = strdup(ffd->GetName());
            new MFCD(ptr);
            MFCD::Cache(ptr);
            found = Find_Next(ffd);
        }
        if (ffd) {
            Find_Close(ffd);
        }

        strcpy(scan_path, search_path);
        strcat(scan_path, "SS*.MIX");
        found = Find_First(scan_path, 0, &ffd);
        while (found) {
            char* ptr = strdup(ffd->GetName());
            new MFCD(ptr);
            MFCD::Cache(ptr);
            found = Find_Next(ffd);
        }
        if (ffd) {
            Find_Close(ffd);
        }

        path = CDFileClass::Get_Search_Path(p);

        if (path == NULL) {
            break;
        }
    }
#endif // DEMO

    CCDebugString("C&C95 - About to register GENERAL.MIX\n");
    //
    // This is a problem because registering the mix file can call Force_CD_Available which will try to load General.Mix
    // Might as well just cut to the chase and call it directly.
    // ST - 1/3/2019 5:19PM
    //
    // if (GeneralMix) delete GeneralMix;
    // GeneralMix = new MFCD("GENERAL.MIX");
    Force_CD_Available(RequiredCD);

    //	if (!_dos_findfirst("SC*.MIX", _A_NORMAL, &ff)) {
    //		do {
    //			new MFCD(ff.name);
    //			MFCD::Cache(ff.name);
    //		} while(!_dos_findnext(&ff));
    //	}

    /*
    **	Inform the file system of the various MIX files.
    */
#ifdef DEMO
    new MFCD("DEMO.MIX");
    if (CCFileClass("DEMOM.MIX").Is_Available()) {
        if (!MoviesMix)
            MoviesMix = new MFCD("DEMOM.MIX");
        ScoresPresent = true;
        ThemeClass::Scan();
    }

#else

    CCDebugString("C&C95 - About to register GENERAL.MIX\n");
    if (!GeneralMix)
        GeneralMix = new MFCD("GENERAL.MIX"); // Never cached.

    CCDebugString("C&C95 - About to register CONQUER.MIX\n");
    new MFCD("CONQUER.MIX"); // Cached.
    // Sole Survivor addition. Load the extra sole survivor mix files.
    CCDebugString("C&C95 - About to register SOLE.MIX\n");
    new MFCD("SOLE.MIX");
    CCDebugString("C&C95 - About to register SOLEDISK.MIX\n");
    new MFCD("SOLEDISK.MIX");
    CCDebugString("C&C95 - About to register TRANSIT.MIX\n");
    new MFCD("TRANSIT.MIX");

    //	if (CCFileClass("MOVIES.MIX").Is_Available()) {
    CCDebugString("C&C95 - About to register MOVIES.MIX\n");
    if (!MoviesMix)
        MoviesMix = new MFCD("MOVIES.MIX"); // Never cached.
                                            //	}

#if (0)

    /*
    ** Extract a movie from a mixfile.
    */
    char* file_ptr = (char*)Alloc(32 * 1024 * 1024, MEM_NORMAL);
    CCFileClass whatever("PINTLE.VQA");

    int len = whatever.Size();

    whatever.Open();

    DWORD actual;
    HANDLE sfile =
        CreateFile("c:\\temp\\PINTLE.VQA", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (sfile != INVALID_HANDLE_VALUE) {
        SetFilePointer(sfile, 0, NULL, FILE_END);

        do {
            whatever.Read(file_ptr, MIN(len, 1024 * 64));
            WriteFile(sfile, file_ptr, MIN(len, 1024 * 64), &actual, NULL);
            len -= MIN(len, 1024 * 64);
        } while (len > 0);

        CloseHandle(sfile);
    }

    whatever.Close();

    Free(file_ptr);

#endif //(0)

    /*
    **	Register the score mixfile.
    */
    CCDebugString("C&C95 - About to register SCORES.MIX\n");
    ScoresPresent = false;
    //	if (CCFileClass("SCORES.MIX").Is_Available()) {
    ScoresPresent = true;
    if (!ScoreMix) {
        ScoreMix = new MFCD("SCORES.MIX");
        ThemeClass::Scan();
    }
//	}
#endif

    /*
    **	These are sound card specific, but the install program would have
    **	copied the coorect versions to the hard drive.
    */
    CCDebugString("C&C95 - About to register SPEECH.MIX\n");
    if (CCFileClass("SPEECH.MIX").Is_Available()) {
        new MFCD("SPEECH.MIX"); // Never cached.
    }
    CCDebugString("C&C95 - About to register SOUNDS.MIX\n");
    new MFCD("SOUNDS.MIX"); // Cached.

    /*
    **	Initialize the animation system.
    */
    CCDebugString("C&C95 - About to initialise the animation system\n");
    Anim_Init();
    Options.Load_Settings();

    /*
    **	Play the introduction movies.
    */
    CCDebugString("C&C95 - About to play the intro movie\n");
    if (!Special.IsFromInstall)
        Play_Intro(true);

    /*
    **	Wait for a VSync; during the vertical blank, set the game palette & blit
    **	the title screen.  We must ensure no RGB values in the game palette match
    **	those in the WWLIB's 'CurrentPalette', or the WWLIB palette-set routine
    **	will skip that color; the VQ player will have changed that color (behind
    **	WWLIB's back), so it will be incorrect.
    */
    memset(CurrentPalette, 0x01, 768);

    if (!Special.IsFromInstall) {
        Load_Title_Screen("HTITLE.PCX", &HidPage, Palette);
        Blit_Hid_Page_To_Seen_Buff();
    }

    Hide_Mouse();
    Wait_Vert_Blank();
    if (!Special.IsFromInstall) {
        Set_Palette(Palette);
        Blit_Hid_Page_To_Seen_Buff();
        Show_Mouse();
    }
    Call_Back();

#ifdef DEMO
    MFCD::Cache("DEMO.MIX");
    MFCD::Cache("SOUNDS.MIX");
#else
    /*
    **	Cache the main game data. This operation can take a very long time.
    */
    MFCD::Cache("CONQUER.MIX");
    // Sole Survivor addition.
    MFCD::Cache("SOLE.MIX");

    if (SampleType != 0 && !Debug_Quiet) {
        MFCD::Cache("SOUNDS.MIX");
        if (Special.IsJuvenile) {
            new MFCD("ZOUNDS.MIX"); // Cached.
            MFCD::Cache("ZOUNDS.MIX");
        }
    }
    Call_Back();
#endif

    //	malloc(2);

    /*
    **	Perform any special debug-only processing. This includes preparing the
    **	monochrome screen.
    */
    Mono_Clear_Screen();

#ifdef ONHOLD
    /*
    ** Check for addition options not specified on the command-line.  This must
    ** be done before the One_Time calls, but after the shape buffer is set up.
    */
    Parse_INI_File();
#endif

    /*
    **	Perform one-time game system initializations.
    */
    Call_Back();
    //	malloc(3);
    Map.One_Time();
    //	malloc(4);
    Logic.One_Time();
    //	malloc(5);
    Options.One_Time();

    //	malloc(6);

    ObjectTypeClass::One_Time();
    BuildingTypeClass::One_Time();
    BulletTypeClass::One_Time();
    HouseTypeClass::One_Time();

    TemplateTypeClass::One_Time();
    OverlayTypeClass::One_Time();
    SmudgeTypeClass::One_Time();
    TerrainTypeClass::One_Time();
    UnitTypeClass::One_Time();

    InfantryTypeClass::One_Time();
    AnimTypeClass::One_Time();
    AircraftTypeClass::One_Time();
    HouseClass::One_Time();

    if (LogTeams) {
        CCDebugString("*ClearTeamScore B:\n");
    }

    Clear_Team_Scores();

    /*
    **	Speech holding tank buffer. Since speech does not mix, it can be placed
    **	into a custom holding tank only as large as the largest speech file to
    **	be played.
    */
    SpeechBuffer = new char[SPEECH_BUFFER_SIZE];
    Call_Back();

    /*
    **	WWLIB bug: MouseState is in some undefined state; show the mouse until
    **	it really shows.
    */
    Map.Set_Default_Mouse(MOUSE_NORMAL, false);
    Show_Mouse();
    //#ifdef FIX_ME_LATER
    while (Get_Mouse_State() > 0)
        Show_Mouse();
    //#endif //FIX_ME_LATER
    Call_Back();

#ifndef DEMO
    /*
    **	Load multiplayer scenario descriptions
    */
    Read_Scenario_Descriptions();
#endif

    /*
    **	Initialize the multiplayer score values
    */
    // Unused Sole Survivor
    MPlayerGamesPlayed = 0;
    MPlayerNumScores = 0;
    MPlayerCurGame = 0;
    for (int i = 0; i < MAX_MULTI_NAMES; i++) {
        MPlayerScore[i].Name[0] = '\0';
        MPlayerScore[i].Wins = 0;
        for (int j = 0; j < MAX_MULTI_GAMES; j++) {
            MPlayerScore[i].Kills[j] = -1; // -1 = this player didn't play this round
        }
    }

    /*
    ** Copy the title screen's palette into the GamePalette & OriginalPalette,
    ** because the options Load routine uses these palettes to set the brightness, etc.
    */
    memcpy(GamePalette, Palette, 768);
    memcpy(OriginalPalette, Palette, 768);

    /*
    **	Read game options, so the GameSpeed is initialized when multiplayer
    ** dialogs are invoked.  (GameSpeed must be synchronized between systems.)
    */
    Options.Load_Settings();

    // Sole Survivor addition.
    Init_Voice_Themes();
    Read_Server_INI_Params(&GameParams);

    return (true);
}

//#ifndef NOMEMCHECK
void Uninit_Game(void)
{
    delete Map.ShadowPage;
    Map.ShadowPage = NULL;
    Map.Free_Cells();

    delete[] SpeechBuffer;

    CCFileClass::Clear_Search_Drives();
    MFCD::Free_All();

    Units.Set_Heap(0);
    Factories.Set_Heap(0);
    Terrains.Set_Heap(0);
    Templates.Set_Heap(0);
    Smudges.Set_Heap(0);
    Overlays.Set_Heap(0);
    Infantry.Set_Heap(0);
    Bullets.Set_Heap(0);
    Buildings.Set_Heap(0);
    Anims.Set_Heap(0);
    Aircraft.Set_Heap(0);
    Triggers.Set_Heap(0);
    TeamTypes.Set_Heap(0);
    Teams.Set_Heap(0);
    Houses.Set_Heap(0);

    delete[] _ShapeBuffer;
    Set_Shape_Buffer(NULL, 0);
    delete[] BlackPalette;
    delete[] GamePalette;
    delete[] OriginalPalette;
    delete[] WhitePalette;

    WWDOS_Shutdown();
    delete[] Palette;
}
//#endif

extern bool Do_The_Internet_Menu_Thang(void);
extern int ShowCommand;

/***********************************************************************************************
 * Select_Game -- The game's main menu                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		fade		if true, will fade the palette in gradually												  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		none. *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		none. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
extern int Com_Fake_Scenario_Dialog(void);
extern int Com_Show_Fake_Scenario_Dialog(void);

bool Select_Game(bool fade)
{
    enum
    {
        SEL_TIMEOUT = -1, // main menu timeout--go into attract mode
        SEL_PRACTICE,     // start practice game
        SEL_PLAYONLINE,   // start online game
        SEL_HELP,         // display help screen
        SEL_SNEEKPEEK,    // play preview movies
        SEL_EXIT,         // exit to DOS
        SEL_NONE,         // placeholder default value
    };
    bool gameloaded = false;  // Has the game been loaded from the menu?
    int selection = SEL_NONE; // the default selection
    bool process = true;      // false = break out of while loop
    bool display = true;
    CountDownTimerClass count;

    if (Special.IsFromInstall) {
        display = false;
        Show_Mouse();
    }

    /*
    **	[Re]set any globals that need it, in preparation for a new scenario
    */
    GameActive = true;
    DoList.Init();
    OutList.Init();
    Frame = 0;
    PlayerWins = false;
    PlayerLoses = false;
    MPlayerObiWan = false;
    if (!Debug_Map) {
        Debug_Unshroud = false;
    }
    Map.Set_Cursor_Shape(0);
    Map.PendingObjectPtr = 0;
    Map.PendingObject = 0;
    Map.PendingHouse = HOUSE_NONE;

    /*
    **	Some new Sole Globals
    */
    WDTRadarAdded = false;
    server_534780 = true;

    /*
    **	Set default mouse shape
    */
    Map.Set_Default_Mouse(MOUSE_NORMAL, false);

    /*
    **	Main menu processing; only do this if we're not in editor mode.
    */
    if (!Debug_Map) {

        /*
        **	Menu selection processing loop
        */
        ScenarioInit++;
        Theme.Queue_Song(THEME_WIN1);
        ScenarioInit--;

        while (process) {
            if (Theme.What_Is_Playing() != THEME_WIN1) {
                Theme.Play_Song(THEME_NONE);
                Theme.Queue_Song(THEME_WIN1);
                Theme.AI();
            }

            /*
            ** If we have just received input focus again after running in the background then
            ** we need to redraw.
            */
            if (AllSurfaces.SurfacesRestored) {
                AllSurfaces.SurfacesRestored = false;
                display = true;
            }

            /*
            **	Redraw the title page if needed
            */
            if (display && selection != 0) {
                Hide_Mouse();

                /*
                **	Display the title page; fade it in if this is the first time
                **	through the loop, and the 'fade' flag is true
                */
                Load_Title_Screen("HTITLE.PCX", &HidPage, Palette);
                memcpy(GamePalette, Palette, 768);
                Blit_Hid_Page_To_Seen_Buff();

                if (fade) {
                    Fade_Palette_To(Palette, FADE_PALETTE_SLOW, Call_Back);
                    fade = false;
                }

                Set_Logic_Page(SeenBuff);

                Fancy_Text_Print("%s",
                                 SeenBuff.Get_Width() - 1,
                                 SeenBuff.Get_Height() - 10,
                                 CC_GREEN,
                                 TBLACK,
                                 TPF_6POINT | TPF_FULLSHADOW | TPF_USE_GRAD_PAL | TPF_RIGHT | TPF_MEDIUM_COLOR,
                                 VersionText);

                display = false;
                Show_Mouse();
            } else {
                if (RunningAsDLL) {
                    return true;
                }
            }

            /*
            **	Display menu and fetch selection from player.
            */
            if (selection != SEL_PRACTICE) {
                OfflineMode = false;
            }

            if (selection == SEL_NONE) {
                selection = Main_Menu(ATTRACT_MODE_TIMEOUT);
            }
            Call_Back();

            switch (selection) {

            /*
            **	Start an offline practice mode.
            */
            case SEL_PRACTICE: {
                OfflineMode = true;
                GameParams.TimeLimit = Options.OfflineGametime;
                GameParams.ScoreLimit = 0;
                GameParams.LifeLimit = 0;
                GameParams.CaptureTheFlag = 0;
                GameParams.HealthBars = 0;
                GameParams.FreeRadarForAll = 0;
                GameParams.Football = 0;
                GameParams.MinPlayers = 0;
                GameParams.IonCannon = 0;
                GameParams.TeamCrates = 0;
                GameParams.SuperSeconds = 30;
                GameParams.ArmageddonTimer = 400;
                GameParams.NoReshroud = false;
                GameParams.IsLadder = false;
                GameParams.IsCrates = true;
                GameParams.UnkBool = false;
                GameParams.IsMaxNumAIsScaled = 0;
                strcpy(GameParams.ChannelName, "Offline Practice");
                GameParams.CrateDensityOverride = 0;
                GameParams.IsAutoTeaming = 0;
                GameParams.IsSquadChannel = 0;
                GameParams.SuperInvuln = 1;
                GameParams.NumTeams = 0;
                GameTimer.Set(0, 1);
                server_534780 = 0;
                Fade_Palette_To(BlackPalette, 15, Call_Back);
                bool choice_made = Unit_Choice_Dialog();
                if (choice_made) {
                    if (Listener != nullptr || Init_Listener()) {
                        Read_MultiPlayer_Settings();
                        ScenPlayer = SCEN_PLAYER_MPLAYER;
                        ScenDir = SCEN_DIR_EAST;
                        srand(time(NULL));
                        Scenario = Get_Map();
                        Whom = HOUSE_NEUTRAL;
                        GameToPlay = GAME_SERVER;
                        display = true;
                        fade = true;
                        process = false;
                    } else {
                        WWMessageBox().Process("Unable to initialize server!", TXT_OK);
                        selection = SEL_NONE;
                        display = true;
                    }
                } else {
                    display = true;
                    selection = SEL_NONE;
                    GameToPlay = GAME_NORMAL;
                    somestate_591BCC = false;
                }
                break;
            }
            /*
            **	Connect to an online game.
            */
            case SEL_PLAYONLINE:
                Read_MultiPlayer_Settings();
                if (Connect_To_Host(SoleHost)) {
                    if (Wait_For_WDT_Connection()) {
                        GameToPlay = GAME_CLIENT;
                        selection = SEL_NONE;
                        process = false;
                    } else {
                        Client_Remote_Disconnect();
                        display = true;
                        selection = SEL_NONE;
                    }
                } else {
                    WWMessageBox().Process(TXT_CANNOT_INIT_CLIENT);
                    display = true;
                    selection = SEL_NONE;
                }
                break;

            /*
            **	Load a saved game.
            */
            case SEL_HELP:
                Help_Menu();
                display = true;
                fade = true;
                selection = SEL_NONE;
                break;

            /*
            **	Play some VQA trailers.
            */
            case SEL_SNEEKPEEK: {
                GameEnum saved = GameToPlay;
                GameToPlay = GAME_NORMAL;
                Hide_Mouse();
                Play_Movie("SIZZLE", THEME_NONE, 0);
                Play_Movie("SIZZLE2", THEME_NONE, 0);
                Show_Mouse();
                GameToPlay = saved;
                display = true;
                fade = true;
                selection = SEL_NONE;

                break;
            }

            /*
            **	Exit to DOS.
            */
            case SEL_EXIT:
                Theme.Fade_Out();
                Fade_Palette_To(BlackPalette, FADE_PALETTE_SLOW, NULL);

                return (false);

            default:
                display = true;
                fade = true;
                selection = SEL_NONE;
                break;
            }
        }
    } else {

        /*
        ** For Debug_Map (editor) mode, if JP option is on, set to load that scenario
        */
        Scenario = 1;
        if (Special.IsJurassic && AreThingiesEnabled) {
            ScenPlayer = SCEN_PLAYER_JP;
            ScenDir = SCEN_DIR_EAST;
        }

        /*
        ** SS: Looks like debug mode was supposed to create a server in sole.
        */
        if (GameToPlay == GAME_SERVER) {
            if (Listener == nullptr && !Init_Listener()) {
                WWMessageBox().Process("Unable to initialize server!", TXT_OK);
                selection = SEL_NONE;
                display = true;
            }
            ScenPlayer = SCEN_PLAYER_MPLAYER;
            ScenDir = SCEN_DIR_EAST;
            srand(time(NULL));
            Scenario = Get_Map();
            Whom = HOUSE_NEUTRAL;
        }
    }
    CCDebugString("C&C95 - About to start game initialisation.\n");
    /*
    **	Don't carry stray keystrokes into game.
    */
    Keyboard->Clear();

    /*
    ** Get a pointer to the compiler's random number seed.
    **	the Get_EAX() must follow immediately after the srand(0) in order to save
    **	the address of the random seed.  (Currently not used.)
    */
    srand(time(NULL));
    Seed = rand();

    /*
    ** If user has specified a desired random number seed, use it for multiplayer games
    */
    if (CustomSeed != 0) {
        Seed = CustomSeed;
    }

    srand(Seed);
    RandNumb = Seed;
    SimRandIndex = 0;

    /*
    **	Load the scenario.  Specify variation 'A' for the editor; for the game,
    **	don't specify a variation, to make 'Set_Scenario_Name()' pick a random one.
    **	Skip this if we've already loaded a save-game.
    */
    if (!gameloaded) {
        if (Debug_Map) {
            Set_Scenario_Name(ScenarioName, Scenario, ScenPlayer, ScenDir, SCEN_VAR_A);
        } else {
            Set_Scenario_Name(ScenarioName, Scenario, ScenPlayer, ScenDir);
        }

        /*
        ** Start_Scenario() changes the palette; so, fade out & clear the screen
        ** before calling it.
        */
        Hide_Mouse();
        Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
        HiddenPage.Clear();
        VisiblePage.Clear();
        Show_Mouse();

        Special.IsFromInstall = 0;
        CCDebugString("C&C95 - Starting scenario.\n");
        if (!Start_Scenario(ScenarioName)) {
            return (false);
        }
        CCDebugString("C&C95 - Scenario started OK.\n");
    }

    /*
    **	For multiplayer games, initialize the inter-player message system.
    **	Do this after loading the scenario, so the map's upper-left corner is
    **	properly set.
    */
    CCDebugString("C&C95 - Initialising message system.\n");
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;
    Messages.Init(Map.TacPixelX, Map.TacPixelY, 6, MAX_MESSAGE_LENGTH, 6 * factor + 1);

    /*
    **	Hide the SeenBuff; force the map to render one frame.  The caller can
    **	then fade the palette in.
    **	(If we loaded a game, this step will fade out the title screen.  If we
    **	started a scenario, Start_Scenario() will have played a couple of VQ
    **	movies, which will have cleared the screen to black already.)
    */
    CCDebugString("C&C95 - About to call Call_Back.\n");
    Call_Back();

    /*
    ** This is desperately sad isnt it?
    */
    Hide_Mouse();
    Hide_Mouse();
    Hide_Mouse();
    Hide_Mouse();
    WWMouse->Erase_Mouse(&HidPage, true);

    Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
    HiddenPage.Clear();
    VisiblePage.Clear();
    Set_Logic_Page(SeenBuff);
    Map.Flag_To_Redraw();
    Call_Back();
    Map.Render();

    if (Debug_Map) {
        while (Get_Mouse_State() > 1) {
            Show_Mouse();
        }
    }

    constexpr int SpeedScale = 256;

    // Destroy_Server_Vector();
    ClientFPS = 30;
    LastServerAIFrame = 0;
    CountDownTimerClass_590454.Set(120, 1);
    CommStatsSpeedScale = SpeedScale;
    RecievedBytesSec = 0;
    SentBytesSec = 0;
    SentTCP = 0;
    SentUDP = 0;
    RecievedTCP = 0;
    RecievedUDP = 0;
    TransmisionStatsTimer.Set(120, 1);
    CrateMaker = true;
    int crate_density = 1;
    ServerCountDownTimerClass_5721D4.Set(36000, 1);

    if (GameParams.CrateDensityOverride <= 0) {
        if (Density <= 0) {
            CrateDensity = Map.MapCellHeight * Map.MapCellWidth;
        } else {
            crate_density = Density;
            CrateDensity = (Map.MapCellHeight * Map.MapCellWidth) / Density;
        }
    } else {
        crate_density = GameParams.CrateDensityOverride;
        CrateDensity = Map.MapCellHeight * Map.MapCellWidth / GameParams.CrateDensityOverride;
    }

    if (CrateDensity > 1000) {
        CrateDensity = 1000;
    }

    Map.Activate(1);

    if (!OfflineMode && (somestate_591BCC || PlayerPtr->Class->House == HOUSE_MULTI2)) {
        Debug_Unshroud = true;
        Map.Activate(1);
        // Add_WDT_Radar();

        if (Debug_Map) {
            Map.Activate(0);
        }
    }

    if (GameParams.FreeRadarForAll && !WDTRadarAdded) {
        Map.Activate(1);
        // Add_WDT_Radar();

        if (Debug_Map) {
            Map.Activate(0);
        }

        PlayerPtr->IsAlerted = true; // This needs verifying as bitfield layout may have changed.
    }

    if (GameToPlay == GAME_SERVER && (GameParams.CaptureTheFlag || GameParams.Football)) {
        // house_server_47A5F8();
    }

    ClientAICalled = false;

    return (true);
}

/***********************************************************************************************
 * Play_Intro -- plays the introduction & logo movies                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		for_real			if true, this function plays the "real" intro; otherwise, it plays		  *
 *							a delicious smorgasbord of visual delights, guaranteed to titillate		  *
 *							the ocular & auditory nerve pathways.												  *
 *							Well, it plays movies, anyway.														  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		none. *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		none. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/06/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Play_Intro(bool for_real)
{
#ifdef REMASTER_BUILD
    return; // No game intro movies. - LLL
#else
    bool playright = !Keyboard->Down(KN_LCTRL) || !Keyboard->Down(KN_RCTRL);
    static int _counter = -1;
    static char* _names[] = {
        "INTRO2",   "GDIEND1",  "GDIEND2",  "GDIFINA",  "GDIFINB",  "AIRSTRK",  "AKIRA",    "BANNER",   "BCANYON",
        "BKGROUND", "BOMBAWAY", "BOMBFLEE", "BURDET1",  "BURDET2",  "CC2TEASE", "CONSYARD", "DESFLEES", "DESKILL",
        "DESOLAT",  "DESSWEEP", "FLAG",     "FLYY",     "FORESTKL", "GAMEOVER", "GDI1",     "GDI10",    "GDI11",
        "GDI12",    "GDI13",    "GDI14",    "GDI15",    "GDI2",     "GDI3",     "GDI3LOSE", "GDI4A",    "GDI4B",
        "GDI5",     "GDI6",     "GDI7",     "GDI8A",    "GDI8B",    "GDI9",     "GDILOSE",  "GUNBOAT",  "HELLVALY",
        "INSITES",  "KANEPRE",  "LANDING",  "LOGO",     "NAPALM",   "NITEJUMP", "NOD1",     "NOD10A",   "NOD10B",
        "NOD11",    "NOD12",    "NOD13",    "NOD1PRE",  "NOD2",     "NOD3",     "NOD4A",    "NOD4B",    "NOD5",
        "NOD6",     "NOD7A",    "NOD7B",    "NOD8",     "NOD9",     "NODEND1",  "NODEND2",  "NODEND3",  "NODEND4",
        "NODFINAL", "NODFLEES", "NODLOSE",  "NODSWEEP", "NUKE",     "OBEL",     "PARATROP", "PINTLE",   "PLANECRA",
        "PODIUM",   "REFINT",   "RETRO",    "SABOTAGE", "SAMDIE",   "SAMSITE",  "SEIGE",    "SETHPRE",  "SPYCRASH",
        "STEALTH",  "SUNDIAL",  "TANKGO",   "TANKKILL", "TBRINFO1", "TBRINFO2", "TBRINFO3", "TIBERFX",  "TRTKIL_D",
        "TURTKILL", "VISOR",    NULL,
    };

    GameType current = GameToPlay;
    Keyboard->Clear();
    GameToPlay = GAME_NORMAL;
    if (for_real) {
        Hide_Mouse();
        Play_Movie("WESTLOGO", THEME_NONE, false);
        Play_Movie("LOGO", THEME_NONE, false);
        Show_Mouse();
    } else {
        if (!Debug_Flag) {
            _counter = 0;
        } else {
            if (playright)
                _counter++;
            if (_counter == -1)
                _counter = 0;
        }
        Hide_Mouse();
        Play_Movie(_names[_counter], THEME_NONE);
        Show_Mouse();
        if (!_names[_counter]) {
            _counter = -1;
        }
    }

    GameToPlay = current;
#endif
}

/***********************************************************************************************
 * Anim_Init -- Initialize the VQ animation control structure.                                 *
 *                                                                                             *
 *    VQ animations are controlled by a structure passed to the VQ player. This routine        *
 *    initializes the structure to values required by C&C.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only need to call this routine once at the beginning of the game.               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
#ifdef DSOUND_BUILD
extern LPDIRECTSOUND SoundObject;
extern LPDIRECTSOUNDBUFFER PrimaryBufferPtr;
#endif
void Anim_Init(void)
{
#ifndef REMASTER_BUILD
    /* Configure player with INI file */
    VQA_DefaultConfig(&AnimControl);
    //	void const * font = Load_Font(FONT8);
    //	AnimControl.EVAFont = (char *)font;
    //	AnimControl.CapFont = (char *)font;

    AnimControl.DrawFlags = VQACFGF_TOPLEFT;
    AnimControl.DrawFlags |= VQACFGF_BUFFER;

    AnimControl.DrawFlags |= VQACFGF_NOSKIP;

    // AnimControl.X1 =0;
    // AnimControl.Y1 =0;
    AnimControl.FrameRate = -1;
    AnimControl.DrawRate = -1;

    AnimControl.DrawerCallback = VQ_Call_Back;
    AnimControl.ImageWidth = 320;
    AnimControl.ImageHeight = 200;
    AnimControl.Vmode = 0;
    AnimControl.ImageBuf = (unsigned char*)SysMemPage.Get_Offset();
    // AnimControl.VBIBit = VertBlank;
    // AnimControl.DrawFlags |= VQACFGF_TOPLEFT;
    AnimControl.OptionFlags |= VQAOPTF_CAPTIONS | VQAOPTF_EVA;

    if (SlowPalette) {
        AnimControl.OptionFlags |= VQAOPTF_SLOWPAL;
    }
    //	AnimControl.AudioBuf = (unsigned char *)HidPage.Get_Buffer();
    //	AnimControl.AudioBufSize = 32768U;
    // AnimControl.DigiCard = NewConfig.DigitCard;
    // AnimControl.HMIBufSize = 8192;
    // AnimControl.DigiHandle = Get_Digi_Handle();
    // AnimControl.Volume = 0x00FF;
    // AnimControl.AudioRate = 22050;
    //	if (NewConfig.Speed) AnimControl.AudioRate = 11025;

#ifdef DSOUND_BUILD
    AnimControl.SoundObject = SoundObject;
    AnimControl.PrimaryBufferPtr = PrimaryBufferPtr;
#endif
#endif

    // if (!Debug_Quiet && Get_Digi_Handle() != -1) {
    // AnimControl.OptionFlags |= VQAOPTF_AUDIO;
    //}

#if (0)
    if (MonoClass::Is_Enabled()) {
        AnimControl.OptionFlags |= VQAOPTF_MONO;
    }
#endif
}

/***********************************************************************************************
 * Parse_Command_Line -- Parses the command line parameters.                                   *
 *                                                                                             *
 *    This routine should be called before the graphic mode is initialized. It examines the    *
 *    command line parameters and sets the appropriate globals. If there is an error, then     *
 *    it outputs a command summary and then returns false.                                     *
 *                                                                                             *
 * INPUT:   argc  -- The number of command line arguments.                                     *
 *                                                                                             *
 *          argv  -- Pointer to character string array that holds the individual arguments.    *
 *                                                                                             *
 * OUTPUT:  bool; Was the command line parsed successfully?                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Parse_Command_Line(int argc, char* argv[])
{
    /*
    **	Parse the command line and set globals to reflect the parameters
    **	passed in.
    */
#ifdef DEMO
    Scenario = 3;
#else
    Scenario = 1;
#endif
    ScenPlayer = SCEN_PLAYER_GDI;
    ScenDir = SCEN_DIR_EAST;
    Whom = HOUSE_GOOD;
    Special.Init();

    Debug_Map = false;
    //	Debug_Play_Map = false;
    Debug_Unshroud = false;

    for (int index = 1; index < argc; index++) {
        char* string; // Pointer to argument.
        long code = 0;

        char arg_string[512];
        int str_len = (int)strlen(argv[index]);
        char* src = argv[index];
        char* dest = arg_string;
        for (int i = 0; i < str_len; i++) {
            if (*src == '\"') {
                src++;
            } else {
                *dest++ = *src++;
            }
        }
        *dest++ = 0;
        string = arg_string;
        strupr(string);

        // string = strupr(argv[index]);

        /*
        **	Print usage text only if requested.
        */
        if (stricmp("/?", string) == 0 || stricmp("-?", string) == 0 || stricmp("-h", string) == 0
            || stricmp("/h", string) == 0) {
            /*
            **	Unrecognized command line parameter... Display usage
            **	and then exit.
            */
#ifdef GERMAN
            puts("Command & Conquer (c) 1995,1996 Westwood Studios\r\n"
                 "Parameter:\r\n"
                 //						"  -CD<Pfad> = Suchpfad f?r Daten-Dateien festlegen.\r\n"
                 "  -DESTNET  = Netzwerkkennung des Zielrechners festlegen\r\n"
                 "              (Syntax: DESTNETxx.xx.xx.xx)\r\n"
                 "  -SOCKET   = Kennung des Netzwerk-Sockets (0 - 16383)\n"
                 "  -STEALTH  = Namen im Mehrspieler-Modus verstecken (\"Boss-Modus\")\r\n"
                 "  -MESSAGES = Mitteilungen von au�erhalb des Spiels zulassen\r\n"
                 //					"  -ELITE    = Fortgeschrittene KI und Gefechtstechniken.\r\n"
                 "\r\n");
#else
#ifdef FRENCH
            puts("Command & Conquer (c) 1995, Westwood Studios\r\n"
                 "Param�tres:\r\n"
                 //						"  -CD<chemin d'acc�s> = Recherche des fichiers dans le\r\n"
                 //						"                        r�pertoire indiqu�.\r\n"
                 "  -DESTNET  = Sp�cifier le num�ro de r�seau du syst�me de destination\r\n"
                 "              (Syntaxe: DESTNETxx.xx.xx.xx)\r\n"
                 "  -SOCKET   = ID poste r�seau (0 � 16383)\r\n"
                 "  -STEALTH  = Cacher les noms en mode multijoueurs (\"Mode Boss\")\r\n"
                 "  -MESSAGES = Autorise les messages ext�rieurs � ce jeu.\r\n"
                 "\r\n");
#else
            puts("Command & Conquer (c) 1995, 1996 Westwood Studios\r\n"
                 "Parameters:\r\n"
#ifdef NEVER
                 "  CHEAT     = Enable debug keys.\r\n"
                 "  -EDITOR    = Enable scenario editor.\r\n"
#endif
                 //						"  -CD<path> = Set search path for data files.\r\n"
                 "  -DESTNET  = Specify Network Number of destination system\r\n"
                 "              (Syntax: DESTNETxx.xx.xx.xx)\r\n"
                 "  -SOCKET   = Network Socket ID (0 - 16383)\n"
                 "  -STEALTH  = Hide multiplayer names (\"Boss mode\")\r\n"
                 "  -MESSAGES = Allow messages from outside this game.\r\n"
                 "  -o        = Enable compatability with version 1.07.\r\n"
#ifdef JAPANESE
                 "  -ENGLISH  = Enable English keyboard compatibility.\r\n"
#endif
//					"  -ELITE    = Advanced AI and combat characteristics.\r\n"
#ifdef NEVER
                 "  -O[options]= Special control options;\r\n"
                 "     1 : Tiberium grows.\r\n"
                 "     2 : Tiberium grows and spreads.\r\n"
                 "     A : Aggressive player unit defense enabled.\r\n"
                 "     B : Bargraphs always displayed.\r\n"
                 "     C : Capture the flag mode.\r\n"
                 "     E : Elite defense mode disable (attacker advantage).\r\n"
                 "     D : Deploy reversal allowed for construction yard.\r\n"
                 "     F : Fleeing from direct immediate threats is enabled.\r\n"
                 "     H : Hussled recharge time.\r\n"
                 "     G : Growth for Tiberium slowed in multiplay.\r\n"
                 "     I : Inert weapons -- no damage occurs.\r\n"
                 "     J : 7th grade sound effects.\r\n"
                 "     M : Monochrome debug messages.\r\n"
                 "     N : Name the civilians and buildings.\r\n"
                 "     P : Path algorithm displayed as it works.\r\n"
                 "     Q : Quiet mode (no sound).\r\n"
                 "     R : Road pieces are not added to buildings.\r\n"
                 "     T : Three point turns for wheeled vehicles.\r\n"
                 "     U : U can target and burn trees.\r\n"
                 "     V : Show target selection by opponent.\r\n"
                 "     X : Make a recording of a multiplayer game.\r\n"
                 "     Y : Play a recording of a multiplayer game.\r\n"
                 "     Z : Disaster containment team.\r\n"
#endif
                 "\r\n");
#endif
#endif
            return (false);
        }

        bool processed = true;
        switch (Obfuscate(string)) {

        /*
        **	Signal that easy mode is active.
        */
        case PARM_EASY:
            Special.IsEasy = true;
            Special.IsDifficult = false;
            break;

        /*
        **	Signal that hard mode is active.
        */
        case PARM_HARD:
            Special.IsEasy = false;
            Special.IsDifficult = true;
            break;

#ifdef VIRGIN_CHEAT_KEYS
        case PARM_PLAYTEST:
            Debug_Playtest = true;
            break;
#endif

#ifdef PARM_CHEATERIK
        case PARM_CHEATERIK:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEATADAM
        case PARM_CHEATADAM:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEATMIKE
        case PARM_CHEATMIKE:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEATDAVID
        case PARM_CHEATDAVID:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEATPHIL
        case PARM_CHEATPHIL:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEATBILL
        case PARM_CHEATBILL:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_CHEAT_STEVET

        case PARM_CHEAT_STEVET:
            Debug_Playtest = true;
            Debug_Flag = true;
            break;

#endif

#ifdef PARM_EDITORBILL
        case PARM_EDITORBILL:
            Debug_Map = true;
            Debug_Unshroud = true;
            Debug_Flag = true;
            break;
#endif

#ifdef PARM_EDITORERIK
        case PARM_EDITORERIK:
            Debug_Map = true;
            Debug_Unshroud = true;
            Debug_Flag = true;
            break;
#endif

        case PARM_SPECIAL:
            Special.IsJurassic = true;
            AreThingiesEnabled = true;
            break;

        /*
        ** Special flag - is C&C being run from the install program?
        */
        case PARM_INSTALL:
#ifndef DEMO
            Special.IsFromInstall = true;
#endif
            break;

        default:
            processed = false;
            break;
        }
        if (processed)
            continue;

#ifdef CHEAT_KEYS
        /*
        **	Scenario Editor Mode
        */
        if (stricmp(string, "-CHECKMAP") == 0) {
            Debug_Check_Map = true;
            continue;
        }

#endif

        /*
        **	Older version override.
        */
        if (stricmp(string, "-O") == 0 || stricmp(string, "-0") == 0) {
            IsV107 = true;
            continue;
        }

        /*
        **	File search path override.
        */
        if (strstr(string, "-CD")) {
            CCFileClass::Set_Search_Drives(&string[3]);
            continue;
        }
#ifdef JAPANESE
        /*
        ** Enable english-compatible keyboard
        */
        if (!stricmp(string, "-ENGLISH")) {
            ForceEnglish = true;
            continue;
        }
#endif

        /*
        **	Specify destination connection for network play
        */
        if (strstr(string, "-DESTNET")) {
            NetNumType net;
            NetNodeType node;

            /*
            ** Scan the command-line string, pulling off each address piece
            */
            int i = 0;
            char* p = strtok(string + 8, ".");
            while (p) {
                int x;

                sscanf(p, "%x", &x); // convert from hex string to int
                if (i < 4) {
                    net[i] = (char)x; // fill NetNum
                } else {
                    node[i - 4] = (char)x; // fill NetNode
                }
                i++;
                p = strtok(NULL, ".");
            }

            /*
            ** If all the address components were successfully read, fill in the
            ** BridgeNet with a broadcast address to the network across the bridge.
            */
            if (i >= 4) {
                IsBridge = 1;
                memset(node, 0xff, 6);
                BridgeNet = IPXAddressClass(net, node);
            }
            continue;
        }

        /*
        **	Specify socket ID, as an offset from 0x4000.
        */
        if (strstr(string, "-SOCKET")) {
            unsigned short socket;

            socket = (unsigned short)(atoi(string + strlen("SOCKET")));
            socket += 0x4000;
            if (socket >= 0x4000 && socket < 0x8000) {
                Ipx.Set_Socket(socket);
            }
            continue;
        }

        /*
        **	Set the Net Stealth option
        */
        if (strstr(string, "-STEALTH")) {
            NetStealth = true;
            continue;
        }

        /*
        **	Set the Net Protection option
        */
        if (strstr(string, "-MESSAGES")) {
            NetProtect = false;
            continue;
        }

        /*
        **	Allow "attract" mode
        */
        if (strstr(string, "-ATTRACT")) {
            AllowAttract = true;
            continue;
        }

#ifdef CHEAT_KEYS
        /*
        **	Allow solo net play
        */
        if (stricmp(string, "-HANSOLO") == 0) {
            MPlayerSolo = true;
            continue;
        }

        /*
        **	Specify the random number seed (for debugging)
        */
        if (strstr(string, "-SEED")) {
            CustomSeed = (unsigned short)(atoi(string + strlen("SEED")));
            continue;
        }
#endif

#ifdef NEVER
        /*
        **	Handle the prog init differently in this case.
        */
        if (strstr(string, "-V")) {
            continue;
        }
#endif

        /*
        **	Special command line control parsing.
        */
        if (strnicmp(string, "-X", strlen("-O")) == 0) {
            string += strlen("-X");
            while (*string) {
                char code = *string++;
                switch (toupper(code)) {

#ifdef ONHOLD
                /*
                **	Should human generated sound effects be used?
                */
                case 'J':
                    Special.IsJuvenile = true;
                    break;
#endif

#ifdef CHEAT_KEYS
                /*
                **	Monochrome debug screen enable.
                */
                case 'M':
                    Special.IsMonoEnabled = true;
                    break;

                /*
                **	Inert weapons -- no units take damage.
                */
                case 'I':
                    Special.IsInert = true;
                    break;
#endif

#ifdef CHEAT_KEYS
                /*
                **	Hussled recharge timer.
                */
                case 'H':
                    Special.IsSpeedBuild = true;
                    break;

                /*
                **	Turn on super-record mode, which thrashes your disk terribly,
                ** but is really really cool.  Well, sometimes it is, anyway.
                ** At least, it can be.  Once in a while.
                ** This flag tells the recording system to re-open the file for
                ** each write, so the recording survives a crash.
                */
                case 'S':
                    SuperRecord = 1;
                    break;

                /*
                **	"Record" a multi-player game
                */
                case 'X':
                    RecordGame = 1;
                    break;

                /*
                **	"Play Back" a multi-player game
                */
                case 'Y':
                    PlaybackGame = 1;
                    break;
#endif

#ifdef ONHOLD
                /*
                **	Bonus scenario enable.
                */
                case 'Z':
                    Special.IsJurassic = true;
                    break;
#endif

                /*
                **	Quiet mode override control.
                */
                case 'Q':
                    Debug_Quiet = true;
                    break;

#ifdef CHEAT_KEYS
                /*
                **	Target selection by human opponent (network/modem play) will
                **	be visible to the player?
                */
                case 'V':
                    Special.IsVisibleTarget = true;
                    break;
#endif

                default:
#ifdef GERMAN
                    puts("Ung?ltiger Parameter.\n");
#else
#ifdef FRENCH
                    puts("Commande d'option invalide.\n");
#else
                    puts("Invalid option switch.\n");
#endif
#endif
                    return (false);
                }
            }

            if (Special.IsMonoEnabled) {
                MonoClass::Enable();
            }
            continue;
        }
    }
    return (true);
}

#ifdef ONHOLD
/***********************************************************************************************
 * Parse_INI_File -- Parses CONQUER.INI for certain options                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		none. *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		none. *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		none. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/18/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void Parse_INI_File(void)
{
    char* buffer; // INI staging buffer pointer.
    char buf[128];
    static char section[40];
    static char entry[40];
    static char name[40];
    int len;
    int i;

    /*
    ** These arrays store the coded version of the names Geologic, Period, & Jurassic.
    ** Decode them by subtracting 83.  For you curious types, the names look like:
    ** ��¿º��
    ** ��ż·
    ** ?�Ŵ�Ƽ�
    ** If these INI entries aren't found, the IsJurassic flag does nothing.
    */
    static char coded_section[] = {154, 184, 194, 191, 194, 186, 188, 182, 0};
    static char coded_entry[] = {163, 184, 197, 188, 194, 183, 0};
    static char coded_name[] = {157, 200, 197, 180, 198, 198, 188, 182, 0};

    /*------------------------------------------------------------------------
    Fetch working pointer to the INI staging buffer. Make sure that the buffer
    is cleared out before proceeding.
    ------------------------------------------------------------------------*/
    buffer = (char*)_ShapeBuffer;
    memset(buffer, '\0', _ShapeBufferSize);

    /*------------------------------------------------------------------------
    Decode the desired section, entry, & name
    ------------------------------------------------------------------------*/
    strcpy(section, coded_section);
    len = strlen(coded_section);
    for (i = 0; i < len; i++) {
        section[i] -= 83;
    }

    strcpy(entry, coded_entry);
    len = strlen(coded_entry);
    for (i = 0; i < len; i++) {
        entry[i] -= 83;
    }

    strcpy(name, coded_name);
    len = strlen(coded_name);
    for (i = 0; i < len; i++) {
        name[i] -= 83;
    }

    /*------------------------------------------------------------------------
    Create filename and read the file.
    ------------------------------------------------------------------------*/
    CCFileClass file("CONQUER.INI");
    if (!file.Is_Available()) {
        return;
    } else {
        file.Read(buffer, _ShapeBufferSize - 1);
    }
    file.Close();

    WWGetPrivateProfileString(section, entry, "", buf, sizeof(buf), buffer);

    if (!stricmp(buf, name))
        AreThingiesEnabled = true;

    memset(section, 0, sizeof(section));
    memset(entry, 0, sizeof(entry));
    memset(name, 0, sizeof(name));
}
#endif

/***********************************************************************************************
 * Version_Number -- Determines the version number.                                            *
 *                                                                                             *
 *    This routine will determine the version number by analyzing the date and teim that the   *
 *    program was compiled and then generating a unique version number based on it. The        *
 *    version numbers are guaranteed to be larger for later dates.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the version number.                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int Version_Number(void)
{
    // Only print the git tag version number if it starts with 'v'
    if (*GitTag == '\0' || GitUncommittedChanges || *GitTag != 'v') {
        snprintf(VersionText,
                 sizeof(VersionText),
                 "r%d %s%s",
                 GitRevision,
                 (GitUncommittedChanges ? "~" : ""),
                 GitShortSHA1);
    } else {
        snprintf(VersionText, sizeof(VersionText), "%s %s%s", GitTag, (GitUncommittedChanges ? "~" : ""), GitShortSHA1);
    }

    return (0x105);
}

/***********************************************************************************************
 * Init_CDROM_Access -- Initialize the CD-ROM access handler.                                  *
 *                                                                                             *
 *    This routine is called to setup the CD-ROM access or emulation handler. It will ensure   *
 *    that the appropriate CD-ROM is present (dependant on the RequiredCD global).             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The fonts, palettes, and other bootstrap systems must have been initialized     *
 *             prior to calling this routine since this routine will quite likely display      *
 *             a dialog box requesting the appropriate CD be inserted.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Init_CDROM_Access(void)
{
    VisiblePage.Clear();
    HidPage.Clear();

    /*
    **	Always try to look at the CD-ROM for data files.
    **
    **	This call is needed because of a side effect of this function. It will examine the
    **	CD-ROMs attached to this computer and set the appropriate status values. Without this
    **	call, the "?:\\" could not be filled in correctly.
    */
    RequiredCD = Force_CD_Available(-1) ? -1 : -2;
}

/***************************************************************************
 * Save_Recording_Values -- Saves recording values to a recording file     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/15/1995 BRR : Created.                                             *
 *=========================================================================*/
void Save_Recording_Values(void)
{
    RecordFile.Write(&GameToPlay, sizeof(GameToPlay));
    RecordFile.Write(&ModemGameToPlay, sizeof(ModemGameToPlay));
    RecordFile.Write(&BuildLevel, sizeof(BuildLevel));
    RecordFile.Write(MPlayerName, sizeof(MPlayerName));
    RecordFile.Write(&MPlayerPrefColor, sizeof(MPlayerPrefColor));
    RecordFile.Write(&MPlayerColorIdx, sizeof(MPlayerColorIdx));
    RecordFile.Write(&MPlayerHouse, sizeof(MPlayerHouse));
    RecordFile.Write(&MPlayerLocalID, sizeof(MPlayerLocalID));
    RecordFile.Write(&MPlayerCount, sizeof(MPlayerCount));
    RecordFile.Write(&MPlayerBases, sizeof(MPlayerBases));
    RecordFile.Write(&MPlayerCredits, sizeof(MPlayerCredits));
    RecordFile.Write(&MPlayerTiberium, sizeof(MPlayerTiberium));
    RecordFile.Write(&MPlayerGoodies, sizeof(MPlayerGoodies));
    RecordFile.Write(&MPlayerGhosts, sizeof(MPlayerGhosts));
    RecordFile.Write(&MPlayerUnitCount, sizeof(MPlayerUnitCount));
    RecordFile.Write(MPlayerID, sizeof(MPlayerID));
    RecordFile.Write(MPlayerHouses, sizeof(MPlayerHouses));
    RecordFile.Write(&Seed, sizeof(Seed));
    RecordFile.Write(&Scenario, sizeof(Scenario));
    RecordFile.Write(&ScenPlayer, sizeof(ScenPlayer));
    RecordFile.Write(&ScenDir, sizeof(ScenDir));
    RecordFile.Write(&Whom, sizeof(Whom));
    RecordFile.Write(&Special, sizeof(SpecialClass));
    RecordFile.Write(&Options, sizeof(GameOptionsClass));
    RecordFile.Write(&FrameSendRate, sizeof(FrameSendRate));
    RecordFile.Write(&CommProtocol, sizeof(CommProtocol));

    if (SuperRecord) {
        RecordFile.Close();
    }
}

/***************************************************************************
 * Load_Recording_Values -- Loads recording values from recording file     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/15/1995 BRR : Created.                                             *
 *=========================================================================*/
void Load_Recording_Values(void)
{
    Read_MultiPlayer_Settings();

    RecordFile.Read(&GameToPlay, sizeof(GameToPlay));
    RecordFile.Read(&ModemGameToPlay, sizeof(ModemGameToPlay));
    RecordFile.Read(&BuildLevel, sizeof(BuildLevel));
    RecordFile.Read(MPlayerName, sizeof(MPlayerName));
    RecordFile.Read(&MPlayerPrefColor, sizeof(MPlayerPrefColor));
    RecordFile.Read(&MPlayerColorIdx, sizeof(MPlayerColorIdx));
    RecordFile.Read(&MPlayerHouse, sizeof(MPlayerHouse));
    RecordFile.Read(&MPlayerLocalID, sizeof(MPlayerLocalID));
    RecordFile.Read(&MPlayerCount, sizeof(MPlayerCount));
    RecordFile.Read(&MPlayerBases, sizeof(MPlayerBases));
    RecordFile.Read(&MPlayerCredits, sizeof(MPlayerCredits));
    RecordFile.Read(&MPlayerTiberium, sizeof(MPlayerTiberium));
    RecordFile.Read(&MPlayerGoodies, sizeof(MPlayerGoodies));
    RecordFile.Read(&MPlayerGhosts, sizeof(MPlayerGhosts));
    RecordFile.Read(&MPlayerUnitCount, sizeof(MPlayerUnitCount));
    RecordFile.Read(MPlayerID, sizeof(MPlayerID));
    RecordFile.Read(MPlayerHouses, sizeof(MPlayerHouses));
    RecordFile.Read(&Seed, sizeof(Seed));
    RecordFile.Read(&Scenario, sizeof(Scenario));
    RecordFile.Read(&ScenPlayer, sizeof(ScenPlayer));
    RecordFile.Read(&ScenDir, sizeof(ScenDir));
    RecordFile.Read(&Whom, sizeof(Whom));
    RecordFile.Read(&Special, sizeof(SpecialClass));
    RecordFile.Read(&Options, sizeof(GameOptionsClass));
    RecordFile.Read(&FrameSendRate, sizeof(FrameSendRate));
    RecordFile.Read(&CommProtocol, sizeof(CommProtocol));
}

/***********************************************************************************************
 * Obfuscate -- Sufficiently transform parameter to thwart casual hackers.                     *
 *                                                                                             *
 *    This routine borrows from CRC and PGP technology to sufficiently alter the parameter     *
 *    in order to make it difficult to reverse engineer the key phrase. This is designed to    *
 *    be used for hidden game options that will be released at a later time over Westwood's    *
 *    Web page or through magazine hint articles.                                              *
 *                                                                                             *
 *    Since this is a one way transformation, it becomes much more difficult to reverse        *
 *    engineer the pass phrase even if the resultant pass code is known. This has an added     *
 *    benefit of making this algorithm immune to traditional cyrptographic attacks.            *
 *                                                                                             *
 *    The largest strength of this transformation algorithm lies in the restriction on the     *
 *    source vector being legal ASCII uppercase characters. This restriction alone makes even  *
 *    a simple CRC transformation practically impossible to reverse engineer. This algorithm   *
 *    uses far more than a simple CRC transformation to achieve added strength from advanced   *
 *    attack methods.                                                                          *
 *                                                                                             *
 * INPUT:   string   -- Pointer to the key phrase that will be transformed into a code.        *
 *                                                                                             *
 * OUTPUT:  Returns with the code that the key phrase is translated into.                      *
 *                                                                                             *
 * WARNINGS:   A zero length pass phrase results in a 0x00000000 result code.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
long Obfuscate(char const* string)
{
    char buffer[1024];

    if (!string)
        return (0);
    memset(buffer, '\xA5', sizeof(buffer));

    /*
    **	Copy key phrase into a working buffer. This hides any transformation done
    **	to the string.
    */
    strncpy(buffer, string, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    int length = (int)strlen(buffer);

    /*
    **	Only upper case letters are significant.
    */
    strupr(buffer);

    /*
    **	Ensure that only visible ASCII characters compose the key phrase. This
    **	discourages the direct forced illegal character input method of attack.
    */
    int index;
    for (index = 0; index < length; index++) {
        if (!isgraph(buffer[index])) {
            buffer[index] = 'A' + (index % 26);
        }
    }

    /*
    **	Increase the strength of even short pass phrases by extending the
    **	length to be at least a minimum number of characters. This helps prevent
    **	a weak pass phrase from compromising the obfuscation process. This
    **	process also forces the key phrase to be an even multiple of four.
    **	This is necessary to support the cypher process that occurs later.
    */
    if (length < 16 || (length & 0x03)) {
        int maxlen = 16;
        if (((length + 3) & 0x00FC) > maxlen) {
            maxlen = ((length + 3) & 0x00FC);
        }
        for (index = length; index < maxlen; index++) {
            buffer[index] = 'A' + ((('?' ^ buffer[index - length]) + index) % 26);
        }
        length = index;
        buffer[length] = '\0';
    }

    /*
    **	Transform the buffer into a number. This transformation is character
    **	order dependant.
    */
    long code = Calculate_CRC(buffer, length);

    /*
    **	Record a copy of this initial transformation to be used in a later
    **	self referential transformation.
    */
    long copy = code;

    /*
    **	Reverse the character string and combine with the previous transformation.
    **	This doubles the workload of trying to reverse engineer the CRC calculation.
    */
    strrev(buffer);
    code ^= Calculate_CRC(buffer, length);

    /*
    **	Perform a self referential transformation. This makes a reverse engineering
    **	by using a cause and effect attack more difficult.
    */
    code = code ^ copy;

    /*
    **	Unroll and combine the code value into the pass phrase and then perform
    **	another self referential transformation. Although this is a trivial cypher
    **	process, it gives the sophisticated hacker false hope since the strong
    **	cypher process occurs later.
    */
    strrev(buffer); // Restore original string order.
    for (index = 0; index < length; index++) {
        code ^= (unsigned char)buffer[index];
        unsigned char temp = (unsigned char)code;
        buffer[index] ^= temp;
        code >>= 8;
        code |= (((long)temp) << 24);
    }

    /*
    **	Introduce loss into the vector. This strengthens the key against traditional
    **	cryptographic attack engines. Since this also weakens the key against
    **	unconventional attacks, the loss is limited to less than 10%.
    */
    for (index = 0; index < length; index++) {
        static unsigned char _lossbits[] = {0x00, 0x08, 0x00, 0x20, 0x00, 0x04, 0x10, 0x00};
        static unsigned char _addbits[] = {0x10, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x04};

        buffer[index] |= _addbits[index % (sizeof(_addbits) / sizeof(_addbits[0]))];
        buffer[index] &= ~_lossbits[index % (sizeof(_lossbits) / sizeof(_lossbits[0]))];
    }

    /*
    **	Perform a general cypher transformation on the vector
    **	and use the vector itself as the cypher key. This is a variation on the
    **	cypher process used in PGP. It is a very strong cypher process with no known
    **	weaknesses. However, in this case, the cypher key is the vector itself and this
    **	opens up a weakness against attacks that have access to this transformation
    **	algorithm. The sheer workload of reversing this transformation should be enough
    **	to discourage even the most determined hackers.
    */
    for (index = 0; index < length; index += 4) {
        short key1 = buffer[index];
        short key2 = buffer[index + 1];
        short key3 = buffer[index + 2];
        short key4 = buffer[index + 3];
        short val1 = key1;
        short val2 = key2;
        short val3 = key3;
        short val4 = key4;

        val1 *= key1;
        val2 += key2;
        val3 += key3;
        val4 *= key4;

        short s3 = val3;
        val3 ^= val1;
        val3 *= key1;
        short s2 = val2;
        val2 ^= val4;
        val2 += val3;
        val2 *= key3;
        val3 += val2;

        val1 ^= val2;
        val4 ^= val3;

        val2 ^= s3;
        val3 ^= s2;

        buffer[index] = (char)val1;
        buffer[index + 1] = (char)val2;
        buffer[index + 2] = (char)val3;
        buffer[index + 3] = (char)val4;
    }

    /*
    **	Convert this final vector into a cypher key code to be
    **	returned by this routine.
    */
    code = Calculate_CRC(buffer, length);

    /*
    **	Return the final code value.
    */
    return (code);
}