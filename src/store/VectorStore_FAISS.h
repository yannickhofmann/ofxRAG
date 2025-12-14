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

#ifdef USE_FAISS
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#endif

class VectorStore_FAISS : public VectorStoreBase {
public:
    VectorStore_FAISS(int dimension);
    ~VectorStore_FAISS() override;

    void add(const Embedding& embedding, const VectorMetadata& metadata, const std::string& content) override;
    std::vector<SearchResult> search(const Embedding& query, int k) override;

    bool save(const std::string& path) override;
    bool load(const std::string& path) override;

    void clear() override;

    size_t size() const override;

private:
#ifdef USE_FAISS
    faiss::IndexFlatL2* index;
#endif
    int dimension;
    std::vector<VectorMetadata> metadatas;
    std::vector<std::string> contents;
};
