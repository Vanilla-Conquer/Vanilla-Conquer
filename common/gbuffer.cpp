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

/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S   **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood 32 bit Library                  *
 *                                                                         *
 *                    File Name : GBUFFER.CPP                              *
 *                                                                         *
 *                   Programmer : Phil W. Gorrow                           *
 *                                                                         *
 *                   Start Date : May 3, 1994                              *
 *                                                                         *
 *                  Last Update : October 9, 1995   []                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   VVPC::VirtualViewPort -- Default constructor for a virtual viewport   *
 *   VVPC:~VirtualViewPortClass -- Destructor for a virtual viewport       *
 *   VVPC::Clear -- Clears a graphic page to correct color                 *
 *   VBC::VideoBufferClass -- Lowlevel constructor for video buffer class  *
 *   GVPC::Change -- Changes position and size of a Graphic View Port      *
 *   VVPC::Change -- Changes position and size of a Video View Port        *
 *   Set_Logic_Page -- Sets LogicPage to new buffer                        *
 *   GBC::DD_Init -- Inits a direct draw surface for a GBC                 *
 *   GBC::Init -- Core function responsible for initing a GBC              *
 *   GBC::Lock -- Locks a Direct Draw Surface                              *
 *   GBC::Unlock -- Unlocks a direct draw surface                          *
 *   GBC::GraphicBufferClass -- Default constructor (requires explicit init)*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gbuffer.h"
#include "video.h"
#include "drawbuff.h"
#include "ww_win.h"
#include "font.h"

#include <cstdlib>
#include <algorithm>

int TotalLocks = 0;

/*=========================================================================*/
/* The following PRIVATE functions are in this file:                       */
/*=========================================================================*/

/*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

/***************************************************************************
 * GVPC::GRAPHICVIEWPORTCLASS -- Constructor for basic view port class     *
 *                                                                         *
 * INPUT:      GraphicBufferClass * gbuffer - buffer to attach to          *
 *             int x                        - x offset into buffer         *
 *             int y                        - y offset into buffer         *
 *             int w                        - view port width in pixels    *
 *             int h                        - view port height in pixels   *
 *                                                                         *
 * OUTPUT:     Constructors may not have a return value                    *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicViewPortClass::GraphicViewPortClass(GraphicBufferClass* gbuffer, int x, int y, int w, int h)
    : Offset(0)
    , Width(0)
    , Height(0)
    , XAdd(0)
    , XPos(0)
    , YPos(0)
    , Pitch(0)
    , GraphicBuff(nullptr)
    , IsHardware(false)
    , LockCount(0)
{
    Attach(gbuffer, x, y, w, h);
}

/***************************************************************************
 * GVPC::GRAPHICVIEWPORTCLASS -- Default constructor for view port class   *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicViewPortClass::GraphicViewPortClass()
    : Offset(0)
    , Width(0)
    , Height(0)
    , XAdd(0)
    , XPos(0)
    , YPos(0)
    , Pitch(0)
    , GraphicBuff(nullptr)
    , IsHardware(false)
    , LockCount(0)
{
}

/***************************************************************************
 * GVPC::~GRAPHICVIEWPORTCLASS -- Destructor for GraphicViewPortClass      *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     A destructor may not return a value.                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/10/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicViewPortClass::~GraphicViewPortClass(void)
{
}

/***************************************************************************
 * GVPC::ATTACH -- Attaches a viewport to a buffer class                   *
 *                                                                         *
 * INPUT:    GraphicBufferClass *g_buff  -  pointer to gbuff to attach to  *
 *           int x                       - x position to attach to         *
 *           int y                       - y position to attach to         *
 *           int w                       - width of the view port          *
 *           int h                       - height of the view port         *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/10/1994 PWG : Created.                                             *
 *=========================================================================*/
