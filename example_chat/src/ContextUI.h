#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "Scrollbar.h"

class ContextUI {
public:
    void setup(const std::string& fontPath, int fontSize);
    void update(const std::vector<std::string>& sources);
    void draw(const ofRectangle& viewport);

    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseReleased(int x, int y, int button);

    void setViewport(const ofRectangle& viewport);
    bool isInside(int x, int y) const;

private:
    ofTrueTypeFont font;
    std::vector<std::string> sources;
    ofRectangle viewport;

    Scrollbar scrollbar;

    std::string getWrappedString(const std::string& text, float width);
};
