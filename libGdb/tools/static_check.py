#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""libGdb 静的解析スクリプト（実機の g++/gdb を使わない検証）。

実機（Linux + g++ + gdb）でのビルド・実行が難しい環境向けに、
GDB を動かさずに検証できる範囲を静的にチェックする。

検査項目:
  1. ディレクトリ構成（lib/ src/ の必須ファイル、generated は Git 管理外）
  2. include 解決（#include "..." が -I lib -I src で解決できるか）
  3. 業務依存の残存（Cgt/CGT/mpf/pmf/DB API・業務 include が残っていないか）
  4. 観測点の整合（main.cpp のダミー関数と、gdb_debug.sh が生成する .gdb の
     break 対象が一致するか）
  5. .gdb が参照する構造体メンバが CalResult に存在するか
  6. Makefile / gdb_debug.sh のパス整合（SRCDIR/GENDIR 等）
  7. 宣言と定義の対応（hpp の関数宣言が cpp に定義されているか・簡易）

使い方:
  python libGdb/tools/static_check.py            # libGdb を自動検出
  python libGdb/tools/static_check.py <libGdb>   # ルートを明示

終了コード: 0=エラーなし / 1=エラーあり
標準ライブラリのみ使用。
"""

import os
import re
import sys

# ---- 結果集計 -------------------------------------------------
ERRORS = []
WARNINGS = []
OKS = []


def err(msg):
    ERRORS.append(msg)


def warn(msg):
    WARNINGS.append(msg)


def ok(msg):
    OKS.append(msg)


def read(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


# ---- 1. ディレクトリ構成 --------------------------------------
def check_layout(root):
    required = {
        "lib/Makefile": "ライブラリ本体: ビルド定義",
        "lib/gdb_debug.sh": "ライブラリ本体: ランナー",
        "lib/gdb_probe.hpp": "ライブラリ本体: 汎用観測点マーカー",
        "lib/header.h": "ライブラリ本体: 共通ヘッダ",
        "src/main.cpp": "可変: テストシナリオ",
        "src/childResultTest.hpp": "可変: テスト対象ヘッダ",
        "src/childResultTest.cpp": "可変: テスト対象実装",
        "README.md": "説明",
        ".gitignore": "generated 除外",
    }
    for rel, desc in required.items():
        p = os.path.join(root, rel)
        if os.path.isfile(p):
            ok("layout: {} ({})".format(rel, desc))
        else:
            err("layout: 必須ファイルが無い -> {} ({})".format(rel, desc))

    # generated は Git 管理外であるべき（.gitignore に記載）
    gi = os.path.join(root, ".gitignore")
    if os.path.isfile(gi):
        if re.search(r"(?m)^\s*generated/?\s*$", read(gi)):
            ok(".gitignore: generated/ を除外している")
        else:
            err(".gitignore: generated/ の除外記載が無い")


# ---- 2. include 解決 ------------------------------------------
def check_includes(root):
    # 探索パス（-I lib -I src 相当）とカレント（同ディレクトリ）
    search_dirs = [os.path.join(root, "lib"), os.path.join(root, "src")]
    cpp_files = [
        os.path.join(root, "src", "main.cpp"),
        os.path.join(root, "src", "childResultTest.cpp"),
        os.path.join(root, "lib", "header.h"),
        os.path.join(root, "src", "childResultTest.hpp"),
    ]
    inc_re = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)
    for cf in cpp_files:
        if not os.path.isfile(cf):
            continue
        text = read(cf)
        here = os.path.dirname(cf)
        for m in inc_re.finditer(text):
            inc = m.group(1)
            candidates = [os.path.join(here, inc)] + [
                os.path.join(d, inc) for d in search_dirs
            ]
            if any(os.path.isfile(c) for c in candidates):
                ok('include: {} -> "{}" 解決OK'.format(os.path.basename(cf), inc))
            else:
                err(
                    'include: {} の "{}" が -I lib -I src で解決できない'.format(
                        os.path.basename(cf), inc
                    )
                )


# ---- 3. 業務依存の残存 ----------------------------------------
def check_no_business_dep(root):
    # 実コードで出てはいけない業務・DB シンボル / include
    banned = [
        r"#\s*include\s*<Cgt",
        r"#\s*include\s*<Cmn",
        r"#\s*include\s*<mpf_",
        r"\bCgt0927ChildResultKnr\b",
        r"\bCgt0920AbsDate\b",
        r"\bCGT_CAL_RESULT_St\b",
        r"\bSdmAddRec\b",
        r"\bmpf_mfs_\w+",
        r"\bpmf_\w+",
        r"\bCMN0012_STRCPY\b",
        r"\bFNO_CGT_CAL_RESULT\b",
    ]
    targets = [
        os.path.join(root, "src", "main.cpp"),
        os.path.join(root, "src", "childResultTest.cpp"),
        os.path.join(root, "src", "childResultTest.hpp"),
        os.path.join(root, "lib", "header.h"),
    ]
    found_any = False
    for t in targets:
        if not os.path.isfile(t):
            continue
        text = read(t)
        # コメントを除外して実コードだけで判定する。
        #  - 行コメント（// 以降）を各行から除去
        #  - ブロックコメント（/* ... */）を除去
        no_block = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
        code_lines = []
        for ln in no_block.splitlines():
            # 行末コメント // 以降を落とす（文字列内 // は簡易的に無視されるが、
            # 本チェックの banned パターンには影響しない）
            code_lines.append(re.sub(r"//.*$", "", ln))
        code = "\n".join(code_lines)
        for pat in banned:
            if re.search(pat, code):
                found_any = True
                err(
                    "business-dep: {} に業務依存が残存 -> /{}/".format(
                        os.path.basename(t), pat
                    )
                )
    if not found_any:
        ok("business-dep: src/lib の実コードに業務・DB 依存なし")


# ---- 4/5. 観測点と生成 .gdb の整合 ----------------------------
def extract_generated_gdb(sh_text):
    # gdb_debug.sh のヒアドキュメント <<'GDBEOF' ... GDBEOF を取り出す
    m = re.search(r"<<'GDBEOF'\n(.*?)\nGDBEOF", sh_text, re.DOTALL)
    return m.group(1) if m else None


def check_probe_consistency(root):
    """案2（汎用テンプレートマーカー gdb_probe）方式の整合チェック。

    - 生成 .gdb は break gdb_probe / p value の完全汎用形であること
    - gdb_probe が lib/gdb_probe.hpp にテンプレート定義されていること
    - main.cpp が gdb_probe(...) を呼んでいること
    - 対象固有のダミー関数（gdb_dump_*）が残っていないこと（ハードコード排除）
    """
    main_cpp = os.path.join(root, "src", "main.cpp")
    sh = os.path.join(root, "lib", "gdb_debug.sh")
    probe_h = os.path.join(root, "lib", "gdb_probe.hpp")
    if not (os.path.isfile(main_cpp) and os.path.isfile(sh)):
        err("probe: 必須ファイル不足のため整合チェック不可")
        return

    main_text = read(main_cpp)
    sh_text = read(sh)

    # 1. gdb_probe テンプレート定義
    if os.path.isfile(probe_h):
        pt = read(probe_h)
        if re.search(r"template\s*<[^>]*>", pt) and re.search(
            r"\bgdb_probe\s*\(", pt
        ):
            ok("probe: lib/gdb_probe.hpp に gdb_probe テンプレート定義あり")
        else:
            err("probe: lib/gdb_probe.hpp に gdb_probe テンプレートが無い")
        if "noinline" in pt:
            ok("probe: gdb_probe に noinline 指定あり")
        else:
            warn("probe: gdb_probe に noinline 指定が無い（要目視）")
    else:
        err("probe: lib/gdb_probe.hpp が無い")

    # 2. 生成 .gdb の抽出と break 対象の確認
    gdb_text = extract_generated_gdb(sh_text)
    if gdb_text is None:
        err("probe: gdb_debug.sh から生成 .gdb（GDBEOF）を抽出できない")
        return
    ok("probe: gdb_debug.sh の生成 .gdb を抽出できた")

    broken = set(re.findall(r"^\s*break\s+(\S+)", gdb_text, re.MULTILINE))
    if broken == {"gdb_probe"}:
        ok("probe: 生成 .gdb の break 対象は gdb_probe のみ（汎用形）")
    else:
        err(
            "probe: 生成 .gdb の break 対象が gdb_probe 以外を含む -> {}".format(
                sorted(broken)
            )
        )

    # 3. 生成 .gdb が value を p するだけで、メンバ名を列挙していないこと
    if re.search(r"^\s*p\s+value\b", gdb_text, re.MULTILINE):
        ok("probe: 生成 .gdb は p value でメンバ非列挙（ハードコード排除）")
    else:
        err("probe: 生成 .gdb に p value が無い")
    if re.search(r"out\[\$i\]\.", gdb_text):
        err("probe: 生成 .gdb に対象固有メンバ列挙が残存（out[$i].xxx）")
    else:
        ok("probe: 生成 .gdb に対象固有メンバ列挙なし")

    # 4. main.cpp が gdb_probe を呼んでいる
    if re.search(r"\bgdb_probe\s*\(", main_text):
        ok("probe: main.cpp は gdb_probe(...) を使用")
    else:
        err("probe: main.cpp が gdb_probe(...) を使っていない")

    # 5. 対象固有ダミー関数が残っていない（ハードコード排除の確認）
    if re.search(r"\bgdb_dump_\w+\s*\(", main_text):
        err("probe: main.cpp に旧ダミー関数 gdb_dump_* が残存")
    else:
        ok("probe: main.cpp に旧ダミー関数 gdb_dump_* なし")


# ---- 6. Makefile / gdb_debug.sh のパス整合 --------------------
def check_paths(root):
    mk = os.path.join(root, "lib", "Makefile")
    sh = os.path.join(root, "lib", "gdb_debug.sh")
    if os.path.isfile(mk):
        t = read(mk)
        checks = {
            r"SRCDIR\s*=\s*\.\./src": "Makefile: SRCDIR=../src",
            r"GENDIR\s*=\s*\.\./generated": "Makefile: GENDIR=../generated",
            r"(?m)^\s*@?rm\s+-rf\s+\$\(GENDIR\)": "Makefile: build時に generated を削除",
            r"-I\s*\$\(LIBDIR\).*-I\s*\$\(SRCDIR\)": "Makefile: include に lib/src",
        }
        for pat, desc in checks.items():
            if re.search(pat, t):
                ok(desc)
            else:
                warn("{} が確認できない（要目視）".format(desc))
    if os.path.isfile(sh):
        t = read(sh)
        if re.search(r'GENDIR="\.\./generated"', t):
            ok("gdb_debug.sh: GENDIR=../generated")
        else:
            warn("gdb_debug.sh: GENDIR の指定が確認できない（要目視）")
        if re.search(r"make\b", t):
            ok("gdb_debug.sh: make を呼んでいる")
        else:
            err("gdb_debug.sh: make 呼び出しが無い")


# ---- 7. 宣言と定義の対応（簡易） ------------------------------
def check_decl_def(root):
    hpp = os.path.join(root, "src", "childResultTest.hpp")
    cpp = os.path.join(root, "src", "childResultTest.cpp")
    if not (os.path.isfile(hpp) and os.path.isfile(cpp)):
        return
    hpp_text = read(hpp)
    cpp_text = read(cpp)
    # hpp の関数宣言名（ざっくり: 名前( を拾い、struct/enum等を除外）
    decl_names = set(re.findall(r"\b([a-z]\w+)\s*\([^;{]*\)\s*;", hpp_text))
    # 定義側に現れる関数名（名前( を拾う）
    def_text = cpp_text
    for name in sorted(decl_names):
        # 定義は "name(" が cpp に現れることを最低条件とする
        if re.search(r"\b" + re.escape(name) + r"\s*\(", def_text):
            ok("decl/def: {} は cpp に定義あり".format(name))
        else:
            err("decl/def: {} の定義が cpp に見当たらない".format(name))


def main():
    if len(sys.argv) >= 2:
        root = os.path.abspath(sys.argv[1])
    else:
        # スクリプトは libGdb/tools/ にある想定 -> 親が libGdb
        here = os.path.dirname(os.path.abspath(__file__))
        root = os.path.dirname(here)

    if not os.path.isdir(root):
        print("ERROR: libGdb ルートが見つからない: {}".format(root))
        return 2

    print("=== libGdb static check ===")
    print("root: {}".format(root))
    print()

    check_layout(root)
    check_includes(root)
    check_no_business_dep(root)
    check_probe_consistency(root)
    check_paths(root)
    check_decl_def(root)

    print("--- OK ({}) ---".format(len(OKS)))
    for m in OKS:
        print("  [OK]   " + m)
    if WARNINGS:
        print("--- WARN ({}) ---".format(len(WARNINGS)))
        for m in WARNINGS:
            print("  [WARN] " + m)
    if ERRORS:
        print("--- ERROR ({}) ---".format(len(ERRORS)))
        for m in ERRORS:
            print("  [ERR]  " + m)

    print()
    if ERRORS:
        print("RESULT: NG (errors={}, warnings={})".format(len(ERRORS), len(WARNINGS)))
        return 1
    print("RESULT: OK (errors=0, warnings={})".format(len(WARNINGS)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
