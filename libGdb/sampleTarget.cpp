#include "sampleTarget.hpp"

#include <cstring>
#include <string>
#include <vector>

namespace SampleTarget {
namespace {

// インメモリのデータストア
std::vector<Item> g_store;

// 固定長 char 配列へ安全にコピーする（末尾 NUL 保証）
template <std::size_t N> void setField(char (&dst)[N], const std::string &src) {
  std::memset(dst, 0, N);
  const std::size_t n = src.size() < (N - 1) ? src.size() : (N - 1);
  std::memcpy(dst, src.c_str(), n);
}

} // namespace

Item makeItem(int id, int grp, const std::string &code,
              const std::string &name) {
  Item item{};
  item.id = id;
  item.grp = grp;
  setField(item.code, code);
  setField(item.name, name);
  return item;
}

void clearStore() { g_store.clear(); }

void addItem(const Item &item) { g_store.push_back(item); }

int findItems(const Query &query, std::vector<Item> &out) {
  out.clear();
  if (query.grp < 0 || query.code.empty()) {
    return RETURN_NG;
  }
  for (const auto &item : g_store) {
    if (item.grp == query.grp && query.code == item.code) {
      out.push_back(item);
    }
  }
  return RETURN_OK;
}

void setupOne() { addItem(makeItem(1, 1, "A01", "one")); }

void setupMany() {
  addItem(makeItem(1, 1, "A01", "first"));
  addItem(makeItem(2, 1, "A01", "second"));
  addItem(makeItem(3, 1, "A01", "third"));
}

void setupMixed() {
  addItem(makeItem(1, 1, "A01", "match"));      // 一致
  addItem(makeItem(2, 2, "A01", "other grp"));  // grp 違い
  addItem(makeItem(3, 1, "B01", "other code")); // code 違い
}

} // namespace SampleTarget
