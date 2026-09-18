#ifndef LIBGDB_GDB_PROBE_HPP
#define LIBGDB_GDB_PROBE_HPP

// ============================================================
// libGdb 共通: GDB 観測点マーカー（型非依存・対象非依存の固定資産）
//
// 観測対象を「値」としてこのマーカーに渡す。テンプレートなので型は
// 自動推論され、対象が変わってもマーカーは書き直さなくてよい。
// 観測変数がマーカー自身のフレームに存在するため、GDB 側で up は不要。
//
// これにより、生成する .gdb は対象に依存せず
//     break gdb_probe
//     p value
// だけで済む（メンバ名の列挙が不要 = ハードコード排除）。
//
// 使い方（src/ 側・可変）:
//     std::vector<Foo> out;
//     readSomething(key, out);
//     gdb_probe("out_dump", out);   // ここで停止。value=out が見える
//
// 生成される .gdb（lib/gdb_debug.sh が出力・対象非依存）:
//     break gdb_probe
//     commands
//       silent
//       printf "\n[GDB] === probe: %s ===\n", label
//       p value          // value を丸ごと GDB のネイティブ表示で出力
//       continue
//     end
//
// 注意:
//   - テンプレートはインスタンス化されて初めて実体が出る。各対象で実際に
//     gdb_probe(...) を呼べば実体が生成され、noinline で関数として残る。
//   - break gdb_probe は各インスタンスへ pending で一括適用できる
//     （set breakpoint pending on を .gdb 側で有効化済み）。
// ============================================================

template <typename T>
__attribute__((noinline)) void gdb_probe(const char *label, const T &value)
{
    // label / value を最適化で消さないため volatile 経由で参照する。
    // これでマーカー自身のフレームに value が確実に存在し、p value できる。
    volatile const char *l = label;
    volatile const void *v = static_cast<const void *>(&value);
    (void)l;
    (void)v;
}

#endif // LIBGDB_GDB_PROBE_HPP
