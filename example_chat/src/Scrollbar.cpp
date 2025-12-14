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

#include "Scrollbar.h"

Scrollbar::Scrollbar() {
    contentHeight = 0;
    scrollY = 0;
    isDragging = false;
    dragMouseY = 0;
    dragScrollY = 0;
    scrollbarWidth = 6;
    scrollbarGap = 4;
}

void Scrollbar::setViewport(const ofRectangle& viewport) {
    this->viewport = viewport;
}

void Scrollbar::setContentHeight(float contentHeight) {
    this->contentHeight = contentHeight;
}

void Scrollbar::draw() {
    if (contentHeight <= viewport.height) {
        return;
    }

    float trackX = viewport.getRight() - scrollbarGap - scrollbarWidth;
    float trackY = viewport.getTop() + scrollbarGap;
    float trackHeight = viewport.height - (2 * scrollbarGap);

    ofPushStyle();
    // Draw the scrollbar track.
    ofSetColor(50, 50, 50, 150);
    ofDrawRectangle(trackX, trackY, scrollbarWidth, trackHeight);
    
    // Calculate and draw the scrollbar thumb.
    float thumbHeight = std::max(20.f, trackHeight * (viewport.height / contentHeight));
    float thumbY = ofMap(scrollY, 0, contentHeight - viewport.height, trackY, trackY + trackHeight - thumbHeight, true);
    
    thumbRect.set(trackX, thumbY, scrollbarWidth, thumbHeight);

    ofSetColor(150, 150, 150, 200);
    ofDrawRectangle(thumbRect);
    ofPopStyle();
}

void Scrollbar::mouseScrolled(int y) {
    scrollY -= y * 20;
    scrollY = ofClamp(scrollY, 0, std::max(0.f, contentHeight - viewport.height));
}

void Scrollbar::mousePressed(int x, int y) {
    if (thumbRect.inside(x, y)) {
        isDragging = true;
        dragMouseY = y;
        dragScrollY = scrollY;
    }
}

void Scrollbar::mouseDragged(int x, int y) {
    if (isDragging) {
        float dy = y - dragMouseY;
        float trackHeight = viewport.height - (2 * scrollbarGap);
        float scrollableHeight = contentHeight - viewport.height;
        scrollY = dragScrollY + (dy / trackHeight) * scrollableHeight;
        scrollY = ofClamp(scrollY, 0, std::max(0.f, scrollableHeight));
    }
}

void Scrollbar::mouseReleased(int x, int y) {
    isDragging = false;
}

float Scrollbar::getScrollY() const {
    return scrollY;
}

void Scrollbar::scrollToBottom() {
    scrollY = std::max(0.f, contentHeight - viewport.height);
}
