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

#include "ofxRAG.h"

ofxRAG::ofxRAG() : nextId(0), chunkSize(500), overlapSize(100) {}

ofxRAG::~ofxRAG() {
    // Shared pointers will handle memory management automatically.
}

void ofxRAG::setup() {
    ofLogNotice("ofxRAG") << "Setup complete.";
    nextId = 0;
}

// --- Setters for Embedders and Store ---
void ofxRAG::setTextEmbedder(std::shared_ptr<TextEmbeddingBase> embedder) {
    textEmbedder = embedder;
    ofLogNotice("ofxRAG") << "Text embedder set to: " << (embedder ? embedder->getName() : "none");
}

void ofxRAG::setVectorStore(std::shared_ptr<VectorStoreBase> store) {
    vectorStore = store;
    ofLogNotice("ofxRAG") << "Vector store set.";
}

// --- Getters ---
std::shared_ptr<TextEmbeddingBase> ofxRAG::getTextEmbedder() const {
    return textEmbedder;
}

// --- High-Level API: Add data ---
void ofxRAG::addText(const std::string& text, const std::string& source) {
    if (!textEmbedder) {
        ofLogWarning("ofxRAG") << "Cannot add text, no text embedder set.";
        return;
    }
    if (!vectorStore) {
        ofLogWarning("ofxRAG") << "Cannot add text, no vector store set.";
        return;
    }

    std::vector<std::string> chunks;
    if (text.length() > chunkSize) { // Only chunk if text is larger than chunk size
        chunks = chunkText(text, chunkSize, overlapSize);
        ofLogNotice("ofxRAG") << "Chunked text into " << chunks.size() << " parts from source: " << source;
    } else {
        chunks.push_back(text);
    }
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        Embedding embedding = embedText(chunks[i]);
        std::string chunkSource = source;
        if (chunks.size() > 1) { // Append chunk index if there are multiple chunks
            chunkSource += " (chunk " + ofToString(i + 1) + "/" + ofToString(chunks.size()) + ")";
        }
        VectorMetadata meta = {nextId++, chunkSource, "text"};
        vectorStore->add(embedding, meta, chunks[i]);
    }
}

// --- High-Level API: Search data ---
std::vector<SearchResult> ofxRAG::searchText(const std::string& query, int top_k) {
    if (!textEmbedder || !vectorStore) {
        ofLogWarning("ofxRAG") << "Cannot search text, embedder or store not set.";
        return {};
    }
    Embedding queryEmbedding = embedText(query);
    return vectorStore->search(queryEmbedding, top_k);
}

// --- Direct Embedding API ---
Embedding ofxRAG::embedText(const std::string& text) {
    if (!textEmbedder) {
        ofLogWarning("ofxRAG") << "Cannot embed text, no text embedder set.";
        return {};
    }
    return textEmbedder->embed(text);
}

// --- Vector Store Management ---
void ofxRAG::clearStore() {
    if (vectorStore) {
        vectorStore->clear();
        nextId = 0;
        ofLogNotice("ofxRAG") << "Vector store cleared.";
    }
}

bool ofxRAG::saveStore(const std::string& filepath) {
    if (vectorStore) {
        return vectorStore->save(filepath);
    }
    ofLogWarning("ofxRAG") << "Cannot save, no vector store set.";
    return false;
}

bool ofxRAG::loadStore(const std::string& filepath) {
    if (vectorStore) {
        bool result = vectorStore->load(filepath);
        if(result) {
            nextId = vectorStore->size(); // Simple way to reset ID counter
        }
        return result;
    }
    ofLogWarning("ofxRAG") << "Cannot load, no vector store set.";
    return false;
}

size_t ofxRAG::getStoreSize() const {
    if (vectorStore) {
        return vectorStore->size();
    }
    return 0;
}

// --- Get context sources ---
std::vector<std::string> ofxRAG::getContextSources() const {
    if (vectorStore) {
        return vectorStore->getSources();
    }
    return {};
}

// --- Text Chunking Helper ---
std::vector<std::string> ofxRAG::chunkText(const std::string& text, size_t chunkSize, size_t overlapSize) {
    std::vector<std::string> chunks;
    if (text.empty()) {
        return chunks;
    }

    if (chunkSize == 0) {
        ofLogWarning("ofxRAG") << "Chunk size cannot be zero. Returning original text as single chunk.";
        chunks.push_back(text);
        return chunks;
    }

    size_t currentPos = 0;
    while (currentPos < text.length()) {
        size_t endPos = std::min(currentPos + chunkSize, text.length());
        std::string chunk = text.substr(currentPos, endPos - currentPos);
        chunks.push_back(chunk);

        if (endPos == text.length()) {
            break;
        }

        // Move currentPos back by overlapSize for the next chunk
        currentPos += (chunkSize - overlapSize);
        // Ensure currentPos doesn't go beyond string length
        if (currentPos >= text.length()) {
            break;
        }
    }
    return chunks;
}
