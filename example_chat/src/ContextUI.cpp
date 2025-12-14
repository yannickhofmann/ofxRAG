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

#include "ContextUI.h"

void ContextUI::setup(const std::string& fontPath, int fontSize) {
    font.load(fontPath, fontSize, true, true, true);
}

void ContextUI::update(const std::vector<std::string>& sources) {
    this->sources = sources;
}

void ContextUI::draw(const ofRectangle& viewport) {
    this->viewport = viewport;
    ofPushStyle();
    ofSetColor(30);
    ofFill();
    ofDrawRectRounded(viewport, 5);

    // Always draw the title
    ofSetColor(150);
    std::string title = "Knowledge Base (Drop your files here)";
    std::string wrappedTitle = getWrappedString(title, viewport.width - 20); // 20px padding
    ofRectangle titleBounds = font.getStringBoundingBox(wrappedTitle, 0, 0);
    font.drawString(wrappedTitle, viewport.x + 10, viewport.y + 20);

    // Draw the list of sources below the title
    float listStartY = viewport.y + 20 + titleBounds.height + 10;
    ofRectangle listViewport(viewport.x, listStartY, viewport.width, viewport.height - (listStartY - viewport.y));

    float contentHeight = 0;
    for (const auto& source : sources) {
        std::string displayText = ofFilePath::getFileName(source);
        contentHeight += font.getStringBoundingBox(displayText, 0, 0).height + 10;
    }

    scrollbar.setViewport(listViewport);
    scrollbar.setContentHeight(contentHeight);
    scrollbar.draw();

    float textY = listViewport.y - scrollbar.getScrollY();

    for (const auto& source : sources) {
        std::string displayText = ofFilePath::getFileName(source);
        ofRectangle bounds = font.getStringBoundingBox(displayText, 0, 0);
        ofSetColor(255);
        font.drawString(displayText, listViewport.x + 10, textY + bounds.height);
        textY += bounds.height + 10;
    }

    ofPopStyle();
}

std::string ContextUI::getWrappedString(const std::string& text, float width) {
    std::stringstream words(text);
    std::string word;
    
    std::string wrappedText;
    std::string currentLine;
    
    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        if (font.getStringBoundingBox(testLine, 0, 0).width > width) {
            wrappedText += currentLine + "\n";
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    
    wrappedText += currentLine;
    return wrappedText;
}

void ContextUI::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    if (viewport.inside(x, y)) {
        scrollbar.mouseScrolled(scrollY);
    }
}

void ContextUI::mousePressed(int x, int y, int button) {
    if (viewport.inside(x, y)) {
        scrollbar.mousePressed(x, y);
    }
}

void ContextUI::mouseDragged(int x, int y, int button) {
    scrollbar.mouseDragged(x, y);
}

void ContextUI::mouseReleased(int x, int y, int button) {
    scrollbar.mouseReleased(x, y);
}

void ContextUI::setViewport(const ofRectangle& viewport) {
    this->viewport = viewport;
}

bool ContextUI::isInside(int x, int y) const {
    return viewport.inside(x, y);
}