void GraphicViewPortClass::Attach(GraphicBufferClass* gbuffer, int x, int y, int w, int h)
{
    /*======================================================================*/
    /* Can not attach a Graphic View Port if it is actually the physical    */
    /*     representation of a Graphic Buffer.                              */
    /*======================================================================*/
    if (this == Get_Graphic_Buffer()) {
        return;
    }

    /*======================================================================*/
    /* Verify that the x and y coordinates are valid and placed within the  */
    /*      physical buffer.                                                */
    /*======================================================================*/
    if (x < 0)                         // you cannot place view port off
        x = 0;                         //   the left edge of physical buf
    if (x >= gbuffer->Get_Width())     // you cannot place left edge off
        x = gbuffer->Get_Width() - 1;  //   the right edge of physical buf
    if (y < 0)                         // you cannot place view port off
        y = 0;                         //   the top edge of physical buf
    if (y >= gbuffer->Get_Height())    // you cannot place view port off
        y = gbuffer->Get_Height() - 1; //   bottom edge of physical buf

    /*======================================================================*/
    /* Adjust the width and height of necessary                             */
    /*======================================================================*/
    if (x + w > gbuffer->Get_Width()) // if the x plus width is larger
        w = gbuffer->Get_Width() - x; //    than physical, fix width

    if (y + h > gbuffer->Get_Height()) // if the y plus height is larger
        h = gbuffer->Get_Height() - y; //   than physical, fix height

    /*======================================================================*/
    /* Get a pointer to the top left edge of the buffer.                    */
    /*======================================================================*/
    Offset = gbuffer->Get_Offset() + ((gbuffer->Get_Width() + gbuffer->Get_Pitch()) * y) + x;

    /*======================================================================*/
    /* Copy over all of the variables that we need to store.                */
    /*======================================================================*/
    XPos = x;
    YPos = y;
    XAdd = gbuffer->Get_Width() - w;
    Width = w;
    Height = h;
    Pitch = gbuffer->Get_Pitch();
    GraphicBuff = gbuffer;
    IsHardware = gbuffer->IsHardware;
}

/***************************************************************************
 * GVPC::CHANGE -- Changes position and size of a Graphic View Port        *
 *                                                                         *
 * INPUT:       int the new x pixel position of the graphic view port      *
 *              int the new y pixel position of the graphic view port      *
 *              int the new width of the viewport in pixels                *
 *              int the new height of the viewport in pixels               *
 *                                                                         *
 * OUTPUT:      BOOL whether the Graphic View Port could be sucessfully    *
 *                      resized.                                           *
 *                                                                         *
 * WARNINGS:   You may not resize a Graphic View Port which is derived     *
 *                      from a Graphic View Port Buffer,                   *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/14/1994 SKB : Created.                                             *
 *=========================================================================*/
bool GraphicViewPortClass::Change(int x, int y, int w, int h)
{
    /*======================================================================*/
    /* Can not change a Graphic View Port if it is actually the physical    */
    /*      representation of a Graphic Buffer.                             */
    /*======================================================================*/
    if (this == Get_Graphic_Buffer()) {
        return false;
    }

    /*======================================================================*/
    /* Since there is no allocated information, just re-attach it to the    */
    /*      existing graphic buffer as if we were creating the              */
    /*      GraphicViewPort.                                                */
    /*======================================================================*/
    Attach(Get_Graphic_Buffer(), x, y, w, h);
    return true;
}

/***************************************************************************
 * Draw_Rect -- Draws a rectangle to the LogicPage.                        *
 *                                                                         *
 *    This routine will draw a rectangle to the LogicPage.  The rectangle  *
 *    doesn't have to be aligned on the vertical or horizontal axis.  In   *
 *    fact, it doesn't even have to be a rectangle.  The "square" can be   *
 *    skewed.                                                              *
 *                                                                         *
 * INPUT:   x1_pixel, y1_pixel   -- One corner.                            *
 *                                                                         *
 *          x2_pixel, y2_pixel   -- The other corner.                      *
 *                                                                         *
 *          color                -- The color to draw the lines.           *
 *                                                                         *
 * OUTPUT:  none                                                           *
 *                                                                         *
 * WARNINGS:   None, but the rectangle will be clipped to the current      *
 *             draw line clipping rectangle.                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/20/1993 JLB : Created.                                             *
 *=========================================================================*/
