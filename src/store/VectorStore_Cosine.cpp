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

#include "VectorStore_Cosine.h"

//--------------------------------------------------------------
VectorStore_Cosine::VectorStore_Cosine() {
    ofLogNotice("VectorStore_Cosine") << "Initialized in-memory cosine vector store.";
}

//--------------------------------------------------------------
VectorStore_Cosine::~VectorStore_Cosine() {
    ofLogNotice("VectorStore_Cosine") << "Destructed.";
}

//--------------------------------------------------------------
void VectorStore_Cosine::add(const Embedding& embedding, const VectorMetadata& meta, const std::string& content) {
    if (embedding.empty()) {
        ofLogWarning("VectorStore_Cosine") << "Attempted to add empty embedding.";
        return;
    }
    if (!embeddings.empty() && embedding.size() != embeddings[0].size()) {
        ofLogWarning("VectorStore_Cosine") << "Embedding dimension mismatch. Expected " << embeddings[0].size() << ", got " << embedding.size();
        return;
    }
    embeddings.push_back(embedding);
    metadata.push_back(meta);
    contents.push_back(content);
    ofLogVerbose("VectorStore_Cosine") << "Added embedding with ID: " << meta.id << ", current size: " << embeddings.size();
}

//--------------------------------------------------------------
std::vector<SearchResult> VectorStore_Cosine::search(const Embedding& query, int top_k) {
    std::vector<SearchResult> results;
    if (embeddings.empty()) {
        ofLogNotice("VectorStore_Cosine") << "Store is empty, no search results.";
        return results;
    }
    if (query.empty()) {
        ofLogWarning("VectorStore_Cosine") << "Query embedding is empty.";
        return results;
    }
    if (query.size() != embeddings[0].size()) {
        ofLogWarning("VectorStore_Cosine") << "Query embedding dimension mismatch. Expected " << embeddings[0].size() << ", got " << query.size();
        return results;
    }

    std::vector<std::pair<float, int>> similarities; // pair: (similarity, index)

    for (size_t i = 0; i < embeddings.size(); ++i) {
        float sim = cosineSimilarity(query, embeddings[i]);
        similarities.push_back({sim, i});
    }

    // Sort by similarity in descending order
    std::sort(similarities.rbegin(), similarities.rend());

    // Collect top_k results
    for (int i = 0; i < std::min((int)similarities.size(), top_k); ++i) {
        SearchResult res;
        res.metadata = metadata[similarities[i].second];
        res.distance = similarities[i].first; // Using similarity as distance for now
        res.content = contents[similarities[i].second];
        results.push_back(res);
    }
    
    ofLogVerbose("VectorStore_Cosine") << "Search completed, found " << results.size() << " results.";
    return results;
}

//--------------------------------------------------------------
void VectorStore_Cosine::clear() {
    embeddings.clear();
    metadata.clear();
    contents.clear();
    ofLogNotice("VectorStore_Cosine") << "Store cleared.";
}

//--------------------------------------------------------------
bool VectorStore_Cosine::save(const std::string& filepath) {
    ofJson storeJson;
    storeJson["count"] = embeddings.size();
    
    ofJson embeddingsJson = ofJson::array();
    for(const auto& emb : embeddings) {
        embeddingsJson.push_back(emb);
    }
    storeJson["embeddings"] = embeddingsJson;

    ofJson metadataJson = ofJson::array();
    for(const auto& meta : metadata) {
        ofJson metaItem;
        metaItem["id"] = meta.id;
        metaItem["source"] = meta.source;
        metaItem["type"] = meta.type;
        metadataJson.push_back(metaItem);
    }
    storeJson["metadata"] = metadataJson;

    ofJson contentsJson = ofJson::array();
    for(const auto& content : contents) {
        contentsJson.push_back(content);
    }
    storeJson["contents"] = contentsJson;

    return ofSaveJson(filepath, storeJson);
}

//--------------------------------------------------------------
bool VectorStore_Cosine::load(const std::string& filepath) {
    ofJson storeJson; // Re-declare storeJson
    storeJson = ofLoadJson(filepath);
    if (storeJson.empty()) {
        ofLogError("VectorStore_Cosine") << "Failed to load or parsing error from " << filepath;
        return false;
    }

    clear(); // Clear existing data before loading

    size_t count = storeJson["count"].get<size_t>();
    embeddings.reserve(count);
    metadata.reserve(count);
    contents.reserve(count);

    ofJson embeddingsJson = storeJson["embeddings"];
    for(const auto& embJson : embeddingsJson) {
        Embedding emb = embJson.get<Embedding>();
        embeddings.push_back(emb);
    }

    ofJson metadataJson = storeJson["metadata"];
    for(const auto& metaJson : metadataJson) {
        VectorMetadata meta;
        meta.id = metaJson["id"].get<int>();
        meta.source = metaJson["source"].get<std::string>();
        meta.type = metaJson["type"].get<std::string>();
        metadata.push_back(meta);
    }

    if (storeJson.contains("contents")) {
        ofJson contentsJson = storeJson["contents"];
        for(const auto& contentJson : contentsJson) {
            std::string content = contentJson.get<std::string>();
            contents.push_back(content);
        }
    } else {
        // for backwards compatibility, if contents are not present, use the source from metadata
        for(const auto& meta : metadata) {
            contents.push_back(meta.source);
        }
    }
    
    ofLogNotice("VectorStore_Cosine") << "Loaded " << count << " items from " << filepath;
    return true;
}

//--------------------------------------------------------------
size_t VectorStore_Cosine::size() const {
    return embeddings.size();
}

//--------------------------------------------------------------
std::vector<std::string> VectorStore_Cosine::getSources() const {
    std::vector<std::string> sources;
    std::set<std::string> unique_sources;
    for (const auto& meta : metadata) {
        std::string source = meta.source;
        size_t pos = source.rfind(" (chunk");
        if (pos != std::string::npos) {
            source = source.substr(0, pos);
        }
        if (unique_sources.find(source) == unique_sources.end()) {
            sources.push_back(source);
            unique_sources.insert(source);
        }
    }
    return sources;
}

//--------------------------------------------------------------
float VectorStore_Cosine::cosineSimilarity(const Embedding& v1, const Embedding& v2) {
    if (v1.empty() || v2.empty() || v1.size() != v2.size()) {
        return 0.0f; // Or throw an error
    }

    float dotProduct = 0.0f;
    for (size_t i = 0; i < v1.size(); ++i) {
        dotProduct += v1[i] * v2[i];
    }

    float mag1 = magnitude(v1);
    float mag2 = magnitude(v2);

    if (mag1 == 0.0f || mag2 == 0.0f) {
        return 0.0f;
    }

    return dotProduct / (mag1 * mag2);
}

//--------------------------------------------------------------
float VectorStore_Cosine::magnitude(const Embedding& v) {
    float sumSq = 0.0f;
    for (float val : v) {
        sumSq += val * val;
    }
    return std::sqrt(sumSq);
}
