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

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetWindowTitle("ofxRAG Text Search Example");
    ofSetVerticalSync(true);
    ofBackground(30);

    rag.setup(); // Initialize the ofxRAG instance

    // Setup ofxRAG UI
    ragUI.setup(rag, 20, 20);

    // Setup input GUI
    inputPanel.setup("Input/Search", "settings.xml", 20, 300);
    inputPanel.add(inputText.set("Text to embed", "This is some example text."));
    addTextButton.addListener(this, &ofApp::addTextButtonPressed);
    inputPanel.add(addTextButton.set("Add Text", false));
    clearStoreButton.addListener(this, &ofApp::clearStoreButtonPressed);
    inputPanel.add(clearStoreButton.set("Clear Store", false));
    inputPanel.add(searchQuery.set("Search query", "search for similar text"));
    inputPanel.add(topK.set("Top K Results", 5, 1, 20));
    searchButton.addListener(this, &ofApp::searchButtonPressed);
    inputPanel.add(searchButton.set("Search", false));
    
    // Set T5 as the default model
    std::string defaultTextModel = "T5";
    ragUI.onTextModelChanged(defaultTextModel);
    statusMessage = "Ready. T5 model selected by default.";
}

//--------------------------------------------------------------
void ofApp::update(){
    // Check if the Text Embedder is set
    if (!rag.getTextEmbedder()) {
        statusMessage = "Please select a Text Model in the UI.";
    } else {
        statusMessage = "Text Embedder is active. Store size: " + ofToString(rag.getStoreSize());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ragUI.draw();
    inputPanel.draw();

    ofDrawBitmapStringHighlight(statusMessage, 20, ofGetHeight() - 80);

    // Display search results
    ofDrawBitmapString("Search Results:", 20, ofGetHeight() - 60);
    float yOffset = ofGetHeight() - 40;
    if (lastSearchResults.empty()) {
        ofDrawBitmapString("No results or no search performed.", 20, yOffset);
    } else {
        for (const auto& result : lastSearchResults) {
            std::string display = "ID: " + ofToString(result.metadata.id) + 
                                  " | Sim: " + ofToString(result.distance, 4) + 
                                  " | Source: " + result.metadata.source;
            ofDrawBitmapString(display, 20, yOffset);
            yOffset += 15;
        }
    }
    
    // Draw some instructions
    ofDrawBitmapString("Drag and drop text files here to add their content to the store.\nTo use the Llama model, place a .gguf file at 'bin/data/models/llama-2-7b.gguf'", ofGetWidth() - 450, 20);
    ofDrawBitmapString("The T5 model is loaded from the addon's models directory and should work out of the box.", ofGetWidth() - 450, 60);
}


//--------------------------------------------------------------
void ofApp::addTextButtonPressed(bool& value) {
    if (!inputText.get().empty()) {
        rag.addText(inputText.get(), inputText.get().substr(0, 30) + "...");
        ofLogNotice("ofApp") << "Added text to store.";
    } else {
        ofLogWarning("ofApp") << "Input text is empty.";
    }
}

//--------------------------------------------------------------
void ofApp::clearStoreButtonPressed(bool& value) {
    rag.clearStore();
    lastSearchResults.clear();
    ofLogNotice("ofApp") << "Store cleared.";
}

//--------------------------------------------------------------
void ofApp::searchButtonPressed(bool& value) {
    if (!searchQuery.get().empty()) {
        lastSearchResults = rag.searchText(searchQuery.get(), topK.get());
        ofLogNotice("ofApp") << "Search performed. Found " << lastSearchResults.size() << " results.";
    } else {
        ofLogWarning("ofApp") << "Search query is empty.";
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    for(auto& file : dragInfo.files) {
        ofBuffer buffer = ofBufferFromFile(file);
        if(buffer.size() > 0) {
            std::string content = buffer.getText();
            if(!content.empty()) {
                rag.addText(content, file);
                ofLogNotice("ofApp") << "Dragged and added: " << file;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}