void GraphicViewPortClass::Draw_Rect(int x1_pixel, int y1_pixel, int x2_pixel, int y2_pixel, unsigned char color)
{
    Lock();
    Draw_Line(x1_pixel, y1_pixel, x2_pixel, y1_pixel, color);
    Draw_Line(x1_pixel, y2_pixel, x2_pixel, y2_pixel, color);
    Draw_Line(x1_pixel, y1_pixel, x1_pixel, y2_pixel, color);
    Draw_Line(x2_pixel, y1_pixel, x2_pixel, y2_pixel, color);
    Unlock();
}

/***************************************************************************
 * GVPC::DRAW_STAMP -- stub function to draw a tile on a graphic view port *
 *                     This version clips the tile to a window             *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *    07/31/1995 BWG : Created.                                            *
 *=========================================================================*/
void GraphicViewPortClass::Draw_Stamp(void const* icondata,
                                      int icon,
                                      int x_pixel,
                                      int y_pixel,
                                      void const* remap,
                                      int clip_window)
{
    if (Lock()) {
        Buffer_Draw_Stamp_Clip(this,
                               icondata,
                               icon,
                               x_pixel,
                               y_pixel,
                               remap,
                               WindowList[clip_window][WINDOWX],
                               WindowList[clip_window][WINDOWY],
                               WindowList[clip_window][WINDOWWIDTH],
                               WindowList[clip_window][WINDOWHEIGHT]);
    }
    Unlock();
}

/***************************************************************************
 * GBC::DD_INIT -- Inits a direct draw surface for a GBC                   *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/09/1995     : Created.                                             *
 *=========================================================================*/
void GraphicBufferClass::DD_Init(GBC_Enum flags)
{
    VideoSurfacePtr = Video::Shared().CreateSurface(Width, Height, flags);

    Allocated = false;   // even if system alloced, dont flag it cuz
                         //   we dont want it freed.
    IsHardware = true;   // flag it as a video surface
    Offset = NOT_LOCKED; // flag it as unavailable for reading or writing
    LockCount = 0;       //  surface is not locked
}

void GraphicBufferClass::Attach_DD_Surface(GraphicBufferClass* attach_buffer)
{
    VideoSurfacePtr->AddAttachedSurface(attach_buffer->Get_DD_Surface());
}

/***************************************************************************
 * GBC::INIT -- Core function responsible for initing a GBC                *
 *                                                                         *
 * INPUT:      int      - the width in pixels of the GraphicBufferClass    *
 *             int      - the heigh in pixels of the GraphicBufferClass    *
 *             void *   - pointer to user supplied buffer (system will     *
 *                        allocate space if buffer is NULL)                *
 *             long     - size of the user provided buffer                 *
 *             GBC_Enum - flags if this is defined as a direct draw        *
 *                        surface                                          *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/09/1995     : Created.                                             *
 *=========================================================================*/
void GraphicBufferClass::Init(int w, int h, void* buffer, long size, GBC_Enum flags)
{
    Size = size; // find size of physical buffer
    Width = w;   // Record width of Buffer
    Height = h;  // Record height of Buffer

    //
    // If the surface we are creating is a direct draw object then
    //   we need to do a direct draw init.  Otherwise we will do
    //   a normal alloc.
    //
    if (flags & (GBC_VIDEOMEM | GBC_VISIBLE)) {
        DD_Init(flags);
    } else {
        if (buffer) {          // if buffer is specified
            Buffer = buffer;   //      point to it and mark
            Allocated = false; //      it as user allocated
        } else {
            if (!Size)
                Size = w * h;
            Buffer = new char[Size]; // otherwise allocate it and
            Allocated = true;        //     mark it system alloced
        }
        Offset = (long)Buffer; // Get offset to the buffer
        IsHardware = false;
    }

    Pitch = 0;          // Record width of Buffer
    XAdd = 0;           // Record XAdd of Buffer
    XPos = 0;           // Record XPos of Buffer
    YPos = 0;           // Record YPos of Buffer
    GraphicBuff = this; // Get a pointer to our self
}

/***********************************************************************************************
 * GBC::Un_Init -- releases the video surface belonging to this gbuffer                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/6/96 12:44PM ST : Created                                                              *
 *=============================================================================================*/

void GraphicBufferClass::Un_Init()
{
    if (IsHardware) {
        if (VideoSurfacePtr != nullptr) {
            while (LockCount) {
                Unlock();
            }

            delete VideoSurfacePtr;
            VideoSurfacePtr = nullptr;
        }
    }
}

