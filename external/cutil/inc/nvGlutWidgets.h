/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
 
 //
// nvGlutWidgets
//
//  Adaptor classes to integrate the nvWidgets UI library with the GLUT windowing
// toolkit. The adaptors convert native GLUT UI data to native nvWidgets data. All
// adaptor classes are implemented as in-line code in this header. The adaptor
// defaults to using the standard OpenGL paintor implementation.
//
// Author: Ignacio Castano, Samuel Gateau, Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NV_GLUT_WIDGETS_H
#define NV_GLUT_WIDGETS_H

#include <GL/glut.h>
#include <nvGLWidgets.h>


namespace nv {

class GlutUIContext : public UIContext {

protected:

    bool _ownPainter;

public:

    //
    // Default UI constructor
    //
    //  Creates private OpenGL painter
    //////////////////////////////////////////////////////////////////
    GlutUIContext() :
      UIContext( *(new GLUIPainter()) ),
      _ownPainter(true) 
    {
    }

    //
    // Alternate UI constructor
    //
    //  Allows for overriding the standard painter
    //////////////////////////////////////////////////////////////////
    GlutUIContext(UIPainter& painter) : 
        UIContext( painter ),
        _ownPainter(false)
    {
    }

    //
    // UI  destructor
    //
    //  Destroy painter if it is private
    //////////////////////////////////////////////////////////////////
    ~GlutUIContext() {
        if (_ownPainter) delete getPainter();
    }

    //
    // UI method for processing GLUT mouse button events
    //
    //  Call this method from the glutMouseFunc callback, the
    // modifier parameter maps to glutGetModifiers.
    //////////////////////////////////////////////////////////////////
    virtual void mouse(int button, int state, int modifier, int x, int y) { 
        int modifierMask = 0;

        if ( button == GLUT_LEFT_BUTTON) button = MouseButton_Left;
        else if ( button == GLUT_MIDDLE_BUTTON) button = MouseButton_Middle;
        else if ( button == GLUT_RIGHT_BUTTON) button = MouseButton_Right;

        if ( modifier & GLUT_ACTIVE_ALT) modifierMask |= ButtonFlags_Alt;
        if ( modifier & GLUT_ACTIVE_SHIFT) modifierMask |= ButtonFlags_Shift;
        if ( modifier & GLUT_ACTIVE_CTRL) modifierMask |= ButtonFlags_Ctrl;

        if ( state == GLUT_DOWN) state = 1; else state = 0;

        UIContext::mouse( button, state, modifierMask, x, y);
    }

    //
    // UI method for processing key events
    //
    //  Call this method from the glutReshapeFunc callback
    //////////////////////////////////////////////////////////////////
    void specialKeyboard(int k, int x, int y) { UIContext::keyboard( translateKey(k), x, y); }

    //
    //  Translate non-ascii keys from GLUT to nvWidgets
    //////////////////////////////////////////////////////////////////
    unsigned char translateKey( int k )
    {
        switch (k)
        {
        case GLUT_KEY_F1 :
            return Key_F1;
        case GLUT_KEY_F2 :
            return Key_F2;
        case GLUT_KEY_F3 :
            return Key_F3;
        case GLUT_KEY_F4 :
            return Key_F4;
        case GLUT_KEY_F5 :
            return Key_F5;
        case GLUT_KEY_F6 :
            return Key_F6;
        case GLUT_KEY_F7 :
            return Key_F7;
        case GLUT_KEY_F8 :
            return Key_F8;
        case GLUT_KEY_F9 :
            return Key_F9;
        case GLUT_KEY_F10 :
            return Key_F10;
        case GLUT_KEY_F11 :
            return Key_F11;
        case GLUT_KEY_F12 :
            return Key_F12;
        case GLUT_KEY_LEFT :
            return Key_Left;
        case GLUT_KEY_UP :
            return Key_Up;
        case GLUT_KEY_RIGHT :
            return Key_Right;
        case GLUT_KEY_DOWN :
            return Key_Down;
        case GLUT_KEY_PAGE_UP :
            return Key_PageUp;
        case GLUT_KEY_PAGE_DOWN :
            return Key_PageDown;
        case GLUT_KEY_HOME :
            return Key_Home;
        case GLUT_KEY_END :
            return Key_End;
        case GLUT_KEY_INSERT :
            return Key_Insert;
        default:
            return 0;
        } 
    }
};

};



#endif
