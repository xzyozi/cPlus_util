#!/bin/bash
set -euo pipefail

TARGET_EXEC="./testChildPrcKnr"
GDB_SCRIPT="manual_debug.gdb"

echo "====================================================="
echo " 手動デバッグモードを開始します。"
echo " ブレイクポイントで停止したら、内容を確認して 'c' (Enter) を"
echo " 押して次のステップに進んでください。"
echo "====================================================="

gdb -q -x "$GDB_SCRIPT" --args "$TARGET_EXEC"