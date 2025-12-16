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
#include "ofxGui.h"
#include "ofxPoDoFo.h"
#include "store/VectorStore_Cosine.h" // Specific vector store for Cosine similarity
#include "store/VectorStore_FAISS.h" // Specific vector store for FAISS

// --- HELPER FUNCTIONS ---

// Removes a single UTF-8 character from the end of a string.
// param s The string to modify.
static void utf8_pop_back(std::string &s) {
    if (s.empty()) return;
    int i = s.size() - 1;
    // Backtrack until the start of a multi-byte character is found.
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) i--;
    s.erase(i);
}

//--------------------------------------------------------------
void ofApp::setup() {
    // Basic openFrameworks setup
    ofBackground(0);
    ofSetColor(255);
    ofSetFrameRate(60);
    
    // Initialize the chat UI
    mChatUI.setup("fonts/verdana.ttf", 12);
    mContextUI.setup("fonts/verdana.ttf", 12);

    // Initialize ofxRAG and set the T5 Text Embedder
    rag.setup();
    rag.setTextEmbedder(std::make_shared<TextEmbedding_T5>());
    rag.setVectorStore(std::make_shared<VectorStore_FAISS>(768)); // Initialize with FAISS store

    // Add some sample text to the RAG store
    
    // Define the default system prompt for the AI
    system_prompt =
        "You are a specialized AI assistant, the \"Context Guardian\"."
        "\n\n"
        "Your task is to answer questions using ONLY the provided RAG CONTEXT."
        "\n\n"
        "Rules:\n"
        "1. Every factual claim must be supported by the RAG CONTEXT.\n"
        "2. You may paraphrase, summarize, or combine information from the context, but you may not add new facts or external knowledge.\n"
        "3. If the RAG CONTEXT does not support an answer, respond exactly:\n"
        "   \"The provided context does not contain enough information to answer this question.\"\n"
        "4. Answer concisely and directly. No conversational filler.\n"
        "5. If asked to summarize the context, provide a brief factual summary only.\n"
        "\n"
        "Do not use external knowledge. Do not speculate.";

    // --- GUI Setup ---
    gui.setup("LLM Control");
    stopButton.addListener(this, &ofApp::stopGeneration);

    // --- Model Loading ---
    // Scan the 'data/models' directory for .gguf files
    ofDirectory dir(ofToDataPath("models"));
    dir.allowExt("gguf");
    dir.listDir();
    dir.sort();

    vector<string> modelDisplayNames;
    displayNameToFullFileName.clear();
    int max_len = 20; // Max length for display names in the dropdown

    // Populate the dropdown list with truncated model names for readability
    for (auto &f : dir.getFiles()) {
        string filename = f.getFileName();
        string displayName = filename;
        if (filename.length() > max_len) {
            displayName = filename.substr(0, max_len - 3) + "...";
        }
        modelDisplayNames.push_back(displayName);
        displayNameToFullFileName[displayName] = filename; // Map display name to full filename
    }

    if (modelDisplayNames.empty()) {
        modelDisplayNames.push_back("NO MODEL FOUND");
    }

    // --- Dropdown Menus ---
    modelDropdown = std::make_shared<ofxDropdown>("Model");
    modelDropdown->add(modelDisplayNames);
    modelDropdown->disableMultipleSelection();
    modelDropdown->enableCollapseOnSelection();
    modelDropdown->addListener(this, &ofApp::onModelChange);
    gui.add(modelDropdown.get());

    templateDropdown = std::make_shared<ofxDropdown>("Template");
    templateDropdown->add(mTemplateManager.getTemplateNames());
    templateDropdown->disableMultipleSelection();
    templateDropdown->enableCollapseOnSelection(); 
    templateDropdown->addListener(this, &ofApp::onTemplateChange);
    gui.add(templateDropdown.get());

    gui.add(stopButton.setup("Stop Generation"));
    clearContextButton.addListener(this, &ofApp::clearRAGContext);
    gui.add(clearContextButton.setup("Clear RAG Context"));

    // Add GPU status label to the GUI
    gpuStatusLabel.setup("GPU Layers", "N/A"); // Initialize with N/A, assuming ofxLabel adds a colon
    gui.add(&gpuStatusLabel); // Add the label to the GUI panel
    gui.setPosition(20, 20);

    // Store fixed GUI dimensions after setup for stable layout
    guiFixedX = gui.getPosition().x;
    guiFixedWidth = gui.getWidth();


    // --- Default Selections ---
    // Set a default template to ensure the application starts in a valid state
    string t = mTemplateManager.getTemplateNames().front();
    templateDropdown->selectedValue = t;
    onTemplateChange(t);

    // Set a default model
    if (!modelDisplayNames.empty() && modelDisplayNames[0] != "NO MODEL FOUND") {
        string m = modelDisplayNames[0];
        modelDropdown->selectedValue = m;
        onModelChange(m);
    } else {
        string m = "NO MODEL FOUND";
        if(modelDropdown->getNumOptions() > 0) m = modelDropdown->selectedValue.get();
        onModelChange(m);
    }
}

