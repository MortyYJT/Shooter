#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

scripts/build.sh
rm -rf out/test-classes
mkdir -p out/test-classes
javac -cp out/classes -d out/test-classes $(find src/test/java -name '*.java')
java -cp out/classes:out/test-classes bulletbloom.GameSmokeTest
