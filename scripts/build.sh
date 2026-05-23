#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

rm -rf out/classes
mkdir -p out/classes
javac -d out/classes $(find src/main/java -name '*.java')
cp -R src/main/resources/* out/classes/
