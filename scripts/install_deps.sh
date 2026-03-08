#!/usr/bin/env bash
set -euo pipefail

################################################################################
# Versions
################################################################################

SENTENCEPIECE_TAG="v0.1.99"
FAISS_TAG="v1.8.0"
ONNX_VERSION="1.17.1"

################################################################################
# Paths
################################################################################

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ADDON_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

BUILD_DIR="$ADDON_ROOT/.build"
LIBS_DIR="$ADDON_ROOT/libs"

SP_SRC="$BUILD_DIR/sentencepiece"
SP_BUILD="$BUILD_DIR/sp-build"

FAISS_SRC="$BUILD_DIR/faiss"
FAISS_BUILD="$BUILD_DIR/faiss-build"

ONNX_DIR="$LIBS_DIR/onnxruntime"

mkdir -p "$BUILD_DIR" "$LIBS_DIR"

################################################################################
# Platform detection
################################################################################

OS="$(uname -s)"
ARCH="$(uname -m)"

case "$ARCH" in
    x86_64|amd64) ARCH="x86_64" ;;
    arm64|aarch64) ARCH="arm64" ;;
    *) echo "Unsupported architecture: $ARCH"; exit 1 ;;
esac

echo "Platform detected:"
echo "OS   : $OS"
echo "ARCH : $ARCH"

################################################################################
# CPU cores (safe for Raspberry Pi)
################################################################################

if command -v nproc >/dev/null 2>&1; then
    CORES=$(nproc)
elif [[ "$OS" == "Darwin" ]]; then
    CORES=$(sysctl -n hw.logicalcpu)
else
    CORES=2
fi

# Avoid OOM on small machines
if [[ "$CORES" -gt 4 ]]; then
    CORES=4
fi

echo "Build threads: $CORES"

################################################################################
# macOS OpenMP setup
################################################################################

OPENMP_FLAGS=""
OPENMP_LINK=""

if [[ "$OS" == "Darwin" ]]; then
    echo "Checking OpenMP (macOS)"

    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew required for macOS builds"
        exit 1
    fi

    brew install libomp >/dev/null 2>&1 || true
    LIBOMP_PREFIX="$(brew --prefix libomp)"

    OPENMP_FLAGS="-Xpreprocessor -fopenmp -I$LIBOMP_PREFIX/include"
    OPENMP_LINK="-L$LIBOMP_PREFIX/lib -lomp"
fi

################################################################################
# Helper: clone repo safely
################################################################################

clone_repo () {
    local repo=$1
    local tag=$2
    local dst=$3

    if [[ ! -d "$dst" ]]; then
        git clone --depth 1 --branch "$tag" "$repo" "$dst"
    fi
}

################################################################################
# SentencePiece
################################################################################

echo ""
echo "Building SentencePiece"

clone_repo \
https://github.com/google/sentencepiece.git \
"$SENTENCEPIECE_TAG" \
"$SP_SRC"

cmake -S "$SP_SRC" -B "$SP_BUILD" \
 -DCMAKE_BUILD_TYPE=Release \
 -DSPM_ENABLE_SHARED=OFF \
 -DSPM_ENABLE_TCMALLOC=OFF \
 -DCMAKE_INSTALL_PREFIX="$LIBS_DIR/sentencepiece"

cmake --build "$SP_BUILD" --parallel "$CORES"
cmake --install "$SP_BUILD"

################################################################################
# FAISS
################################################################################

echo ""
echo "Building FAISS"

clone_repo \
https://github.com/facebookresearch/faiss.git \
"$FAISS_TAG" \
"$FAISS_SRC"

FAISS_FLAGS="\
-DFAISS_ENABLE_GPU=OFF \
-DFAISS_ENABLE_PYTHON=OFF \
-DFAISS_ENABLE_C_API=ON \
-DFAISS_ENABLE_TESTS=OFF \
-DFAISS_ENABLE_BENCHMARKS=OFF \
-DFAISS_ENABLE_OPENMP=OFF \
-DFAISS_OPT_LEVEL=generic"

CXX_FLAGS="-O2 -fno-tree-vectorize -fno-strict-aliasing"

if [[ "$OS" == "Darwin" ]]; then
    FAISS_FLAGS="$FAISS_FLAGS -DFAISS_ENABLE_OPENMP=ON"
    CXX_FLAGS="$CXX_FLAGS $OPENMP_FLAGS"
fi

cmake -S "$FAISS_SRC" -B "$FAISS_BUILD" \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX="$LIBS_DIR/faiss" \
 -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
 $FAISS_FLAGS

cmake --build "$FAISS_BUILD" --parallel "$CORES"
cmake --install "$FAISS_BUILD"

################################################################################
# ONNX Runtime (prebuilt binaries)
################################################################################

echo ""
echo "Installing ONNX Runtime $ONNX_VERSION"

TMP_DIR="$(mktemp -d)"

if [[ "$OS" == "Darwin" ]]; then
    FILE="onnxruntime-osx-arm64-${ONNX_VERSION}.tgz"
    DEST="macos"
elif [[ "$ARCH" == "x86_64" ]]; then
    FILE="onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
    DEST="linux64"
else
    FILE="onnxruntime-linux-aarch64-${ONNX_VERSION}.tgz"
    DEST="linuxarm64"
fi

URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/${FILE}"

curl -L "$URL" -o "$TMP_DIR/$FILE"
tar -xzf "$TMP_DIR/$FILE" -C "$TMP_DIR"

rm -rf "$ONNX_DIR"
mkdir -p "$ONNX_DIR/lib/$DEST"

cp -r "$TMP_DIR"/onnxruntime*/include "$ONNX_DIR/"
cp "$TMP_DIR"/onnxruntime*/lib/* "$ONNX_DIR/lib/$DEST/"

rm -rf "$TMP_DIR"

################################################################################
# Done
################################################################################

echo ""
echo "All dependencies built successfully."
echo "Installed into:"
echo "$LIBS_DIR"
