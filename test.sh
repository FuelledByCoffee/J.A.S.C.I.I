#!/bin/bash
source emsdk/emsdk_env.sh
cd frontend
# emrun index.html

emrun --browser="/mnt/c/Program Files/Google/Chrome/Application/chrome.exe" index.html \
|| emrun --browser="/Applications/Safari.app/Contents/MacOS/Safari" index.html

