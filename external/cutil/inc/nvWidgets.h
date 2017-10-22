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
// nvWidgets.h - User Interface library
//
//
// Author: Ignacio Castano, Samuel Gateau, Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef NV_WIDGETS_H
#define NV_WIDGETS_H

#include <GL/glut.h>
#include <time.h>   // clock
#include <nvMath.h>

#ifdef WIN32
#ifdef NVWIDGETS_EXPORTS
#define NVSDKENTRY __declspec(dllexport)
#else
#define NVSDKENTRY __declspec(dllimport)
#endif
#endif


namespace nv
{
    struct Point
    {
        NVSDKENTRY Point() : x(0), y(0) {}
        NVSDKENTRY Point(int ix, int iy) : x(ix), y(iy) {}
        NVSDKENTRY Point(const Point & p) : x(p.x), y(p.y) {}

        NVSDKENTRY const Point& operator= (const Point & p) { this->x = p.x; this->y = p.y; return *this; }

        int x, y;
    };

    struct Rect
    {
        NVSDKENTRY Rect() : x(0), y(0), w(0), h(0) {}
        NVSDKENTRY Rect(const Point & p) : x(p.x), y(p.y), w(0), h(0) {}
        NVSDKENTRY Rect(int ix, int iy, int iw = 0, int ih = 0) : x(ix), y(iy), w(iw), h(ih) {}
        NVSDKENTRY Rect(const Rect & r) : x(r.x), y(r.y), w(r.w), h(r.h) {}

        NVSDKENTRY const Rect& operator= (const Rect & r) { this->x = r.x; this->y = r.y; this->w = r.w; this->h = r.h; return *this; }

        int x, y;
        int w, h;

        NVSDKENTRY static const Rect null;
    };

    enum ButtonFlags
    {
        ButtonFlags_Off = 0x0,
        ButtonFlags_On = 0x1,
        ButtonFlags_Begin = 0x2,
        ButtonFlags_End = 0x4,
        ButtonFlags_Shift = 0x8,
        ButtonFlags_Alt = 0x10,
        ButtonFlags_Ctrl = 0x20,
    };

    struct ButtonState
    {
        int state;
        time_t time;
        Point cursor;
    };

    // An enum to identify the mouse buttons
    enum MouseButton
    {
        MouseButton_Left,
        MouseButton_Middle,
        MouseButton_Right,
    };

    // An enum to identify the special key buttons not translated with ASCII codes
    enum Key
    {
        Key_F1 = 128,
        Key_F2,
        Key_F3,
        Key_F4,
        Key_F5,
        Key_F6,
        Key_F7,
        Key_F8,
        Key_F9,
        Key_F10,
        Key_F11,
        Key_F12,

        Key_Left,
        Key_Up,
        Key_Right,
        Key_Down,
        Key_PageUp,
        Key_PageDown,
        Key_Home,
        Key_End,
        Key_Insert,
    };

     // The various flags to modify the behavior of the groups   
    enum GroupFlags
    {
        // Layout behavior flags
        GroupFlags_LayoutNone = 0x01,
        GroupFlags_LayoutVertical = 0x02,
        GroupFlags_LayoutHorizontal = 0x04,
        GroupFlags_LayoutMask = 0x07,
        GroupFlags_LayoutXMask = 0xffff ^ GroupFlags_LayoutMask,

        // Alignment flags for the widgets inserted in the group
        GroupFlags_AlignLeft = 0x10,
        GroupFlags_AlignRight = 0x20,
        GroupFlags_AlignTop = 0x40,
        GroupFlags_AlignBottom = 0x80,
        GroupFlags_AlignMask = 0xf0,
        GroupFlags_AlignXMask = 0xffff ^ GroupFlags_AlignMask,

        // Start flags defining the starting origin of the group
        GroupFlags_StartLeft = 0x100,
        GroupFlags_StartRight = 0x200,
        GroupFlags_StartTop = 0x400,
        GroupFlags_StartBottom = 0x800,
        GroupFlags_StartMask = 0xf00,
        GroupFlags_StartXMask = 0xffff ^ GroupFlags_StartMask,

