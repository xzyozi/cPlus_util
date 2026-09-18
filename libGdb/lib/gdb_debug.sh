#!/bin/bash
# ============================================================
# libGdb: build → .gdb 自動生成 → test 実行 を1本で行うランナー
#
# ディレクトリ構成（このスクリプトは lib/ にある）:
#   libGdb/
#   ├── lib/        … ライブラリ本体（このスクリプト・Makefile・header.h）
#   ├── src/        … 可変ファイル（sample: テスト対象を書く）
#   └── generated/  … 自動生成物（obj・実行ファイル・.gdb）。make で毎回作り直す
#
# 流れ:
#   1. make で build（既定 USE_LIBS=0＝スタンドアローン。引数で切替可）
#      -> make が generated/ を作り直し、実行ファイルを generated/ に出力
#   2. dump 用 .gdb を generated/ に自動生成
#   3. 生成した .gdb で gdb を起動し、test を実行してエビデンスを採取
#
# 使い方（lib/ で実行）:
#   ./gdb_debug.sh                 # スタンドアローン build → 実行
#   ./gdb_debug.sh USE_LIBS=1      # 業務ライブラリ使用 build → 実行
#
# 実行環境: Linux + g++ + gdb（PostgreSQL 不要）
# ============================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

GENDIR="../generated"
TARGET_EXEC="${GENDIR}/testChildPrcKnr"
GEN_GDB="${GENDIR}/dump_generated.gdb"

# make へ渡す追加引数（例: USE_LIBS=1）をそのまま透過する
MAKE_ARGS=("$@")

echo "====================================================="
echo " [1/3] build (make ${MAKE_ARGS[*]:-})"
echo "====================================================="
# make が generated/ を削除して作り直す（生成物は make 時に作り直す想定）
make "${MAKE_ARGS[@]}"

echo "====================================================="
echo " [2/3] generate gdb script -> ${GEN_GDB}"
echo "====================================================="
mkdir -p "${GENDIR}"
# dump 用 .gdb をヒアドキュメントで自動生成する。
# 観測点（ダミー関数）でブレイクして結果を出力する定型。
cat > "${GEN_GDB}" <<'GDBEOF'
set pagination off
set confirm off
set breakpoint pending on
set print elements 0
set print repeats 0
set print pretty on
set print symbol-filename off
set unwindonsignal on

# 汎用観測点マーカー gdb_probe(label, value) で停止し、value を
# GDB のネイティブ表示で丸ごと出力する。対象に依存しない完全汎用形。
# 見せたいメンバの列挙は不要（value を p するだけ）。
break gdb_probe
commands
  silent
  printf "\n[GDB] === probe: %s ===\n", label
  printf "(gdb) p value\n"
  p value
  continue
end

run
GDBEOF

echo "====================================================="
echo " [3/3] run test (gdb -x ${GEN_GDB})"
echo "====================================================="
gdb -q -x "${GEN_GDB}" --args "${TARGET_EXEC}"