//--------------------------------------------------------------
void ofApp::onModelChange(string &displayName) {
    ready = false;
    // Clear chat history when changing models to reset the context
    chatHistory.clear();
    currentState = CHATTING;

    // Retrieve the full model filename from the display name
    string model = displayName;
    if (displayNameToFullFileName.count(displayName)) {
        model = displayNameToFullFileName[displayName];
    }
    string fullPath = ofToDataPath("models/" + model);
    ofLogNotice() << "Loading model: " << fullPath;

    // Load the model using ofxLlamaCpp
    if (llama.loadModel(fullPath, 2048)) { // 2048 context size
        ready = true;
        
        
        // Set the number of layers to offload to the GPU.
        // By default, offload all layers.
        llama.setN_GpuLayers(llama.getNLayers());
        llama.setOffloadKqv(true);
        
        // Update the GPU status label
        gpuStatusLabel.setup("GPU Layers", ofToString(llama.getN_GpuLayers()));
        
    // Set generation parameters
    llama.setTemperature(0.8f);
    llama.setTopP(0.9f);
    llama.setTopK(40);
    llama.setRepeatPenalty(1.1f);
    
    // Re-apply stop words based on the currently selected template
    string t = templateDropdown->selectedValue.get();
    onTemplateChange(t);

            ofLogNotice() << "Model loaded successfully.";
        } else { // Added this else block for completeness, assuming model load failure
            ofLogError() << "Model load failed!";
        }
    }
//--------------------------------------------------------------
void ofApp::onTemplateChange(string &t) {
    // Clear existing stop words before applying new ones
    llama.clearStopWords(); 
    template_string = mTemplateManager.getTemplate(t);
    
    // Add specific stop words based on the selected template's format
    // This is crucial to prevent the model from generating conversational turns for both user and assistant
    if (t == "DeepSeek") {
        llama.addStopWord("<｜User｜>");
        llama.addStopWord("<｜Assistant｜>");
        llama.addStopWord("<｜End｜>");
        llama.addStopWord("\n<｜User｜>");
        llama.addStopWord("\n<｜Assistant｜>");
        llama.addStopWord("<｜end_of_turn｜>");
        llama.addStopWord("<EndOfThought>");
        llama.addStopWord("<EndofAssistantResponse>");
        llama.addStopWord("<End of response>");
        llama.addStopWord("</End of response>");
        llama.addStopWord("<／Assistant〉");
        llama.addStopWord("\n<／Assistant〉");
    }
    else if (t == "Phi4") {
        llama.addStopWord("<|im_end|>");
        llama.addStopWord("<|end|>");
        llama.addStopWord("<｜end_of_turn｜>");
        llama.addStopWord("<EndOfThought>");
        llama.addStopWord("<EndofAssistantResponse>");
        llama.addStopWord("<End of response>");
        llama.addStopWord("</End of response>");
        llama.addStopWord("<／Assistant〉");
        llama.addStopWord("\n<／Assistant〉");
    }
    else if (t == "Teuken") {
        llama.addStopWord("User:");
        llama.addStopWord("Assistant:");
        llama.addStopWord("<｜end_of_turn｜>");
        llama.addStopWord("<EndOfThought>");
        llama.addStopWord("<EndofAssistantResponse>");
        llama.addStopWord("<End of response>");
        llama.addStopWord("</End of response>");
        llama.addStopWord("<／Assistant〉");
        llama.addStopWord("\n<／Assistant〉");
    }

    // Re-create the chat template object with the new template string
    try {
        chat_template = std::make_unique<minja::chat_template>(template_string, "", "");
        ofLogNotice() << "Template switched to: " << t;
    } catch (const std::exception& e) {
        ofLogError() << "Failed to create chat template: " << e.what();
        chat_template = nullptr; // Invalidate the template if parsing fails
    }
}


