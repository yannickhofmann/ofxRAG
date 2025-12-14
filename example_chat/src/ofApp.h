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
#include "ofxGui.h"
#include "ofxLlamaCpp.h"
#include "ofxRAG.h" // Added for RAG capabilities
#include "embeddings/TextEmbedding_T5.h" // Specific embedder for T5
#include "store/VectorStore_Cosine.h" // Specific vector store for Cosine similarity
#include "minja/chat-template.hpp"
#include "ofxDropdown.h"
#include "nlohmann/json.hpp"
#include "ChatUI.h"
#include "AppTypes.h"
#include "TemplateManager.h"
#include "ContextUI.h"
#include <map>

// class ofApp
// The main application class that orchestrates the entire chat application.
//
// This class handles the application's state, user input, the Llama language model,
// and the user interface. It follows the openFrameworks application structure.
class ofApp : public ofBaseApp {
public:
    // Called once at the beginning of the application's lifecycle.
    void setup();
    
    // Called repeatedly to update the application's state.
    void update();
    
    // Called repeatedly to draw the application's visuals.
    void draw();
    
    void keyPressed(ofKeyEventArgs &args);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void stopGeneration();
    void clearRAGContext();
    void dragEvent(ofDragInfo dragInfo); // Declaration added for drag-and-drop
    void gotMessage(ofMessage msg);     // Declaration added for message handling

private:
    // --- State Machine ---
    AppState currentState = CHATTING; // The current state of the application (e.g., chatting, summarizing).
    void startReplyGeneration(); // Initiates the process of the AI generating a reply.
    void startSummarization();   // Initiates the process of summarizing the conversation.
    std::string temp_summary_output; // Temporary storage for the summary while it's being generated.

    // --- GUI ---
    ofxPanel gui; // The main GUI panel for controls.
    ofxButton stopButton; // A button to stop AI generation.
    ofxButton clearContextButton; // A button to clear the RAG context.
    std::shared_ptr<ofxDropdown> modelDropdown; // Dropdown for selecting the AI model.
    std::shared_ptr<ofxDropdown> templateDropdown; // Dropdown for selecting the chat template.
    std::map<std::string, std::string> displayNameToFullFileName; // Maps user-friendly model names to their file paths.
    float guiFixedX; // Stores the initial X position of the GUI for stable layout.
    float guiFixedWidth; // Stores the initial width of the GUI for stable layout.
    ofxLabel gpuStatusLabel; // Label to display GPU offload status within the GUI.

    // Callback for when the AI model is changed via the dropdown.
    void onModelChange(string &displayName);
    
    // Callback for when the chat template is changed via the dropdown.
    void onTemplateChange(string &t);

    // --- RAG ---
    ofxRAG rag; // RAG instance
    // --- Llama Engine ---
    ofxLlamaCpp llama; // The core Llama language model object.
    bool ready = false; // Flag indicating if the model is loaded and ready.
    bool wasGenerating = false; // Flag to track if the model was generating in the previous frame.

    // --- Templates ---
    std::string system_prompt; // The system prompt, defining the AI's role or persona.
    std::string template_string; // The raw string for the chat template.
    std::unique_ptr<minja::chat_template> chat_template; // The parsed chat template object.

    // --- Chat/I/O ---
    std::string input;   // The current string of user input.
    std::string mPrompt; // The fully formatted prompt sent to the Llama model.
    
    // --- Memory ---
    std::vector<ChatMessage> chatHistory; // A vector storing the history of the conversation.
    
    // --- UI ---
    ChatUI mChatUI; // The object that manages the chat user interface.
    ContextUI mContextUI; // The object that manages the context user interface.
    TemplateManager mTemplateManager; // The object that manages loading and selecting chat templates.
};
