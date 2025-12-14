#!/usr/bin/env bash
set -euo pipefail

################################################################################
# Versions (locked)
################################################################################

ONNX_VERSION="1.17.1"
FAISS_TAG="v1.8.0"
SENTENCEPIECE_TAG="v0.1.99"

################################################################################
# Paths
################################################################################

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ADDON_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

BUILD_DIR="$ADDON_ROOT/.build"
LIBS_DIR="$ADDON_ROOT/libs"

SP_SRC="$BUILD_DIR/sentencepiece"
SP_BUILD="$BUILD_DIR/sentencepiece-build"

FAISS_SRC="$BUILD_DIR/faiss"
FAISS_BUILD="$BUILD_DIR/faiss-build"

ONNX_DIR="$LIBS_DIR/onnxruntime"

################################################################################
# Platform check
################################################################################

ARCH="$(uname -m)"
OS="$(uname -s)"

if [[ "$OS" != "Linux" || "$ARCH" != "x86_64" ]]; then
  echo "This script currently supports Linux x86_64 only"
  exit 1
fi

echo "Addon root : $ADDON_ROOT"
echo "Platform   : linux (x86_64)"

################################################################################
# Clean build + libs (safe)
################################################################################

echo "Resetting build directories"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR" "$LIBS_DIR"

################################################################################
# SentencePiece (static, vendored)
################################################################################

echo "Building SentencePiece"

git clone --depth 1 --branch "$SENTENCEPIECE_TAG" \
  https://github.com/google/sentencepiece.git "$SP_SRC"

cmake -S "$SP_SRC" -B "$SP_BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSPM_ENABLE_SHARED=OFF \
  -DSPM_ENABLE_TCMALLOC=OFF

cmake --build "$SP_BUILD" --parallel
cmake --install "$SP_BUILD" --prefix "$LIBS_DIR/sentencepiece"

################################################################################
# FAISS (GCC-13 SAFE BUILD)
################################################################################

echo "Building FAISS (NSG OFF, NNDescent OFF, OpenMP OFF)"

git clone --depth 1 --branch "$FAISS_TAG" \
  https://github.com/facebookresearch/faiss.git "$FAISS_SRC"

cmake -S "$FAISS_SRC" -B "$FAISS_BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DFAISS_ENABLE_GPU=OFF \
  -DFAISS_ENABLE_PYTHON=OFF \
  -DFAISS_ENABLE_C_API=ON \
  -DFAISS_ENABLE_TESTS=OFF \
  -DFAISS_ENABLE_BENCHMARKS=OFF \
  -DFAISS_ENABLE_OPENMP=OFF \
  -DFAISS_ENABLE_NSG=OFF \
  -DFAISS_ENABLE_NN_DESCENT=OFF \
  -DCMAKE_CXX_FLAGS="-O1 -fno-tree-vectorize -fno-strict-aliasing"

cmake --build "$FAISS_BUILD" --parallel
cmake --install "$FAISS_BUILD" --prefix "$LIBS_DIR/faiss"

################################################################################
# ONNX Runtime (prebuilt binary, NO BUILD)
################################################################################

echo "Installing ONNX Runtime ${ONNX_VERSION} (prebuilt)"

TMP_DIR="$(mktemp -d)"
ONNX_TGZ="onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/${ONNX_TGZ}"

curl -L "$ONNX_URL" -o "$TMP_DIR/$ONNX_TGZ"
tar -xzf "$TMP_DIR/$ONNX_TGZ" -C "$TMP_DIR"

rm -rf "$ONNX_DIR"
mkdir -p "$ONNX_DIR/lib/linux64"

cp -r "$TMP_DIR/onnxruntime-linux-x64-${ONNX_VERSION}/include" "$ONNX_DIR/"
cp "$TMP_DIR/onnxruntime-linux-x64-${ONNX_VERSION}/lib/"* \
   "$ONNX_DIR/lib/linux64/"

rm -rf "$TMP_DIR"

################################################################################
# Cleanup (optional but recommended)
################################################################################

echo "Cleaning up build directory"
rm -rf "$BUILD_DIR"

################################################################################
# Summary
################################################################################

echo ""
echo "All dependencies installed successfully"
echo ""
echo "Installed libraries:"
echo "  libs/sentencepiece"
echo "  libs/faiss"
echo "  libs/onnxruntime"
echo ""
echo "You can now build ofxRAG example projects"
