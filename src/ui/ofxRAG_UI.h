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
#include "ofxGui.h" // Assuming ofxDropdown might be part of the core GUI or a separate addon
#include "ofxDropdown.h" 
#include "ofxRAG.h"

class ofxRAG_UI {
public:
    void setup(ofxRAG& ragInstance, float x, float y);
    void draw();

    // Event listeners for dropdowns
    void onTextModelChanged(std::string& modelName);


private:
    ofxRAG* rag; // A pointer to the main RAG instance

    ofParameterGroup params;
    std::shared_ptr<ofxDropdown> textModelDropdown;

    
    ofxPanel gui;
};
