#ifndef SAMPLE_TARGET_HPP
#define SAMPLE_TARGET_HPP

#include <string>
#include <vector>

// ============================================================
// libGdb sample の試験対象。
// 業務コード・DB に依存しない、インメモリの検索処理。
//
// 実際のテストでは、このファイルの位置に「実ライブラリの関数宣言」
// （例: sclComBatTest の SclMakeKonzatsuNo）が入る。
// GDB の表示を見せるため、次の3種類を持つ。
//   - 構造体        : Query / Item
//   - 固定長 char 配列: Item::code / Item::name
//   - vector        : findItems の出力
// ============================================================
namespace SampleTarget {

// 戻り値
enum { RETURN_OK = 0, RETURN_NG = 1 };

// 登録するデータ（固定長 char 配列は GDB で文字列として表示される）
struct Item {
  int id;        // 区別用の番号（非キー）
  int grp;       // グループ（キー）
  char code[8];  // コード（キー）
  char name[16]; // 名称（非キー）
};

// 検索条件
struct Query {
  int grp;          // 0 以上であること
  std::string code; // 空でないこと
};

// 1件分のデータを作る
Item makeItem(int id, int grp, const std::string &code,
              const std::string &name);

// ストアを空にする
void clearStore();

// ストアへ1件登録する
void addItem(const Item &item);

// 条件（grp と code が両方一致）に合うデータを out へ返す。
// 条件が不正（grp < 0 または code が空）のときは RETURN_NG を返し、out は空にする。
int findItems(const Query &query, std::vector<Item> &out);

// ケース用のデータセットアップ
void setupOne();   // 一致1件
void setupMany();  // 一致3件
void setupMixed(); // 一致1件 + grp 違い + code 違い

} // namespace SampleTarget

#endif // SAMPLE_TARGET_HPP
