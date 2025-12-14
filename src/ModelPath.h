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
 
#include "ofMain.h"
#include <string>

// Use OFXRAG_MODELS_RELATIVE_PATH defined in addon_config.mk
#ifndef OFXRAG_MODELS_RELATIVE_PATH
#define OFXRAG_MODELS_RELATIVE_PATH "models"
#endif

inline std::string ofxragModelsRoot() {
    return ofToDataPath(OFXRAG_MODELS_RELATIVE_PATH, true);
}

inline std::string ofxragJoinModelPath(const std::string& relative) {
    // a leading slash in relative path would make ofFilePath::join treat it as absolute
    std::string clean_relative = relative;
    if (!clean_relative.empty() && clean_relative[0] == '/') {
        clean_relative = clean_relative.substr(1);
    }
    return ofFilePath::join(ofxragModelsRoot(), clean_relative);
}
