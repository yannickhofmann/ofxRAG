# ofxRAG

openFrameworks addon for low-level Retrieval-Augmented Generation (RAG).

## What is ofxRAG?

ofxRAG brings retrieval-augmented context building into the openFrameworks ecosystem. It acts as a low-level bridge between:

* **openFrameworks (oF)**  
* **SentencePiece**  
* **ONNX Runtime**  
* **FAISS**  
  
ofxRAG focuses exclusively on retrieval and context preparation. It does not include any text generation logic.

Instead, it is designed to be fully compatible with [ofxLlamaCpp](https://github.com/yannickhofmann/ofxLlamaCpp), which can be used as a separate, local runtime backend for text generation using GGUF LLaMA-family models.

Together, ofxRAG and ofxLlamaCpp enable fully local, offline, and transparent Retrieval-Augmented Generation pipelines for creative coding projects, interactive installations, research systems, and long-running real-time applications—without Python, cloud services, or high-level framework abstractions.

## Tested Environments

ofxRAG has been validated on:

*   Ubuntu 24.04.3 LTS
*   macOS (Apple Silicon M2)
*   openFrameworks:
    *   `of_v0.12.0_linux64gcc6_release`
    *   `of_v0.12.1_linux64_gcc6_release`


## Dependencies

* **SentencePiece**  
  Subword tokenization library for text inputs, ensuring model-consistent and language-agnostic preprocessing.  
  [https://github.com/google/sentencepiece](https://github.com/google/sentencepiece)

* **ONNX Runtime**  
  High-performance inference engine for executing pre-exported embedding models in a deployment-ready environment.  
  [https://github.com/microsoft/onnxruntime](https://github.com/microsoft/onnxruntime)

* **FAISS**  
  Efficient similarity search and clustering library for large-scale vectorized document collections.  
  [https://github.com/facebookresearch/faiss](https://github.com/facebookresearch/faiss)


### Build Tool Dependency

* **CMake**  
  CMake is required to configure and build the native dependencies used by ofxRAG, specifically SentencePiece and FAISS.  
  ONNX Runtime is integrated as a prebuilt binary and does not require compilation.

CMake is cross-platform and must be installed before
    attempting to build these libraries

**Ubuntu / Debian (Linux):**

```bash
sudo apt update
sudo apt install cmake
```

**macOS (OS X):**

```bash
brew install cmake
```

### Example Dependencies

* **ofxDropdown** (developed by [roymacdonald](https://github.com/roymacdonald)) is used in the example_chat to provide a graphical user interface for selecting LLM models and chat templates via a dropdown menu.
    [https://github.com/roymacdonald/ofxDropdown](https://github.com/roymacdonald/ofxDropdown)
* **ofxLlamaCpp** (developed by [yannickhofmann](https://github.com/yannickhofmann)) is used in the example_chat to provide local, offline text generation using GGUF LLaMA-family models.
    [https://github.com/yannickhofmann/ofxLlamaCpp](https://github.com/yannickhofmann/ofxLlamaCpp)
* **ofxPoDoFo** (developed by [nariakiiwatani](https://github.com/nariakiiwatani), forked and edited by Yannick Hofmann [yannickhofmann](https://github.com/yannickhofmann)) is used in example_chat to enable information retrieval from PDFs.
    [https://github.com/yannickhofmann/ofxPoDoFo](https://github.com/yannickhofmann/ofxPoDoFo)

## Setup

In the `scripts` folder, you will find a shell script to automatically clone and build the proper libraries. Navigate to the `scripts` folder and execute:
```bash
chmod +x install_deps.sh
./install_deps.sh
```

ofxRAG uses pre-exported ONNX embedding models and their corresponding SentencePiece tokenizers.
A helper script is provided to download compatible models into the addon's local models/ directory.

From the scripts folder, run:
```bash
chmod +x download_models.sh
./download_models.sh
```

### Build and Run the Examples

Once the static libraries are compiled and the models are in place, you can build and run the example projects.
The example_chat additionally requires at least one GGUF language model to be placed manually in the `data/models` folder.  
Please refer to the `notes.txt` file inside the `data/models` directory for details and model recommendations.

Navigate into the example folder you wish to build (e.g., `example_search`):

```bash
cd example_search
```

Run make to compile the openFrameworks example:

```bash
make
```

Run the release executable:

```bash
make RunRelease
```

## License

Copyright (c) 2025 Yannick Hofmann.

BSD Simplified License.

For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the file, "LICENSE.txt," in this distribution.
