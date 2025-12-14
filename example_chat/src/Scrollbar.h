/*
 * ofxRAG
 *
 * Copyright (c) 2025 Yannick Hofmann
 * <contact@yannickhofmann.de>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once
#include "ofMain.h"

class Scrollbar {
public:
    Scrollbar();

    void setViewport(const ofRectangle& viewport);
    void setContentHeight(float contentHeight);

    void draw();
    
    void mouseScrolled(int y);
    void mousePressed(int x, int y);
    void mouseDragged(int x, int y);
    void mouseReleased(int x, int y);

    float getScrollY() const;
    void scrollToBottom();

private:
    ofRectangle viewport;
    float contentHeight;
    float scrollY;
    
    bool isDragging;
    float dragMouseY;
    float dragScrollY;

    ofRectangle thumbRect;
    
    float scrollbarWidth;
    float scrollbarGap;
};
