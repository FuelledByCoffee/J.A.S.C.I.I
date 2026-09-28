#!/bin/bash
set -e

mkdir -p frontend/scripts

git submodule update --init --recursive

cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ../src

emcmake cmake -B build
# emmake make -j target=a.out.js config=release LDFLAGS="-sINVOKE_RUN=0  -sEXIT_RUNTIME=0  -sALLOW_MEMORY_GROWTH=1  -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap,ccall"

ln -s build/a.out.js ../frontend/scripts
ln -s build/a.out.wasm ../frontend/scripts
cd ..

npm install
cd frontend
npm install ejs
npm install
curl -L https://unpkg.com/@tailwindcss/browser@4 -o scripts/tailwind-browser.js
cd ..
npm install