/***************************************************************************
 * GBC::GRAPHICBUFFERCLASS -- Default constructor (requires explicit init) *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/09/1995     : Created.                                             *
 *=========================================================================*/
GraphicBufferClass::GraphicBufferClass()
    : VideoSurfacePtr(nullptr)
{
    GraphicBuff = this; // Get a pointer to our self
}

/***************************************************************************
 * GBC::GRAPHICBUFFERCLASS -- Constructor for fixed size buffers           *
 *                                                                         *
 * INPUT:      long size     - size of the buffer to create                *
 *             int w         - width of buffer in pixels (default = 320)   *
 *             int h         - height of buffer in pixels (default = 200)  *
 *             void *buffer  - a pointer to the buffer if any (optional)   *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/13/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicBufferClass::GraphicBufferClass(int w, int h, void* buffer, long size)
    : VideoSurfacePtr(nullptr)
{
    Init(w, h, buffer, size, GBC_NONE);
}
/*=========================================================================*
 * GBC::GRAPHICBUFFERCLASS -- inline constructor for GraphicBufferClass    *
 *                                                                         *
 * INPUT:      int w         - width of buffer in pixels (default = 320)   *
 *             int h         - height of buffer in pixels (default = 200)  *
 *             void *buffer  - a pointer to the buffer if any (optional)   *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/03/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicBufferClass::GraphicBufferClass(int w, int h, void* buffer)
    : VideoSurfacePtr(nullptr)
{
    Init(w, h, buffer, w * h, GBC_NONE);
}

/*====================================================================================*
 * GBC::GRAPHICBUFFERCLASS -- contructor for GraphicsBufferClass with special flags   *
 *                                                                                    *
 * INPUT:        int w          - width of buffer in pixels (default = 320)           *
 *               int h          - height of buffer in pixels (default = 200)          *
 *               void *buffer   - unused                                              *
 *               unsigned flags - flags for creation of special buffer types          *
 *                                GBC_VISIBLE - buffer is a visible screen surface    *
 *                                GBC_VIDEOMEM - buffer resides in video memory       *
 *                                                                                    *
 * OUTPUT:     none                                                                   *
 *                                                                                    *
 * HISTORY:                                                                           *
 *   09-21-95 04:19pm ST : Created                                                    *
 *====================================================================================*/
GraphicBufferClass::GraphicBufferClass(int w, int h, GBC_Enum flags)
    : VideoSurfacePtr(nullptr)
{
    Init(w, h, NULL, w * h, flags);
}

/*=========================================================================*
 * GBC::~GRAPHICBUFFERCLASS -- Destructor for the graphic buffer class     *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/03/1994 PWG : Created.                                             *
 *=========================================================================*/
GraphicBufferClass::~GraphicBufferClass()
{

    //
    // Release the direct draw surface if it exists
    //
    Un_Init();
}

/***************************************************************************
 * SET_LOGIC_PAGE -- Sets LogicPage to new buffer                          *
 *                                                                         *
 * INPUT:      GraphicBufferClass * the buffer we are going to set         *
 *                                                                         *
 * OUTPUT:     GraphicBufferClass * the previous buffer type               *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/23/1995 PWG : Created.                                             *
 *=========================================================================*/
GraphicViewPortClass* Set_Logic_Page(GraphicViewPortClass* ptr)
{
    GraphicViewPortClass* old = LogicPage;
    LogicPage = ptr;
    return (old);
}

/***************************************************************************
 * SET_LOGIC_PAGE -- Sets LogicPage to new buffer                          *
 *                                                                         *
 * INPUT:      GraphicBufferClass & the buffer we are going to set         *
 *                                                                         *
 * OUTPUT:     GraphicBufferClass * the previous buffer type               *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/23/1995 PWG : Created.                                             *
 *=========================================================================*/
GraphicViewPortClass* Set_Logic_Page(GraphicViewPortClass& ptr)
{
    GraphicViewPortClass* old = LogicPage;
    LogicPage = &ptr;
    return (old);
}

