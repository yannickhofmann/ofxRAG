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
#include "embeddings/TextEmbeddingBase.h"

#include "store/VectorStoreBase.h"

class ofxRAG {
public:
    ofxRAG();
    ~ofxRAG();

    // --- Setup ---
    void setup(); // for potential heavy setup tasks
    
    // --- Setters for Embedders and Store ---
    // The user of the class is responsible for memory management of the passed pointers.
    void setTextEmbedder(std::shared_ptr<TextEmbeddingBase> embedder);

    void setVectorStore(std::shared_ptr<VectorStoreBase> store);

    // --- Getters ---
    std::shared_ptr<TextEmbeddingBase> getTextEmbedder() const;

    // --- High-Level API ---
    // Add data to the vector store
    void addText(const std::string& text, const std::string& source = "");

    
    // Search for similar items
    std::vector<SearchResult> searchText(const std::string& query, int top_k = 5);


    // --- Direct Embedding API ---
    Embedding embedText(const std::string& text);


    // --- Vector Store Management ---
    void clearStore();
    bool saveStore(const std::string& filepath);
    bool loadStore(const std::string& filepath);
    size_t getStoreSize() const;
    std::vector<std::string> getContextSources() const;

private:
    std::shared_ptr<TextEmbeddingBase> textEmbedder;

    std::shared_ptr<VectorStoreBase> vectorStore;
    
    int nextId;
    
    // Chunking parameters
    size_t chunkSize;
    size_t overlapSize;

    // --- Text Chunking Helper ---
    std::vector<std::string> chunkText(const std::string& text, size_t chunkSize, size_t overlapSize);
};