        // Optional flags
        GroupFlags_LayoutForce = 0x8000,
        GroupFlags_LayoutDefault = 0x4000,
        GroupFlags_LayoutNoMargin = 0x2000,
        GroupFlags_LayoutNoSpace = 0x1000,

        // Predefined configurations
        GroupFlags_GrowRightFromBottom   = GroupFlags_LayoutHorizontal    | GroupFlags_StartLeft   | GroupFlags_AlignLeft   | GroupFlags_StartBottom | GroupFlags_AlignBottom ,
        GroupFlags_GrowRightFromTop      = GroupFlags_LayoutHorizontal    | GroupFlags_StartLeft   | GroupFlags_AlignLeft   | GroupFlags_StartTop    | GroupFlags_AlignTop ,
        GroupFlags_GrowLeftFromBottom    = GroupFlags_LayoutHorizontal    | GroupFlags_StartRight  | GroupFlags_AlignRight  | GroupFlags_StartBottom | GroupFlags_AlignBottom,
        GroupFlags_GrowLeftFromTop       = GroupFlags_LayoutHorizontal    | GroupFlags_StartRight  | GroupFlags_AlignRight  | GroupFlags_StartTop    | GroupFlags_AlignTop,
        GroupFlags_GrowUpFromLeft        = GroupFlags_LayoutVertical      | GroupFlags_StartBottom | GroupFlags_AlignBottom | GroupFlags_StartLeft   | GroupFlags_AlignLeft ,
        GroupFlags_GrowUpFromRight       = GroupFlags_LayoutVertical      | GroupFlags_StartBottom | GroupFlags_AlignBottom | GroupFlags_StartRight  | GroupFlags_AlignRight ,
        GroupFlags_GrowDownFromLeft      = GroupFlags_LayoutVertical      | GroupFlags_StartTop    | GroupFlags_AlignTop    | GroupFlags_StartLeft   | GroupFlags_AlignLeft ,
        GroupFlags_GrowDownFromRight     = GroupFlags_LayoutVertical      | GroupFlags_StartTop    | GroupFlags_AlignTop    | GroupFlags_StartRight  | GroupFlags_AlignRight ,

        GroupFlags_LayoutDefaultFallback = GroupFlags_GrowDownFromLeft,
    };

    struct Group
    {
        Rect bounds;  // anchor point + width and height of the region
        int flags;   // group behavior 
        int margin; // border 
        int space;  // interior
    };

    //*************************************************************************
    // UIPainter
    class UIPainter
    {
    public:
        NVSDKENTRY UIPainter() {}

        NVSDKENTRY virtual void begin( const Rect& window ) { init(); }
        NVSDKENTRY virtual void end() {}

        // These methods should be called between begin/end

        NVSDKENTRY virtual void drawFrame(const Rect & r, int margin, int style) = 0;

        NVSDKENTRY virtual Rect getLabelRect(const Rect & r, const char * text, Rect & rt, int& nbLines) const = 0;
        NVSDKENTRY virtual void drawLabel(const Rect & r, const char * text, const Rect & rt, const int& nbLines, bool isHover, int style) = 0;
      
        NVSDKENTRY virtual Rect getButtonRect(const Rect & r, const char * text, Rect & rt) const = 0;
        NVSDKENTRY virtual void drawButton(const Rect & r, const char * text, const Rect & rt, bool isDown, bool isHover, bool isFocus, int style) = 0;
    
        NVSDKENTRY virtual Rect getCheckRect(const Rect & r, const char * text, Rect & rt, Rect & rc) const = 0;
        NVSDKENTRY virtual void drawCheckButton(const Rect & r, const char * text, const Rect & rt, const Rect & rr, bool isChecked, bool isHover, bool isFocus, int style) = 0;

        NVSDKENTRY virtual Rect getRadioRect(const Rect & r, const char * text, Rect & rt, Rect & rr) const = 0;
        NVSDKENTRY virtual void drawRadioButton(const Rect & r, const char * text, const Rect & rt, const Rect & rr, bool isOn, bool isHover, bool isFocus, int style) = 0;

