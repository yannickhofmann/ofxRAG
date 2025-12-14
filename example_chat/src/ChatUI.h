/*
 * ofxLlamaCpp
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
#include "AppTypes.h"
#include "Scrollbar.h"

class ChatUI {
public:
    ChatUI();
    void setup(const std::string &fontPath, int fontSize);
    void draw(const ofRectangle &viewport, const std::vector<ChatMessage> &history, const std::string &currentInput, const AppState &appState, bool isModelReady, float contextFillRatio);

    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseReleased(int x, int y, int button);

    void scrollToBottom();

private:
    std::string wrapText(const std::string &text, float maxWidth);

    ofTrueTypeFont font;
    Scrollbar scrollbar;

    // --- Layout Constants ---
    float chatAreaOuterPadding;
    float chatAreaBottomOffset;
    float textInnerPadding;
    float interMessageSpacing;
};
