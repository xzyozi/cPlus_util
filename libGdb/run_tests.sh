#!/bin/bash
# ============================================================
# libGdb: ケース表（cases/*.tsv）を1ケースずつ別プロセスで実行し、良/否を判定する
#
# 使い方（このディレクトリで実行）:
#   ./run_tests.sh                      # cases/*.tsv を全件実行
#   ./run_tests.sh cases/sample.tsv     # 指定ファイルのみ
#   ./run_tests.sh --filter 'S0[1-3]'   # ID が glob に一致するケースのみ
#   ./run_tests.sh --gdb                # gdb 経由で実行し、gdb_probe の入出力を証跡に保存する
#
# ケース表の形式（タブ区切り・UTF-8・LF。# で始まる行はコメント）:
#   id  case  exp_ret  exp_value
#   exp_value の - は「照合しない」
#
# 判定: 良 / 否
#
# 証跡: generated/evidence/<実行日時>/<id>.log（実行ごとに分離。過去の実行分は消さない）
#   各ログの先頭付近に入力（IN_*=）、続いて RET= / VALUE= を記録する。
#   --gdb 時は gdb_probe の観測点ごとに、GDB のネイティブ表示（p value）が入る。
#
# 終了コード: 0=全件 良 / 1=否あり / 2=準備不備
# 実行環境: Linux（g++ + gdb）。事前に make 済みであること。
# ============================================================
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BIN="./generated/libGdbSample"
GENDIR="./generated"
EVIDENCE_ROOT="${GENDIR}/evidence"
USE_GDB=0
FILTER="*"
CASE_FILES=()

usage() {
  cat <<'USAGEEOF'
使い方:
  ./run_tests.sh                        cases/*.tsv を全件実行
  ./run_tests.sh cases/sample.tsv       指定ファイルのみ
  ./run_tests.sh --filter 'S0[1-3]'     ID が glob に一致するケースのみ
  ./run_tests.sh --gdb                  gdb 経由で実行し、gdb_probe の入出力を証跡に保存する
終了コード: 0=全件 良 / 1=否あり / 2=準備不備
USAGEEOF
}

while [ $# -gt 0 ]; do
  case "$1" in
    --gdb)    USE_GDB=1 ;;
    --filter) FILTER="${2:-*}"; shift ;;
    -h|--help) usage; exit 0 ;;
    *) CASE_FILES+=("$1") ;;
  esac
  shift
done

if [ ${#CASE_FILES[@]} -eq 0 ]; then
  CASE_FILES=(cases/*.tsv)
fi

if [ ! -x "$BIN" ]; then
  echo "実行ファイルがありません: $BIN （先に make を実行してください）" >&2
  exit 2
fi
if [ "$USE_GDB" = "1" ] && ! command -v gdb >/dev/null 2>&1; then
  echo "gdb が見つかりません（--gdb は gdb が必要です）" >&2
  exit 2
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
EVIDENCE_DIR="${EVIDENCE_ROOT}/${STAMP}"
mkdir -p "$EVIDENCE_DIR"
RESULT_FILE="${GENDIR}/result_${STAMP}.tsv"
printf 'id\t結果\t詳細\n' > "$RESULT_FILE"

# gdb 実行用スクリプト（対象非依存の汎用形。gdb_probe の value を丸ごと出力）
GDB_SCRIPT="${GENDIR}/probe.gdb"
if [ "$USE_GDB" = "1" ]; then
  cat > "$GDB_SCRIPT" <<'GDBEOF'
set pagination off
set confirm off
set breakpoint pending on
set print elements 0
set print repeats 0
set print pretty on
set print symbol-filename off
set unwindonsignal on

# 汎用観測点マーカー gdb_probe(label, value) で停止し、value を
# GDB のネイティブ表示で丸ごと出力する。対象に依存しない。
# gdb_probe はテンプレートのため実体名は gdb_probe<T>(char const*, T const&)。
# プレーン名の break では張れないので、正規表現で全インスタンスに張れる rbreak を使う。
rbreak ^gdb_probe
commands
  silent
  printf "\n[GDB] === probe: %s ===\n", label
  printf "(gdb) p value\n"
  p value
  continue
end
run
GDBEOF
fi

PASS=0
FAIL=0

# run_case <id> <exp_ret> <exp_value> <driver args...>
run_case() {
  local id="$1" exp_ret="$2" exp_val="$3"
  shift 3
  local out ret value detail="" ok=1

  if [ "$USE_GDB" = "1" ]; then
    gdb -q -batch -x "$GDB_SCRIPT" --args "$BIN" "$@" > "${EVIDENCE_DIR}/${id}.log" 2>&1
    out="$(cat "${EVIDENCE_DIR}/${id}.log")"
  else
    out="$("$BIN" "$@" 2>&1)"
    printf '%s\n' "$out" > "${EVIDENCE_DIR}/${id}.log"
  fi

  ret="$(printf '%s\n' "$out" | grep -m1 '^RET=' | sed 's/^RET=//')"
  value="$(printf '%s\n' "$out" | grep -m1 '^VALUE=' | sed 's/^VALUE=//')"

  if [ "$ret" != "$exp_ret" ]; then
    ok=0; detail="${detail}戻り値 期待[${exp_ret}] 実際[${ret}]; "
  fi
  if [ "$exp_val" != "-" ] && [ "$value" != "$exp_val" ]; then
    ok=0; detail="${detail}出力値 期待[${exp_val}] 実際[${value}]; "
  fi

  if [ "$ok" = "1" ]; then
    PASS=$((PASS + 1))
    printf '%-6s 良  %s\n' "$id" "$detail"
    printf '%s\t良\t%s\n' "$id" "$detail" >> "$RESULT_FILE"
  else
    FAIL=$((FAIL + 1))
    printf '%-6s 否  %s\n' "$id" "$detail"
    printf '%s\t否\t%s\n' "$id" "$detail" >> "$RESULT_FILE"
  fi
}

for cf in "${CASE_FILES[@]}"; do
  [ -f "$cf" ] || { echo "ケース表が見つかりません: $cf" >&2; exit 2; }
  echo "===== $cf ====="
  while IFS= read -r line || [ -n "$line" ]; do
    line="${line%$'\r'}"
    case "$line" in
      ""|"#"*) continue ;;
    esac
    IFS=$'\t' read -r -a f <<< "$line"
    id="${f[0]}"
    # shellcheck disable=SC2053
    [[ "$id" == $FILTER ]] || continue
    [ ${#f[@]} -eq 4 ] || { echo "$id: 列数不正(4列必要)" >&2; FAIL=$((FAIL + 1)); continue; }
    run_case "$id" "${f[2]}" "${f[3]}" sample "${f[1]}"
  done < "$cf"
done

echo "-----------------------------------------"
echo "良=${PASS} 否=${FAIL}  結果: ${RESULT_FILE}  証跡: ${EVIDENCE_DIR}/"
if [ "$FAIL" -ne 0 ]; then
  exit 1
fi
exit 0
