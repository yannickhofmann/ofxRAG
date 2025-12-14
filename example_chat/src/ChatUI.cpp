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

#include "ChatUI.h"

// --- CONSTRUCTOR & SETUP ---

ChatUI::ChatUI() {
    // Initialize all layout-related constants to default values.
    chatAreaOuterPadding = 20;
    chatAreaBottomOffset = 110;
    textInnerPadding = 30;
    interMessageSpacing = 20;
}

void ChatUI::setup(const std::string& fontPath, int fontSize) {
    // Load the font for the UI. Anti-aliasing, full character set, and mipmaps are enabled for better rendering.
    font.load(fontPath, fontSize, true, true, true);
    ofLogNotice("ChatUI") << "UI setup complete, font loaded.";
}

// --- DRAWING LOGIC ---

void ChatUI::draw(const ofRectangle& viewport,
                  const std::vector<ChatMessage>& history,
                  const std::string& currentInput,
                  const AppState& appState,
                  bool isModelReady,
                  float contextFillRatio) {
    
    // The main GUI panel (ofxGui) is drawn in ofApp, not here.
    // We estimate its height to correctly position our chat window below it.
    float guiHeight = 150;
    float chatAreaTopOffset = guiHeight + 30;

    // Define the main rectangular area for displaying chat messages.
    ofRectangle chatArea(
        chatAreaOuterPadding,
        chatAreaTopOffset,
        viewport.width - 2 * chatAreaOuterPadding,
        viewport.height - chatAreaTopOffset - chatAreaBottomOffset
    );

    float totalContentHeight = textInnerPadding;
    if (!history.empty()) {
        for (size_t i = 0; i < history.size(); ++i) {
            const auto& msg = history[i];
            std::string role = msg.isUser ? "You: " : "LLM: ";
            if (msg.content.rfind("[Summarized", 0) == 0) role = "";
            std::string wrapped = wrapText(role + msg.content, chatArea.width - 2 * textInnerPadding);
            totalContentHeight += font.stringHeight(wrapped);
            if (i < history.size() - 1) {
                totalContentHeight += interMessageSpacing;
            }
        }
        totalContentHeight += textInnerPadding;
    }
    
    if (history.empty()) {
        totalContentHeight = chatArea.height;
    }

    scrollbar.setViewport(chatArea);
    scrollbar.setContentHeight(totalContentHeight);

    // --- Drawing the Chat Area ---

    ofPushStyle();
    ofNoFill();
    ofSetColor(ofColor::white);
    ofSetLineWidth(1);
    ofDrawRectangle(chatArea);
    ofPopStyle();

    glEnable(GL_SCISSOR_TEST);
    glScissor(chatArea.x + 1, ofGetHeight() - (chatArea.y + chatArea.height) + 1, chatArea.width - 2, chatArea.height - 2);

    ofPushMatrix();
    ofTranslate(chatArea.x + textInnerPadding, chatArea.y - scrollbar.getScrollY());

    float currentY = textInnerPadding;
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& msg = history[i];
        ofSetColor(msg.isUser ? ofColor::yellow : (msg.content.rfind("[Summarized", 0) == 0 ? ofColor::gray : ofColor::white));
        
        std::string role = msg.isUser ? "You: " : "LLM: ";
        if (msg.content.rfind("[Summarized", 0) == 0) role = "";
        
        std::string wrapped = wrapText(role + msg.content, chatArea.width - 2 * textInnerPadding - (scrollbar.getScrollY() > 0 ? 20 : 0));
        font.drawString(wrapped, 0, currentY);
        currentY += font.stringHeight(wrapped);
        if (i < history.size() - 1) {
            currentY += interMessageSpacing;
        }
    }

    ofPopMatrix();
    
    glDisable(GL_SCISSOR_TEST);

    scrollbar.draw();

    // --- Bottom UI Elements ---
    float bottomTextY = viewport.height - 80;
    ofSetColor(255);

    std::string status_text = "Status: CHATTING";
    if (appState == SUMMARIZING) status_text = "Status: SUMMARIZING...";
    if (appState == GENERATING_REPLY) status_text = "Status: GENERATING...";
    font.drawString(status_text, viewport.width - 250, bottomTextY);

    if (isModelReady) {
        font.drawString("CTX fill: " + ofToString(contextFillRatio * 100, 1) + "%", 20, bottomTextY);
    }
    
    font.drawString("Prompt (ENTER):", 20, viewport.height - 50);
    ofSetColor(ofColor::yellow);
    font.drawString("> " + currentInput, 20, viewport.height - 30);
}


// --- INTERACTION & HELPERS ---

void ChatUI::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    scrollbar.mouseScrolled(scrollY);
}

void ChatUI::mousePressed(int x, int y, int button) {
    scrollbar.mousePressed(x, y);
}

void ChatUI::mouseDragged(int x, int y, int button) {
    scrollbar.mouseDragged(x, y);
}

void ChatUI::mouseReleased(int x, int y, int button) {
    scrollbar.mouseReleased(x, y);
}

void ChatUI::scrollToBottom() {
    scrollbar.scrollToBottom();
}

std::string ChatUI::wrapText(const std::string &text, float maxWidth) {
    std::stringstream ss(text);
    std::string word;
    std::string line;
    std::string result;

    while (ss >> word) {
        std::string testLine = line + word + " ";
        if (font.stringWidth(testLine) > maxWidth) {
            result += line + "\n";
            line = word + " ";
        } else {
            line = testLine;
        }
    }
    result += line;
    return result;
}
