#pragma once

#include "TextEmbeddingBase.h"

#ifdef USE_ONNX
#include <onnxruntime_cxx_api.h>
#ifdef USE_SENTENCEPIECE
#include <sentencepiece_processor.h>
#endif
#else
// Define a placeholder dimension if ONNX is not used
#define DUMMY_T5_EMBEDDING_DIM 768
#endif

class TextEmbedding_T5 : public TextEmbeddingBase {
public:
    TextEmbedding_T5();
    ~TextEmbedding_T5() override;

    Embedding embed(const std::string& text) override;
    
    std::string getName() const override {
#ifdef USE_ONNX
        return "T5 (ONNX)";
#else
        return "T5 (ONNX - Placeholder)";
#endif
    }
    
    int getDimension() const override {
#ifdef USE_ONNX
        // TODO: Dynamically get dimension from loaded ONNX model
        return 768; // Typical T5 dimension
#else
        return DUMMY_T5_EMBEDDING_DIM;
#endif
    }
    
private:
#ifdef USE_ONNX
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    std::unique_ptr<Ort::Session> session;
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    
    // Path to the ONNX model file
    std::string modelPath = "models/sentence-t5-base/model.onnx";
    // Path to the SentencePiece model file
    std::string spModelPath = "models/sentence-t5-base/spiece.model";

#ifdef USE_SENTENCEPIECE
    std::unique_ptr<sentencepiece::SentencePieceProcessor> tokenizer;
#endif

    // Helper to tokenize input text
    std::vector<int64_t> tokenize(const std::string& text);
#endif
    bool onnx_initialized = false; // Flag to track successful ONNX initialization (always declared)
};
