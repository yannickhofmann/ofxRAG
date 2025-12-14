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

#include "VectorStoreBase.h"
#include "ofJson.h"

class VectorStore_Cosine : public VectorStoreBase {
public:
    VectorStore_Cosine();
    ~VectorStore_Cosine() override;

    void add(const Embedding& embedding, const VectorMetadata& metadata, const std::string& content) override;
    std::vector<SearchResult> search(const Embedding& query, int top_k) override;
    void clear() override;
    bool save(const std::string& filepath) override;
    bool load(const std::string& filepath) override;
    size_t size() const override;
    std::vector<std::string> getSources() const override;

private:
    // Helper function to calculate cosine similarity
    static float cosineSimilarity(const Embedding& v1, const Embedding& v2);
    static float magnitude(const Embedding& v);

    // In-memory storage for embeddings and their metadata
    std::vector<Embedding> embeddings;
    std::vector<VectorMetadata> metadata;
    std::vector<std::string> contents;
};