//--------------------------------------------------------------
void ofApp::startReplyGeneration() {
    ofLogNotice("ofApp") << "Starting reply generation...";
    
    bool isDeepSeek = (templateDropdown->selectedValue.get() == "DeepSeek");
    nlohmann::json messages = nlohmann::json::array();

    // 2. Retrieve context from ofxRAG based on the latest user query
    std::string latestUserQuery;
    for (int i = chatHistory.size() - 1; i >= 0; --i) {
        if (chatHistory[i].isUser) {
            latestUserQuery = chatHistory[i].content;
            break;
        }
    }

    std::string ragContext = "";
    if (!latestUserQuery.empty()) {
        std::vector<SearchResult> results = rag.searchText(latestUserQuery, 5); // Increased top_k from 3 to 5
        if (!results.empty()) {
            ragContext += "[RAG CONTEXT]\n";
            for (const auto& result : results) {
                ragContext += "Source: " + result.metadata.source + "\n";
                ragContext += "Text: " + result.content + "\n\n";
            }
        }
    }
    
    // 2. Add the main system prompt (AI's personality/instructions)
    messages.push_back({
        {"role", isDeepSeek ? "user" : "system"},
        {"content", system_prompt}
    });

    // 3. If RAG context exists, add it as context
    if (!ragContext.empty()) {
        messages.push_back({
            {"role", isDeepSeek ? "user" : "system"},
            {"content", ragContext}
        });
    } else {
        messages.push_back({
            {"role", isDeepSeek ? "user" : "system"},
            {"content", "[RAG CONTEXT]\nNo context provided."}
        });
    }

    // 4. Add only the most recent user query to the prompt
    if (!latestUserQuery.empty()) {
        messages.push_back({
            {"role", "user"},
            {"content", latestUserQuery}
        });
    }

    // 4. Apply the template to format the final prompt and start generation
    minja::chat_template_inputs tmplInputs;
    tmplInputs.messages = messages;
    tmplInputs.add_generation_prompt = true;

    mPrompt = chat_template->apply(tmplInputs);
    
    ofLogNotice("ofApp PROMPT") << mPrompt;

    llama.resetContext(); // Explicitly reset the context before generation
    llama.startGeneration(mPrompt, 1024); // Limit reply length to 1024 tokens
    wasGenerating = true; 
}


//--------------------------------------------------------------
void ofApp::update() {
    if (!ready) return; // Don't do anything if the model isn't loaded

    // State machine for handling model generation (replying vs. summarizing)
    std::string chunk = llama.getNewOutput();
    if (!chunk.empty()) {
        // Append the new text chunk to the appropriate variable based on the current state
        switch (currentState) {
            case GENERATING_REPLY:
                // If this is the first chunk, create a new message; otherwise, append to the last one
                if (chatHistory.empty() || chatHistory.back().isUser) {
                    chatHistory.push_back({chunk, false, ofColor::white});
                } else {
                    chatHistory.back().content += chunk;
                }
                mChatUI.scrollToBottom();
                break;
            default:
                break;
        }
    }

    // Check if generation has just finished
    if (wasGenerating && !llama.isGenerating()) {
        wasGenerating = false; // Reset the flag

        switch (currentState) {
            case GENERATING_REPLY:
                ofLogNotice("ofApp") << "Reply finished.";
                // Clean up any trailing stop words from the generated text
                if (!chatHistory.empty() && !chatHistory.back().isUser) {
                    const auto& stopWords = llama.getStopWords();
                    for (const auto& stopWord : stopWords) {
                        size_t pos = chatHistory.back().content.rfind(stopWord);
                        if (pos != std::string::npos) {
                            chatHistory.back().content.erase(pos);
                        }
                    }
                    ofLogNotice("ofApp ASSISTANT") << chatHistory.back().content;
                }
                // Transition back to the idle chatting state
                currentState = CHATTING;
                break;
            default:
                break;
        }
    }
    wasGenerating = llama.isGenerating();
}


