#!/bin/bash

# This script downloads the ONNX models required for the ofxRAG addon.
# It creates a 'models' directory in the addon's root and downloads the models into it.

# Get the directory of the script itself, regardless of where it's called from
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# The root directory of the addon is one level up from the script's directory
ADDON_ROOT="$SCRIPT_DIR/.."
MODELS_DIR="$ADDON_ROOT/models"

# Function to download a file from a URL to a specific directory
download_file() {
  local url=$1
  local dest_dir=$2
  local filename=$(basename "$url" | sed 's/?download=true//')

  echo "Downloading $filename to $dest_dir..."
  curl -L -o "$dest_dir/$filename" "$url"
}

# --- Main Script ---

echo "Creating models directory at $MODELS_DIR..."
mkdir -p "$MODELS_DIR"

# --- Text Embeddings ---

echo "Downloading Text Embedding models..."
TEXT_EMBEDDINGS_DIR="$MODELS_DIR/text_embeddings"
mkdir -p "$TEXT_EMBEDDINGS_DIR/sentence-t5-base"

download_file "https://huggingface.co/onnx-models/sentence-t5-base-onnx/resolve/main/model.onnx?download=true" "$TEXT_EMBEDDINGS_DIR/sentence-t5-base"
download_file "https://huggingface.co/onnx-models/sentence-t5-base-onnx/resolve/main/spiece.model?download=true" "$TEXT_EMBEDDINGS_DIR/sentence-t5-base"

echo "All models downloaded successfully."
