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
 
 /*
    ParamListGL
    - class derived from ParamList to do simple OpenGL rendering of a parameter list
    sgg 8/2001
*/

#ifndef PARAMGL_H
#define PARAMGL_H

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <param.h>

void beginWinCoords();
void endWinCoords();
void glPrint(int x, int y, const char *s, void *font);
void glPrintShadowed(int x, int y, const char *s, void *font, float *color);

class ParamListGL : public ParamList {
public:
    ParamListGL(const char *name = "");

    void Render(int x, int y, bool shadow = false);
    bool Mouse(int x, int y, int button=GLUT_LEFT_BUTTON, int state=GLUT_DOWN);
    bool Motion(int x, int y);
    void Special(int key, int x, int y);

    void SetFont(void *font, int height) { m_font = font; m_font_h = height; }

    void SetSelectedColor(float r, float g, float b) { m_text_color_selected = Color(r, g, b); }
    void SetUnSelectedColor(float r, float g, float b) { m_text_color_unselected = Color(r, g, b); }
    void SetBarColorInner(float r, float g, float b) { m_bar_color_inner = Color(r, g, b); }
    void SetBarColorOuter(float r, float g, float b) { m_bar_color_outer = Color(r, g, b); }

private:
    void *m_font;
    int m_font_h;       // font height

    int m_bar_x;        // bar start x position
    int m_bar_w;        // bar width
    int m_bar_h;        // bar height
    int m_text_x;       // text start x position
    int m_separation;   // bar separation in y
    int m_value_x;      // value text x position
    int m_bar_offset;   // bar offset in y

    int m_start_x, m_start_y;

    struct Color {
        Color(float _r, float _g, float _b) { r = _r; g = _g; b = _b; }
        float r, g, b;
    };

    Color m_text_color_selected;
    Color m_text_color_unselected;
    Color m_text_color_shadow;
    Color m_bar_color_outer;
    Color m_bar_color_inner;
};

#endif
