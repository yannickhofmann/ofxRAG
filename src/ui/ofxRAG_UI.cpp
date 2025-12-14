#include "ofxRAG_UI.h"
#include "embeddings/TextEmbedding_T5.h" // Include T5 header

#include "store/VectorStore_Cosine.h" // For default setup


void ofxRAG_UI::setup(ofxRAG& ragInstance, float x, float y) {
    rag = &ragInstance;

    textModelDropdown = std::make_shared<ofxDropdown>("Text Model");

    // Populate dropdowns with model names from the master plan
    textModelDropdown->add(std::string("T5"));

    // Add listeners
    textModelDropdown->ofxDropdown::selectedValue.addListener(this, &ofxRAG_UI::onTextModelChanged);

    // Setup GUI panel
    params.setName("ofxRAG Controls");
    params.add(textModelDropdown->getParameter());

    
    gui.setup(params);
    gui.setPosition(x, y);
    
    // --- Initial Default Setup ---
    // For now, let's log that a change is requested.
    // In the future, this is where we'd instantiate the models.
    ofLogNotice("ofxRAG_UI") << "UI setup complete. Select a model to instantiate.";
    
    // Example of setting a default store
    rag->setVectorStore(std::make_shared<VectorStore_Cosine>());
}

void ofxRAG_UI::draw() {
    gui.draw();
}

void ofxRAG_UI::onTextModelChanged(std::string& modelName) {
    ofLogNotice("ofxRAG_UI") << "Text model selection changed to: " << modelName;
    if (modelName == "T5") {
        rag->setTextEmbedder(std::make_shared<TextEmbedding_T5>());
    } else {
        // Handle other text models here later, or clear if none selected
        rag->setTextEmbedder(nullptr);
    }
}


