#!/bin/bash
set -e

mkdir -p frontend/scripts

git submodule update --init --recursive

cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ../src

cmake -B build --fresh -DCMAKE_TOOLCHAIN_FILE="../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
cmake --build build
# emmake make -j target=asciiart.js config=release LDFLAGS="-sINVOKE_RUN=0  -sEXIT_RUNTIME=0  -sALLOW_MEMORY_GROWTH=1  -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall"

ln -sfn ../../src/build/asciiart.js ../frontend/scripts/asciiart.js
ln -sfn ../../src/build/asciiart.wasm ../frontend/scripts/asciiart.wasm
cd ..

npm install
cd frontend
npm install ejs
npm install
curl -L https://unpkg.com/@tailwindcss/browser@4 -o scripts/tailwind-browser.js
cd ..
npm install
