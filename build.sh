#!/bin/bash
set -e

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$ROOT_DIR/frontend/scripts"

git -C "$ROOT_DIR" submodule update --init --recursive

cd "$ROOT_DIR/emsdk"
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd "$ROOT_DIR"

cmake -S "$ROOT_DIR/src" -B "$ROOT_DIR/build-web" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_EXECUTABLE_SUFFIX=".js" \
	-DCMAKE_TOOLCHAIN_FILE="$ROOT_DIR/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
	-DCMAKE_EXE_LINKER_FLAGS="\
		-sINVOKE_RUN=0 \
	    -sEXIT_RUNTIME=0 \
	    -sALLOW_MEMORY_GROWTH=1 \
		-sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall"

cmake --build "$ROOT_DIR/build-web"

ln -sfn "$ROOT_DIR/build-web/asciiart.js" "$ROOT_DIR/frontend/scripts/asciiart.js"
ln -sfn "$ROOT_DIR/build-web/asciiart.wasm" "$ROOT_DIR/frontend/scripts/asciiart.wasm"

npm install --prefix "$ROOT_DIR"
cd "$ROOT_DIR/frontend"
npm install ejs
npm install
curl -L https://unpkg.com/@tailwindcss/browser@4 -o scripts/tailwind-browser.js
cd "$ROOT_DIR"
npm install --prefix "$ROOT_DIR"
