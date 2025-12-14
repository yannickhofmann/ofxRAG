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

// Define a type alias for a vector of floats for clarity.
using Embedding = std::vector<float>;

class TextEmbeddingBase {
public:
    virtual ~TextEmbeddingBase() = default;

    // Pure virtual function to get embeddings for a single text.
    // Derived classes must implement this.
    virtual Embedding embed(const std::string& text) = 0;

    // Optional: A virtual function for batch processing.
    // Can be overridden by derived classes for efficiency.
    virtual std::vector<Embedding> embedBatch(const std::vector<std::string>& texts) {
        std::vector<Embedding> embeddings;
        embeddings.reserve(texts.size());
        for (const auto& text : texts) {
            embeddings.push_back(embed(text));
        }
        return embeddings;
    }

    // Returns the name of the model/implementation.
    virtual std::string getName() const = 0;
    
    // Returns the dimension of the embedding vector.
    virtual int getDimension() const = 0;
};
