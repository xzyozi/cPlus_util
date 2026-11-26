#!/usr/bin/env bash
# ============================================================
# リポジトリ直下の各ライブラリディレクトリを順に検証する。
#
# 対象: 直下のディレクトリのうち Makefile を持つもの（ci.sh の変更なしで自動追加）
#   1. make でビルドする
#   2. run_tests.sh があれば実行し、終了コードで合否を判定する
#      - gdb があり、run_tests.sh が --gdb に対応していれば --gdb 付きで実行する
#
# 1 つでも失敗したら、全ライブラリを処理した後に終了コード 1 で終わる。
# ============================================================
set -uo pipefail

for command in g++ make; do
  if ! command -v "$command" >/dev/null 2>&1; then
    echo "Required command not found: $command" >&2
    exit 1
  fi
done

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

HAS_GDB=0
if command -v gdb >/dev/null 2>&1; then
  HAS_GDB=1
fi

passed=()
failed=()

for dir in */; do
  lib="${dir%/}"
  [ -f "$lib/Makefile" ] || continue

  echo "::group::$lib"
  status=0

  echo "[$lib] make"
  if ! make -C "$lib"; then
    echo "::error::[$lib] build failed"
    status=1
  fi

  if [ "$status" -eq 0 ] && [ -f "$lib/run_tests.sh" ]; then
    test_args=()
    if [ "$HAS_GDB" -eq 1 ] && grep -q -- '--gdb' "$lib/run_tests.sh"; then
      test_args=(--gdb)
    fi
    echo "[$lib] run_tests.sh ${test_args[*]:-}"
    if ! bash "$lib/run_tests.sh" "${test_args[@]}"; then
      echo "::error::[$lib] tests failed"
      status=1
    fi
  fi

  echo "::endgroup::"

  if [ "$status" -eq 0 ]; then
    passed+=("$lib")
  else
    failed+=("$lib")
  fi
done

echo "== CI summary =="
echo "passed: ${passed[*]:-(none)}"
echo "failed: ${failed[*]:-(none)}"

if [ "${#passed[@]}" -eq 0 ] && [ "${#failed[@]}" -eq 0 ]; then
  echo "No library directory with a Makefile was found." >&2
  exit 1
fi

if [ "${#failed[@]}" -gt 0 ]; then
  exit 1
fi
