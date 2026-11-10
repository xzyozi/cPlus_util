#!/usr/bin/env bash
set -euo pipefail

for command in g++ make; do
  if ! command -v "$command" >/dev/null 2>&1; then
    echo "Required command not found: $command" >&2
    exit 1
  fi
done

cd libGdb/lib
if command -v gdb >/dev/null 2>&1; then
  ./gdb_debug.sh
else
  echo "gdb is not available; running the build-only validation."
  make
fi