        NVSDKENTRY virtual Rect getHorizontalSliderRect(const Rect & r, Rect& rs, float v, Rect& rc) const = 0;
        NVSDKENTRY virtual void drawHorizontalSlider(const Rect & r, Rect& rs, float v, Rect& rc, bool isHover, int style) = 0;

        NVSDKENTRY virtual Rect getItemRect(const Rect & r, const char * text, Rect & rt) const = 0;
        NVSDKENTRY virtual void drawListItem(const Rect & r, const char * text, const Rect & rt, bool isSelected, bool isHover, int style) = 0;
 
        NVSDKENTRY virtual Rect getListRect(const Rect & r, int numOptions, const char * options[], Rect& ri, Rect & rt) const = 0;
        NVSDKENTRY virtual void drawListBox(const Rect & r, int numOptions, const char * options[], const Rect& ri, const Rect & rt, int selected, int hovered, int style) = 0;
      
        NVSDKENTRY virtual Rect getComboRect(const Rect & r, int numOptions, const char * options[], int selected, Rect& rt, Rect& ra) const = 0;
        NVSDKENTRY virtual Rect getComboOptionsRect(const Rect & rCombo, int numOptions, const char * options[], Rect& ri, Rect & rit) const = 0;
        NVSDKENTRY virtual void drawComboBox(const Rect & rect, int numOptions, const char * options[], const Rect & rt, const Rect & ra, int selected, bool isHover, bool isFocus, int style) = 0;
        NVSDKENTRY virtual void drawComboOptions(const Rect & rect, int numOptions, const char * options[], const Rect& ri, const Rect & rit, int selected, int hovered, bool isHover, bool isFocus, int style) = 0;

        NVSDKENTRY virtual Rect getLineEditRect(const Rect & r, const char * text, Rect & rt) const = 0;
        NVSDKENTRY virtual void drawLineEdit(const Rect & r, const char * text, const Rect & rt, int caretPos, bool isSelected, bool isHover, int style) = 0;

        NVSDKENTRY virtual Rect getPanelRect(const Rect & r, const char * text, Rect& rt, Rect& ra) const = 0;
        NVSDKENTRY virtual void drawPanel(const Rect & rect, const char * text, const Rect & rt, const Rect & ra, bool isUnfold, bool isHover, bool isFocus, int style) = 0;

        NVSDKENTRY virtual Rect getTextureViewRect(const Rect & rect, Rect& rt) const = 0;
        NVSDKENTRY virtual void drawTextureView(const Rect & rect, const void* texID, const Rect& rt, const Rect & rz, int mipLevel, int style) = 0;

        // Eval widget dimensions
        NVSDKENTRY virtual int getCanvasMargin() const = 0;
        NVSDKENTRY virtual int getCanvasSpace() const = 0;
        NVSDKENTRY virtual int getFontHeight() const = 0;
        NVSDKENTRY virtual int getTextLineWidth(const char * text) const = 0;
        NVSDKENTRY virtual int getTextSize(const char * text, int& nbLines) const = 0;
        NVSDKENTRY virtual int getPickedCharNb(const char * text, const Point& at) const = 0;

        NVSDKENTRY virtual void drawDebugRect(const Rect & r) = 0;

    protected:
    
        NVSDKENTRY void init() {}
    };


    class UIContext
    {
    public:
        NVSDKENTRY UIContext( UIPainter& painter );

        //
        // UI method for processing window size events
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void reshape(int w, int h);
       
        //
        // Check if the UI is currently on Focus
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY bool isOnFocus() const { return m_uiOnFocus; }

        //
        // UI method for processing mouse events
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void mouse(int button, int state, int modifier, int x, int y);
        NVSDKENTRY void mouse(int button, int state, int x, int y) { mouse( button, state, 0, x, y); }
 
        //
        // UI method for processing mouse motion events
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void mouseMotion(int x, int y);

        //
        // UI method for processing key events
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void keyboard(unsigned char k, int x, int y);

