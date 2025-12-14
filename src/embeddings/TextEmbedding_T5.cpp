#include "TextEmbedding_T5.h"
#include "ofFileUtils.h" // For ofFilePath::join
#include "../ModelPath.h"

//--------------------------------------------------------------
TextEmbedding_T5::TextEmbedding_T5() {
#ifdef USE_ONNX
    ofLogNotice("TextEmbedding_T5") << "T5 Text Embedder (ONNX) initializing. Attempting to load models.";
    
    // The ONNX model path needs to be relative to the data folder or an absolute path.
    std::string fullModelPath = ofxragJoinModelPath("/text_embeddings/sentence-t5-base/model.onnx");
    
    try {
        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "T5_TextEmbedder");
        sessionOptions = Ort::SessionOptions();
        sessionOptions.SetIntraOpNumThreads(1); 

        session = std::make_unique<Ort::Session>(env, fullModelPath.c_str(), sessionOptions);

        Ort::AllocatorWithDefaultOptions allocator;
        // Get input names
        for (size_t i = 0; i < session->GetInputCount(); ++i) {
            inputNames.push_back(std::string(session->GetInputNameAllocated(i, allocator).get()));
        }

        // Get output names
        for (size_t i = 0; i < session->GetOutputCount(); ++i) {
            outputNames.push_back(std::string(session->GetOutputNameAllocated(i, allocator).get()));
        }

#ifdef USE_SENTENCEPIECE
        ofLogNotice("TextEmbedding_T5") << "Initializing SentencePiece tokenizer.";
        tokenizer = std::make_unique<sentencepiece::SentencePieceProcessor>();
        std::string fullSpModelPath = ofxragJoinModelPath("/text_embeddings/sentence-t5-base/spiece.model");
        auto spStatus = tokenizer->Load(fullSpModelPath);
        if (!spStatus.ok()) {
            ofLogError("TextEmbedding_T5") << "Failed to load SentencePiece model from: " << fullSpModelPath;
            throw std::runtime_error("SentencePiece model loading failed.");
        }
        ofLogNotice("TextEmbedding_T5") << "SentencePiece model loaded from: " << fullSpModelPath;
#endif

        onnx_initialized = true;
        ofLogNotice("TextEmbedding_T5") << "T5 ONNX model loaded from: " << fullModelPath;
        ofLogNotice("TextEmbedding_T5") << "Input name: " << inputNames[0];
        ofLogNotice("TextEmbedding_T5") << "Output name: " << outputNames[0];

    } catch (const Ort::Exception& e) {
        ofLogError("TextEmbedding_T5") << "ONNX Runtime error: " << e.what();
        ofLogError("TextEmbedding_T5") << "T5 ONNX initialization failed. Falling back to placeholder behavior.";
        onnx_initialized = false; // Ensure flag is false on failure
    } catch (const std::runtime_error& e) {
        ofLogError("TextEmbedding_T5") << "Tokenizer error: " << e.what();
        ofLogError("TextEmbedding_T5") << "T5 ONNX initialization failed due to tokenizer. Falling back to placeholder behavior.";
        onnx_initialized = false; // Ensure flag is false on failure
    }
#else
    ofLogNotice("TextEmbedding_T5") << "T5 Text Embedder (ONNX - Placeholder) initialized. Compile with USE_ONNX and ONNX Runtime libs for actual functionality.";
    onnx_initialized = false; // Always false if ONNX not compiled
#endif
}

//--------------------------------------------------------------
TextEmbedding_T5::~TextEmbedding_T5() {
#ifdef USE_ONNX
    if (onnx_initialized) {
        ofLogNotice("TextEmbedding_T5") << "T5 Text Embedder (ONNX) destructed.";
    } else {
        ofLogNotice("TextEmbedding_T5") << "T5 Text Embedder (ONNX - Placeholder/Failed ONNX) destructed.";
    }
#else
    ofLogNotice("TextEmbedding_T5") << "T5 Text Embedder (ONNX - Placeholder) destructed.";
#endif
}