/***************************************************************************
 * GBC::LOCK -- Locks a Direct Draw Surface                                *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/09/1995     : Created.                                             *
 *   10/09/1995     : Code stolen from Steve Tall                          *
 *=========================================================================*/
extern void Colour_Debug(int call_number);
extern bool GameInFocus;

extern void Block_Mouse(GraphicBufferClass* buffer);
extern void Unblock_Mouse(GraphicBufferClass* buffer);

bool GraphicBufferClass::Lock()
{
    //
    // If its not a direct draw surface then the lock is always sucessful.
    //
    if (!IsHardware) {
        return (Offset != 0);
    }

    /*
    ** If the video surface pointer is null then return
    */
    if (VideoSurfacePtr == nullptr) {
        return false;
    }

    /*
    ** If we dont have focus then return failure
    */
    if (!GameInFocus) {
        return false;
    }

    //
    // If surface is already locked then inc the lock count and return true
    //
    if (LockCount) {
        LockCount++;
        return true;
    }

    //
    // If it isn't locked at all then we will have to request that Direct
    // Draw actually lock the surface.
    //

    if (VideoSurfacePtr == nullptr) {
        return false;
    }

    Block_Mouse(this);

    bool result = false;

    if (VideoSurfacePtr->LockWait()) {
        Offset = (unsigned long)VideoSurfacePtr->GetData();
        Pitch = VideoSurfacePtr->GetPitch();
        Pitch -= Width;
        LockCount++;  // increment count so we can track if
        TotalLocks++; // Total number of times we have locked (for debugging)
        result = true;
    }

    Unblock_Mouse(this);

    return result; // Return false because we couldnt lock or restore the surface
}

/***************************************************************************
 * GBC::UNLOCK -- Unlocks a direct draw surface                            *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/09/1995     : Created.                                             *
 *   10/09/1995     : Code stolen from Steve Tall                          *
 *=========================================================================*/

bool GraphicBufferClass::Unlock()
{
    //
    // If there is no lock count or this is not a direct draw surface
    // then just return true as there is no harm done.
    //
    if (!(LockCount && IsHardware)) {
        return true;
    }

    //
    // If lock count is directly equal to one then we actually need to
    // unlock so just give it a shot.
    //
    if (LockCount == 1 && VideoSurfacePtr) {
        Block_Mouse(this);
        if (VideoSurfacePtr->Unlock()) {
            Offset = NOT_LOCKED;
            LockCount--;
            Unblock_Mouse(this);
            return true;
        } else {
            Unblock_Mouse(this);
            return false;
        }
    }
    // Colour_Debug (0);
    LockCount--;
    return true;
}

bool GraphicBufferClass::IsAllocated() const
{
    if (VideoSurfacePtr == nullptr) {
        return false;
    }

    return VideoSurfacePtr->IsAllocated();
}

/***********************************************************************************************
 * GVPC::DD_Linear_Blit_To_Linear -- blit using the hardware blitter                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    destination vvpc                                                                  *
 *           x coord to blit from                                                              *
 *           y coord to blit from                                                              *
 *           x coord to blit to                                                                *
 *           y coord to blit to                                                                *
 *           width to blit                                                                     *
 *           height to blit                                                                    *
 *                                                                                             *
 * OUTPUT:   DD_OK if successful                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09-22-95 11:05am ST : Created                                                             *
 *=============================================================================================*/

bool GraphicViewPortClass::DD_Linear_Blit_To_Linear(GraphicViewPortClass& dest,
                                                    int source_x,
                                                    int source_y,
                                                    int dest_x,
                                                    int dest_y,
                                                    int width,
                                                    int height,
                                                    bool mask)

{
    Rect source_rectangle = {0};
    source_rectangle.X = source_x;
    source_rectangle.Y = source_y;
    source_rectangle.Width = width;
    source_rectangle.Height = height;

    Rect dest_rectangle = {0};
    dest_rectangle.X = dest_x;
    dest_rectangle.Y = dest_y;
    dest_rectangle.Width = width;
    dest_rectangle.Height = height;

    dest.GraphicBuff->Get_DD_Surface()->Blt(dest_rectangle, GraphicBuff->Get_DD_Surface(), source_rectangle, mask);

    return true;
}
