#pragma once

#include "ofMain.h"
#include "ofxRAG.h"
#include "ofxRAG_UI.h"
#include "ofxGui.h" // For a simple text input for adding content

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
        ofxRAG rag;
        ofxRAG_UI ragUI;

        ofxPanel inputPanel;
        ofParameter<std::string> inputText;
        ofParameter<bool> addTextButton;
        ofParameter<bool> clearStoreButton;
        ofParameter<std::string> searchQuery;
        ofParameter<bool> searchButton;
        ofParameter<int> topK;

        std::vector<SearchResult> lastSearchResults;
        std::string statusMessage;

        void addTextButtonPressed(bool& value);
        void clearStoreButtonPressed(bool& value);
        void searchButtonPressed(bool& value);
};
