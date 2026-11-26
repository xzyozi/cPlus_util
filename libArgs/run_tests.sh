#!/bin/bash
# ============================================================
# libArgs: ケース表（cases/*.tsv）を1ケースずつ別プロセスで実行し、良/否を判定する
#
# 使い方（どのディレクトリからでも可。スクリプトのディレクトリへ移動して動く）:
#   bash run_tests.sh                      # cases/*.tsv を全件実行
#   bash run_tests.sh cases/sample.tsv     # 指定ファイルのみ
#   bash run_tests.sh --filter 'E0[1-3]'   # ID が glob に一致するケースのみ
#
# ケース表の形式（タブ区切り・UTF-8・LF。# で始まる行はコメント。CR は除去して読む）:
#   id  args  exp_ret  exp_value
#   args は空白で単語分割してドライバへ渡す（クォートは解釈しない）
#   exp_value の - は「照合しない」
#
# 判定: 良 / 否（RET=、VALUE=、終了コード NML=0 / ERR=2 を照合する）
#
# 結果: generated/result_<実行日時>.tsv
# 証跡: generated/evidence/<実行日時>/<id>.log（実行ごとに分離。過去の実行分は消さない）
#   各ログにドライバの出力（IN_ARGS= / MSG= / RET= / VALUE=）と EXIT= を記録する。
#
# 終了コード: 0=全件 良 / 1=否あり / 2=準備不備
# 実行環境: Linux（bash）。事前に make 済みであること。
# ============================================================
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 2

BIN="./generated/libArgsSample"
GENDIR="./generated"
EVIDENCE_ROOT="${GENDIR}/evidence"
FILTER="*"
CASE_FILES=()

usage() {
  cat <<'USAGEEOF'
使い方:
  bash run_tests.sh                        cases/*.tsv を全件実行
  bash run_tests.sh cases/sample.tsv       指定ファイルのみ
  bash run_tests.sh --filter 'E0[1-3]'     ID が glob に一致するケースのみ
終了コード: 0=全件 良 / 1=否あり / 2=準備不備
USAGEEOF
}

while [ $# -gt 0 ]; do
  case "$1" in
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

STAMP="$(date +%Y%m%d_%H%M%S)"
EVIDENCE_DIR="${EVIDENCE_ROOT}/${STAMP}"
mkdir -p "$EVIDENCE_DIR"
RESULT_FILE="${GENDIR}/result_${STAMP}.tsv"
printf 'id\t結果\t詳細\n' > "$RESULT_FILE"

PASS=0
FAIL=0

# 期待する終了コード（NML=0 / ERR=2）
expected_exit() {
  case "$1" in
    NML) echo 0 ;;
    ERR) echo 2 ;;
    *) echo "?" ;;
  esac
}

# run_case <id> <exp_ret> <exp_value> <driver args...>
run_case() {
  local id="$1" exp_ret="$2" exp_val="$3"
  shift 3
  local out rc ret value exp_rc detail="" ok=1

  out="$("$BIN" "$@" < /dev/null 2>&1)"
  rc=$?
  printf '%s\nEXIT=%s\n' "$out" "$rc" > "${EVIDENCE_DIR}/${id}.log"

  ret="$(printf '%s\n' "$out" | grep -m1 '^RET=' | sed 's/^RET=//')"
  value="$(printf '%s\n' "$out" | grep -m1 '^VALUE=' | sed 's/^VALUE=//')"
  exp_rc="$(expected_exit "$exp_ret")"

  if [ "$ret" != "$exp_ret" ]; then
    ok=0; detail="${detail}戻り値 期待[${exp_ret}] 実際[${ret}]; "
  fi
  if [ "$rc" != "$exp_rc" ]; then
    ok=0; detail="${detail}終了コード 期待[${exp_rc}] 実際[${rc}]; "
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
    if [ ${#f[@]} -ne 4 ]; then
      echo "$id: 列数不正(4列必要)" >&2
      FAIL=$((FAIL + 1))
      printf '%s\t否\t%s\n' "$id" "列数不正" >> "$RESULT_FILE"
      continue
    fi
    read -r -a case_args <<< "${f[1]}"
    run_case "$id" "${f[2]}" "${f[3]}" "${case_args[@]}"
  done < "$cf"
done

echo "-----------------------------------------"
echo "良=${PASS} 否=${FAIL}  結果: ${RESULT_FILE}  証跡: ${EVIDENCE_DIR}/"
if [ "$PASS" -eq 0 ] && [ "$FAIL" -eq 0 ]; then
  echo "実行したケースがありません" >&2
  exit 1
fi
if [ "$FAIL" -ne 0 ]; then
  exit 1
fi
exit 0