        //
        // UI method for entering UI processing mode
        //
        //  This function must be used to begin the UI processing
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void begin();

        //
        // UI method for leaving UI processing mode
        //
        //  This function must be used to end the UI processing
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void end();

        ////////////////////////////////////////////////////////////////////////////
        //
        //  UI element processing
        //
        // The following methods provide the interface for rendering and querying
        // UI objects. These methods must be called between begin/end.
        ////////////////////////////////////////////////////////////////////////////

        //
        //  UI method for drawing a static text label
        //
        // rect - optionally provides a location and size for the label
        // text - Text to display for the label
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY void doLabel(const Rect & rect, const char * text, int style = 0);
        
        //
        //  UI method for rendering and processing a push button
        //
        // rect - optionally provides a location and size for the button
        // text - text to display on the button
        // state -  whether the button is depressed
        //          if state is NULL, the buttoin behave like a touch button
        //          else, the button behave like a togle button
        // style - optional style flag to modify the look
        //////////////////////////////////////////////////////////////////
        NVSDKENTRY bool doButton(const Rect & rect, const char * text, bool * state = NULL, int style = 0);
        
        NVSDKENTRY bool doCheckButton(const Rect & rect, const char * text, bool * state, int style = 0);

        NVSDKENTRY bool doRadioButton(int reference, const Rect & r, const char * text, int * value, int style = 0);

        NVSDKENTRY bool doHorizontalSlider(const Rect & rect, float min, float max, float * value, int style = 0);

        NVSDKENTRY bool doListItem(int index, const Rect & rect, const char * text, int * selected, int style = 0);
        NVSDKENTRY bool doListBox(const Rect & rect, int numOptions, const char * options[], int * selected, int style = 0);
        NVSDKENTRY bool doComboBox(const Rect & rect, int numOptions, const char * options[], int * selected, int style = 0);

        NVSDKENTRY bool doLineEdit(const Rect & rect, char * text, int maxTextLength, int * nbCharsReturned, int style = 0);
        
        NVSDKENTRY void beginGroup(int groupFlags = GroupFlags_LayoutDefault, const Rect& rect = Rect::null);
        NVSDKENTRY void endGroup();

        NVSDKENTRY void beginFrame(int groupFlags = GroupFlags_LayoutDefault, const Rect& rect = Rect::null, int style = 0);
        NVSDKENTRY void endFrame();

        NVSDKENTRY bool beginPanel(Rect & rect, const char * text, bool * isUnfold, int groupFlags = GroupFlags_LayoutDefault, int style = 0);
        NVSDKENTRY void endPanel();

        NVSDKENTRY int getGroupWidth() { return m_groupStack[m_groupIndex].bounds.w; }
        NVSDKENTRY int getGroupHeight() { return m_groupStack[m_groupIndex].bounds.h; }

        NVSDKENTRY int getCursorX() { return m_currentCursor.x;}
        NVSDKENTRY int getCursorY() { return m_currentCursor.y;}

        NVSDKENTRY const ButtonState& getMouseState( int button) { return m_mouseButton[button]; }

        NVSDKENTRY void doTextureView(const Rect & rect, const void* texID, Rect & zoomRect, int mipLevel = 0, int style = 0);

    protected:
        NVSDKENTRY UIPainter* getPainter() { return m_painter; }

    private:
        NVSDKENTRY void setCursor(int x, int y);

        NVSDKENTRY static bool overlap(const Rect & rect, const Point & p);

        NVSDKENTRY bool hasFocus(const Rect & rect);
        NVSDKENTRY bool isHover(const Rect & rect);

        NVSDKENTRY Rect placeRect(const Rect & r);

    private:
        UIPainter * m_painter;
        
        int m_groupIndex;
        Group m_groupStack[64];

        Rect m_window;

        Point m_currentCursor;
        ButtonState m_mouseButton[3];
        unsigned char m_keyBuffer[32];
        int m_nbKeys;

        int m_focusCaretPos;
        Point m_focusPoint;
        bool m_twoStepFocus;
        bool m_uiOnFocus;
    };

};





#endif  // NV_WIDGETS_H
