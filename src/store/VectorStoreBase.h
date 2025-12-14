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
#include "embeddings/TextEmbeddingBase.h" // For Embedding type

// A simple struct to hold metadata for each stored vector.
// Can be extended as needed.
struct VectorMetadata {
    int id;
    std::string source; // e.g., file path, text snippet
    std::string type;   // "text", "image", "audio"
};

// A struct to hold the results of a search.
struct SearchResult {
    VectorMetadata metadata;
    float distance; // or similarity score
    std::string content;
};

class VectorStoreBase {
public:
    virtual ~VectorStoreBase() = default;

    // Adds an embedding, its metadata, and the original text content to the store.
    virtual void add(const Embedding& embedding, const VectorMetadata& metadata, const std::string& content) = 0;

    // Searches the store for the top_k most similar vectors to the query.
    virtual std::vector<SearchResult> search(const Embedding& query, int top_k) = 0;

    // Clears all entries from the store.
    virtual void clear() = 0;

    // Saves the store's contents to a file.
    virtual bool save(const std::string& filepath) = 0;

    // Loads the store's contents from a file.
    virtual bool load(const std::string& filepath) = 0;
    
    // Returns the number of items in the store
    virtual size_t size() const = 0;
    virtual std::vector<std::string> getSources() const = 0;
};
