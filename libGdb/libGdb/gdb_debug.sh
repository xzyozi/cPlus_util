#!/bin/bash
set -euo pipefail

# このスクリプトは libGdb/ 配下にあるが、実行ファイル・gdbスクリプトは
# ルート基準で解決する（ルートから実行される想定）。
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

TARGET_EXEC="$ROOT_DIR/testChildPrcKnr"
GDB_SCRIPT="$SCRIPT_DIR/manual_debug.gdb"

echo "====================================================="
echo " 手動デバッグモードを開始します。"
echo " ブレイクポイントで停止したら、内容を確認して 'c' (Enter) を"
echo " 押して次のステップに進んでください。"
echo "====================================================="

gdb -q -x "$GDB_SCRIPT" --args "$TARGET_EXEC"
