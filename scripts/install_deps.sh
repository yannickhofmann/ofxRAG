#!/usr/bin/env bash
set -euo pipefail

################################################################################
# Versions (locked, platform-specific)
################################################################################

# SentencePiece
SENTENCEPIECE_COMMIT_MAC="3863f7648e5d8edb571ac592f3ac4f5f0695275a"
SENTENCEPIECE_TAG_LINUX="v0.1.99"

# FAISS
FAISS_MAC_COMMIT="943d08bdad7946b22f56d040756669ee444dd681"
FAISS_LINUX_TAG="v1.8.0"

# ONNX Runtime
ONNX_VERSION="1.17.1"

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
# Platform detection
################################################################################

OS="$(uname -s)"
ARCH="$(uname -m)"

echo "Addon root : $ADDON_ROOT"
echo "OS         : $OS"
echo "Arch       : $ARCH"

################################################################################
# Reset build directories
################################################################################

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR" "$LIBS_DIR"

################################################################################
# macOS arm64
################################################################################

if [[ "$OS" == "Darwin" && "$ARCH" == "arm64" ]]; then
  echo "Platform   : macOS arm64"

  # ---------------------------------------------------------------------------
  # libomp (required for FAISS on macOS)
  #
  # AppleClang does not ship OpenMP.
  # FAISS CMake will fail without libomp:
  # https://github.com/facebookresearch/faiss/issues/433
  # ---------------------------------------------------------------------------

  if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew not found. Install Homebrew first."
    exit 1
  fi

  echo "Installing libomp"
  brew install libomp || true
  LIBOMP_PREFIX="$(brew --prefix libomp)"

  if [[ ! -f "$LIBOMP_PREFIX/include/omp.h" ]]; then
    echo "omp.h not found in $LIBOMP_PREFIX/include"
    exit 1
  fi

  # ---------------------------------------------------------------------------
  # SentencePiece (macOS — pinned commit + lowered CMake policy)
  # ---------------------------------------------------------------------------

  echo "Building SentencePiece (macOS)"

  git clone https://github.com/google/sentencepiece.git "$SP_SRC"
  cd "$SP_SRC"
  git checkout "$SENTENCEPIECE_COMMIT_MAC"

  cmake -S "$SP_SRC" -B "$SP_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$LIBS_DIR/sentencepiece"

  cmake --build "$SP_BUILD" --parallel
  cmake --install "$SP_BUILD"

  # ---------------------------------------------------------------------------
  # FAISS (macOS, OpenMP ON)
  # ---------------------------------------------------------------------------

  echo "Building FAISS (macOS)"

  git clone https://github.com/facebookresearch/faiss.git "$FAISS_SRC"
  cd "$FAISS_SRC"
  git checkout "$FAISS_MAC_COMMIT"

  cmake -S "$FAISS_SRC" -B "$FAISS_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$LIBS_DIR/faiss" \
    -DFAISS_ENABLE_GPU=OFF \
    -DFAISS_ENABLE_PYTHON=OFF \
    -DFAISS_ENABLE_OPENMP=ON \
    -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp" \
    -DOpenMP_CXX_LIB_NAMES="omp" \
    -DOpenMP_omp_LIBRARY="$LIBOMP_PREFIX/lib/libomp.dylib" \
    -DCMAKE_CXX_FLAGS="-I$LIBOMP_PREFIX/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-L$LIBOMP_PREFIX/lib"

  cmake --build "$FAISS_BUILD" --parallel
  cmake --install "$FAISS_BUILD"

  # ---------------------------------------------------------------------------
  # ONNX Runtime (macOS arm64, prebuilt)
  # ---------------------------------------------------------------------------

  echo "Installing ONNX Runtime ${ONNX_VERSION} (macOS arm64)"

  TMP_DIR="$(mktemp -d)"
  ONNX_TGZ="onnxruntime-osx-arm64-${ONNX_VERSION}.tgz"
  ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/${ONNX_TGZ}"

  curl -L "$ONNX_URL" -o "$TMP_DIR/$ONNX_TGZ"
  tar -xzf "$TMP_DIR/$ONNX_TGZ" -C "$TMP_DIR"

  rm -rf "$ONNX_DIR"
  mkdir -p "$ONNX_DIR/lib/macos"

  cp -r "$TMP_DIR/onnxruntime-osx-arm64-${ONNX_VERSION}/include" "$ONNX_DIR/"
  cp "$TMP_DIR/onnxruntime-osx-arm64-${ONNX_VERSION}/lib/"* \
     "$ONNX_DIR/lib/macos/"

  rm -rf "$TMP_DIR"

  echo "macOS dependencies installed successfully"
  exit 0
fi

################################################################################
# Linux x86_64
################################################################################

if [[ "$OS" == "Linux" && "$ARCH" == "x86_64" ]]; then
  echo "Platform   : linux x86_64"

  # ---------------------------------------------------------------------------
  # SentencePiece (Linux)
  # ---------------------------------------------------------------------------

  echo "Building SentencePiece (Linux)"

  git clone --depth 1 --branch "$SENTENCEPIECE_TAG_LINUX" \
    https://github.com/google/sentencepiece.git "$SP_SRC"

  cmake -S "$SP_SRC" -B "$SP_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPM_ENABLE_SHARED=OFF \
    -DSPM_ENABLE_TCMALLOC=OFF

  cmake --build "$SP_BUILD" --parallel
  cmake --install "$SP_BUILD" --prefix "$LIBS_DIR/sentencepiece"

  # ---------------------------------------------------------------------------
  # FAISS (Linux, OpenMP OFF)
  # ---------------------------------------------------------------------------

  echo "Building FAISS (Linux)"

  git clone --depth 1 --branch "$FAISS_LINUX_TAG" \
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

  # ---------------------------------------------------------------------------
  # ONNX Runtime (Linux, prebuilt)
  # ---------------------------------------------------------------------------

  echo "Installing ONNX Runtime ${ONNX_VERSION} (Linux)"

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

  echo "Linux dependencies installed successfully"
  exit 0
fi


################################################################################
# Cleanup build directory
################################################################################

echo ""
echo "Cleaning up build directory (.build)"
rm -rf "$BUILD_DIR"

################################################################################
# Unsupported platform
################################################################################

echo "Unsupported platform: $OS $ARCH"
exit 1
