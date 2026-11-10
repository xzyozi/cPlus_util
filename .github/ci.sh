#!/usr/bin/env bash
set -euo pipefail

for command in g++ make gdb; do
  if ! command -v "$command" >/dev/null 2>&1; then
    echo "Required command not found: $command" >&2
    exit 1
  fi
done

cd libGdb/lib
./gdb_debug.sh
