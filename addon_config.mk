meta:
	ADDON_NAME = ofxRAG
	ADDON_DESCRIPTION = Retrieval-augmented generation helpers (ONNX Runtime, SentencePiece, FAISS)
	ADDON_AUTHOR = ofxRAG contributors
	ADDON_TAGS = "rag" "ml" "onnx" "faiss"

common:
	ADDON_DEPENDENCIES = ofxDropdown ofxGui

	# Feature flags needed by the addon sources
	ADDON_DEFINES = USE_ONNX USE_SENTENCEPIECE USE_FAISS
	ADDON_CPPFLAGS = -DUSE_ONNX -DUSE_SENTENCEPIECE -DUSE_FAISS

	# Headers
	ADDON_INCLUDES = src src/embeddings src/store src/ui
	ADDON_INCLUDES += libs/onnxruntime/include libs/sentencepiece/include libs/faiss/include

	# Ship models alongside the addon
	ADDON_DATA = models

linux64:
	ADDON_LIBS = libs/onnxruntime/lib/linux64/libonnxruntime.so
	ADDON_LIBS += libs/sentencepiece/lib/libsentencepiece.a
	ADDON_LIBS += libs/faiss/lib/libfaiss.a

	# Let the binary find ONNX Runtime without manual LD_LIBRARY_PATH
	ADDON_LDFLAGS = -Wl,-rpath,$(addon)/libs/onnxruntime/lib/linux64
	# Static FAISS needs OpenMP + BLAS/LAPACK
	ADDON_LDFLAGS += -fopenmp -lgomp -llapack -lblas

osx:
	ADDON_LIBS = libs/onnxruntime/lib/osx_arm64/libonnxruntime.dylib
	ADDON_LIBS += libs/sentencepiece/lib/libsentencepiece.a
	ADDON_LIBS += libs/faiss/lib/libfaiss.a

	ADDON_LDFLAGS = -Wl,-rpath,@loader_path/../libs
	ADDON_LDFLAGS += -Wl,-rpath,$(addon)/libs/onnxruntime/lib/osx_arm64