//--------------------------------------------------------------
Embedding TextEmbedding_T5::embed(const std::string& text) {
    Embedding embedding;
#ifdef USE_ONNX
    if (onnx_initialized) {
        // --- ONNX Inference ---
        std::vector<int64_t> input_ids = tokenize(text);
        if (input_ids.empty()) {
            ofLogError("TextEmbedding_T5") << "Tokenization failed for text: " << text.substr(0, 50) << "...";
            ofLogWarning("TextEmbedding_T5") << "Returning dummy embedding after tokenization failure.";
            embedding.resize(getDimension());
            for (int i = 0; i < getDimension(); ++i) {
                embedding[i] = ofRandomf();
            }
            return embedding;
        }

        std::vector<int64_t> attention_mask(input_ids.size(), 1); // For T5, attention mask is typically all 1s for non-padding tokens
        
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
        
        std::vector<Ort::Value> inputTensors;
        
        std::vector<int64_t> input_ids_shape = {1, (int64_t)input_ids.size()};
        inputTensors.push_back(Ort::Value::CreateTensor<int64_t>(memoryInfo, input_ids.data(), input_ids.size(), input_ids_shape.data(), input_ids_shape.size()));
        
        std::vector<int64_t> attention_mask_shape = {1, (int64_t)attention_mask.size()};
        inputTensors.push_back(Ort::Value::CreateTensor<int64_t>(memoryInfo, attention_mask.data(), attention_mask.size(), attention_mask_shape.data(), attention_mask_shape.size()));
        
        std::vector<const char*> c_inputNames;
        for (const auto& name : inputNames) {
            c_inputNames.push_back(name.c_str());
        }

        std::vector<const char*> c_outputNames;
        for (const auto& name : outputNames) {
            c_outputNames.push_back(name.c_str());
        }

        std::vector<Ort::Value> outputTensors;
        try {
            outputTensors = session->Run(Ort::RunOptions{nullptr}, c_inputNames.data(), inputTensors.data(), inputTensors.size(), c_outputNames.data(), c_outputNames.size());
        } catch (const Ort::Exception& e) {
            ofLogError("TextEmbedding_T5") << "ONNX Runtime inference error: " << e.what();
            ofLogWarning("TextEmbedding_T5") << "Returning dummy embedding after inference failure.";
            embedding.resize(getDimension());
            for (int i = 0; i < getDimension(); ++i) {
                embedding[i] = ofRandomf();
            }
            return embedding;
        }

        float* floatData = outputTensors[0].GetTensorMutableData<float>();
        embedding.assign(floatData, floatData + getDimension()); 
        
        ofLogVerbose("TextEmbedding_T5") << "Generated ONNX embedding for text: '" << text.substr(0, 50) << "...'";

    } else { // Fallback if ONNX was defined but failed to initialize, or not defined at all
        ofLogVerbose("TextEmbedding_T5") << "Generating placeholder embedding for text: '" << text.substr(0, 50) << "...'";
        embedding.resize(getDimension()); // Use getDimension() to respect DUMMY_T5_EMBEDDING_DIM
        for (int i = 0; i < getDimension(); ++i) {
            embedding[i] = ofRandomf(); 
        }
    }
#else
    ofLogVerbose("TextEmbedding_T5") << "Generating placeholder embedding for text: '" << text.substr(0, 50) << "...'";
    embedding.resize(DUMMY_T5_EMBEDDING_DIM);
    for (int i = 0; i < DUMMY_T5_EMBEDDING_DIM; ++i) {
        embedding[i] = ofRandomf(); // Generate a random float
    }
#endif
    return embedding;
}

#ifdef USE_ONNX
std::vector<int64_t> TextEmbedding_T5::tokenize(const std::string& text) {
#ifdef USE_SENTENCEPIECE
    if (!tokenizer) {
        ofLogError("TextEmbedding_T5") << "SentencePiece tokenizer not initialized.";
        return {}; // Return empty if tokenizer is not ready
    }

    const int EOS_TOKEN_ID = 1; // End-of-sentence token for T5
    const int PAD_TOKEN_ID = 0; // Padding token ID

    std::vector<int> piece_ids;
    tokenizer->Encode(text, &piece_ids);

    // Max length for sentence-t5-base is 256
    const size_t MAX_LENGTH = 256; 

    std::vector<int64_t> input_ids;

    // Add encoded tokens, truncating if necessary
    for (size_t i = 0; i < piece_ids.size() && input_ids.size() < (MAX_LENGTH - 1); ++i) { // -1 for EOS token
        input_ids.push_back(piece_ids[i]);
    }

    input_ids.push_back(EOS_TOKEN_ID); // Add EOS token

    
    ofLogVerbose("TextEmbedding_T5") << "Tokenized text: '" << text.substr(0, 50) << "...' into " << input_ids.size() << " tokens.";
    return input_ids;

#else
    ofLogWarning("TextEmbedding_T5") << "SentencePiece is not enabled. Cannot tokenize text effectively.";
    return {};
#endif
}
#endif
