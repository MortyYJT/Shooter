#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

scripts/build.sh
mkdir -p out/dist
jar --create --file out/dist/bullet-bloom.jar --main-class bulletbloom.app.GameApp -C out/classes .
