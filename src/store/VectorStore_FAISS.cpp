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

#include "VectorStore_FAISS.h"
#include "ofLog.h"
#include "ofJson.h"
#include "ofFileUtils.h"

//--------------------------------------------------------------
VectorStore_FAISS::VectorStore_FAISS(int dimension) : dimension(dimension) {
#ifdef USE_FAISS
    ofLogNotice("VectorStore_FAISS") << "Initializing FAISS vector store with dimension: " << dimension;
    index = new faiss::IndexFlatL2(dimension);
#else
    ofLogWarning("VectorStore_FAISS") << "FAISS is not enabled. Vector store will not function.";
#endif
}

//--------------------------------------------------------------
VectorStore_FAISS::~VectorStore_FAISS() {
#ifdef USE_FAISS
    ofLogNotice("VectorStore_FAISS") << "Destroying FAISS vector store.";
    delete index;
#endif
}

//--------------------------------------------------------------
void VectorStore_FAISS::add(const Embedding& embedding, const VectorMetadata& metadata, const std::string& content) {
#ifdef USE_FAISS
    if (embedding.size() != dimension) {
        ofLogError("VectorStore_FAISS") << "Embedding size does not match index dimension.";
        return;
    }
    index->add(1, embedding.data());
    metadatas.push_back(metadata);
    contents.push_back(content);
#endif
}

//--------------------------------------------------------------
std::vector<SearchResult> VectorStore_FAISS::search(const Embedding& query, int k) {
    std::vector<SearchResult> results;
#ifdef USE_FAISS
    if (query.size() != dimension) {
        ofLogError("VectorStore_FAISS") << "Query embedding size does not match index dimension.";
        return results;
    }

    std::vector<faiss::idx_t> labels(k);
    std::vector<float> distances(k);

    index->search(1, query.data(), k, distances.data(), labels.data());

    for (int i = 0; i < k; ++i) {
        if (labels[i] >= 0 && labels[i] < metadatas.size()) {
            SearchResult res;
            res.metadata = metadatas[labels[i]];
            res.distance = distances[i];
            res.content = contents[labels[i]];
            results.push_back(res);
        }
    }
#endif
    return results;
}

//--------------------------------------------------------------
bool VectorStore_FAISS::save(const std::string& path) {
#ifdef USE_FAISS
    try {
        // Save FAISS index
        faiss::write_index(index, path.c_str());
        ofLogNotice("VectorStore_FAISS") << "FAISS index saved to: " << path;

        // Save metadata
        ofJson metaJson;
        for (const auto& meta : metadatas) {
            ofJson meta_json;
            meta_json["id"] = meta.id;
            meta_json["source"] = meta.source;
            meta_json["type"] = meta.type;
            metaJson.push_back(meta_json);
        }
        ofSaveJson(ofFilePath::removeExt(path) + ".meta", metaJson);
        ofLogNotice("VectorStore_FAISS") << "Metadata saved to: " << ofFilePath::removeExt(path) + ".meta";

        // Save contents
        ofJson contentsJson = ofJson::array();
        for(const auto& content : contents) {
            contentsJson.push_back(content);
        }
        ofSaveJson(ofFilePath::removeExt(path) + ".contents", contentsJson);
        ofLogNotice("VectorStore_FAISS") << "Contents saved to: " << ofFilePath::removeExt(path) + ".contents";

        return true;
    } catch (const std::exception& e) {
        ofLogError("VectorStore_FAISS") << "Failed to save FAISS index: " << e.what();
        return false;
    }
#else
    return false;
#endif
}

//--------------------------------------------------------------
bool VectorStore_FAISS::load(const std::string& path) {
#ifdef USE_FAISS
    try {
        // Load FAISS index
        faiss::Index* new_index = faiss::read_index(path.c_str());
        if (new_index->d != dimension) {
            ofLogError("VectorStore_FAISS") << "Loaded index dimension (" << new_index->d << ") does not match configured dimension (" << dimension << ").";
            delete new_index;
            return false;
        }
        delete index;
        index = dynamic_cast<faiss::IndexFlatL2*>(new_index);
        ofLogNotice("VectorStore_FAISS") << "FAISS index loaded from: " << path;

        // Load metadata
        ofJson metaJson = ofLoadJson(ofFilePath::removeExt(path) + ".meta");
        metadatas.clear();
        for (const auto& meta_json : metaJson) {
            VectorMetadata meta;
            meta.id = meta_json["id"];
            meta.source = meta_json["source"];
            meta.type = meta_json["type"];
            metadatas.push_back(meta);
        }
        ofLogNotice("VectorStore_FAISS") << "Metadata loaded from: " << ofFilePath::removeExt(path) + ".meta";

        // Load contents
        ofJson contentsJson = ofLoadJson(ofFilePath::removeExt(path) + ".contents");
        contents.clear();
        for (const auto& contentJson : contentsJson) {
            contents.push_back(contentJson.get<std::string>());
        }
        ofLogNotice("VectorStore_FAISS") << "Contents loaded from: " << ofFilePath::removeExt(path) + ".contents";


        return true;
    } catch (const std::exception& e) {
        ofLogError("VectorStore_FAISS") << "Failed to load FAISS index: " << e.what();
        return false;
    }
#else
    return false;
#endif
}

//--------------------------------------------------------------
void VectorStore_FAISS::clear() {
#ifdef USE_FAISS
    index->reset();
    metadatas.clear();
    contents.clear();
    ofLogNotice("VectorStore_FAISS") << "FAISS index and metadata cleared.";
#endif
}

//--------------------------------------------------------------
size_t VectorStore_FAISS::size() const {
#ifdef USE_FAISS
    return index->ntotal;
#else
    return 0;
#endif
}

//--------------------------------------------------------------
std::vector<std::string> VectorStore_FAISS::getSources() const {
    std::set<std::string> unique_sources;
    for (const auto& metadata : metadatas) {
        unique_sources.insert(metadata.source);
    }
    return std::vector<std::string>(unique_sources.begin(), unique_sources.end());
}