//--------------------------------------------------------------
void ofApp::draw() {
    // New full-width layout
    float chatWidth = ofGetWidth() * 0.75;
    float contextWidth = ofGetWidth() * 0.25;

    ofRectangle chatViewport(0, 0, chatWidth, ofGetHeight());
    mChatUI.draw(
        chatViewport,
        chatHistory,
        input,
        currentState,
        ready,
        ready ? llama.getContextFillRatio() : 0.0f
    );

    ofRectangle contextViewport(chatWidth, 0, contextWidth, ofGetHeight());
    mContextUI.setViewport(contextViewport);
    mContextUI.draw(contextViewport);

    // Draw the main GUI panel on top
    gui.draw();
}


//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    // Delegate scrolling events to the ChatUI and ContextUI classes
    mChatUI.mouseScrolled(x, y, scrollX, scrollY);
    mContextUI.mouseScrolled(x, y, scrollX, scrollY);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    mChatUI.mousePressed(x, y, button);
    mContextUI.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    mChatUI.mouseDragged(x, y, button);
    mContextUI.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    mChatUI.mouseReleased(x, y, button);
    mContextUI.mouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs &args) {
    if (!ready) return;

    if (args.key == OF_KEY_RETURN) {
        // Only process input if the model is not busy
        if (input.empty() || currentState != CHATTING) return;

        // Add the user's message to the history and clear the input field
        chatHistory.push_back({input, true, ofColor::yellow});
        mChatUI.scrollToBottom();
        input.clear();
        
        // Always generate a reply after user input
        currentState = GENERATING_REPLY;
        startReplyGeneration();
        return;
    }

    // Handle backspace for UTF-8 strings
    if (args.key == OF_KEY_BACKSPACE) {
        if (!input.empty()) {
            utf8_pop_back(input);
        }
        return;
    }

    // Append typed characters to the input string
    if (args.codepoint >= 32) { // Ignore control characters
        ofUTF8Append(input, args.codepoint);
    }    
}

//--------------------------------------------------------------
void ofApp::stopGeneration() {
    llama.stopGeneration();
    wasGenerating = false;
    
    // Reset the state machine to idle
    if (currentState != CHATTING) {
        currentState = CHATTING;
        
        // Add a marker to indicate that generation was manually stopped
        if (!chatHistory.empty() && !chatHistory.back().isUser) {
            chatHistory.back().content += " [...] (Stopped)";
            chatHistory.back().color = ofColor::orange;
        }
    }
}

//--------------------------------------------------------------
void ofApp::clearRAGContext() {
    rag.clearStore();
    chatHistory.clear(); // Clear chat history as well, as RAG context is tied to it
    mContextUI.update(rag.getContextSources());
    ofLogNotice("ofApp") << "RAG context and chat history cleared.";
}

//--------------------------------------------------------------

void ofApp::dragEvent(ofDragInfo dragInfo){ 
    if (!mContextUI.isInside(dragInfo.position.x, dragInfo.position.y)) {
        return; 
    }

    for(auto& file : dragInfo.files) {
        std::string content = "";
        std::string fileExtension = ofToLower(ofFilePath::getFileExt(file));

        if (fileExtension == "pdf") {
            ofxPoDoFo pdf;
            if (pdf.load(file)) {
                content = pdf.getText();
            } else {
                ofLogWarning("ofApp") << "Could not load PDF file: " << file;
            }
        } else {
            ofBuffer buffer = ofBufferFromFile(file);
            if(buffer.size() > 0) {
                content = buffer.getText();
            } else {
                ofLogWarning("ofApp") << "Could not read dragged file: " << file;
            }
        }

        if(!content.empty()) {
            rag.addText(content, file);
            mContextUI.update(rag.getContextSources());
            ofLogNotice("ofApp") << "Dragged and added: " << file << ". RAG store size: " << rag.getStoreSize();
        } else {
            ofLogWarning("ofApp") << "Dragged file is empty or could not be processed: " << file;
        }
    }
}



//--------------------------------------------------------------

void ofApp::gotMessage(ofMessage msg){



}


