#ifndef UTIL_TOKENIZER_HPP
#define UTIL_TOKENIZER_HPP

#include <string>
#include <vector>

namespace util {

/// 文字列トークン解析ユーティリティ。
/// インスタンス生成禁止（静的クラスとして運用）。
///
/// NOTE: isIntegerToken / isFloatingToken 内の static std::regex は
/// C++11 以降の仕様により初期化自体はスレッドセーフだが、
/// 一部の libstdc++ 実装では std::regex_match がスレッドセーフでない場合がある。
/// マルチスレッドで使用する場合はミューテックスで保護するか、
/// regex を thread_local にすることを検討すること。
class Tokenizer {
public:
    Tokenizer() = delete;

    enum class TagStyle {
        None,   // 値のみ
        Prefix  // INT:xxx, FLOAT:xxx 等の接頭辞
    };

    struct ParsedVectors {
        std::vector<long long> ints;
        std::vector<double> doubles;
        std::vector<std::string> strings;
    };

    // --- 判定・フィルタリング群 ---

    static std::vector<std::string> splitWords(const std::string& s);
    static bool isIntegerToken(const std::string& s);
    static bool isFloatingToken(const std::string& s);
    static bool isValidString(const std::string& s);
    static std::vector<std::string> filterValidTokens(const std::vector<std::string>& tokens);

    // --- 解析・変換群 ---

    static ParsedVectors parseTokens(const std::vector<std::string>& tokens);
    static std::string normalizeDoubleToString(double value);
    static std::vector<std::string> parseToStrVector(
        const std::vector<std::string>& tokens,
        TagStyle tagStyle = TagStyle::None
    );
    static void printParsed(const ParsedVectors& pv);
};

} // namespace util

#endif // UTIL_TOKENIZER_HPP
