#include "tokenizer.hpp"

#include <sstream>
#include <regex>
#include <iomanip>
#include <iostream>
#include <cctype>

namespace util {

// 文字列分割
std::vector<std::string> Tokenizer::splitWords(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    if (!s.empty()) {
        tokens.reserve(s.length() / 2);
    }
    for (std::string t; iss >> t; ) {
        tokens.push_back(t);
    }
    return tokens;
}

// 整数判定
// NOTE: static regex の利用に関するスレッドセーフの注意はヘッダを参照
bool Tokenizer::isIntegerToken(const std::string& s) {
    if (s.empty()) return false;
    static const std::regex re(R"(^[+-]?\d+$)");
    return std::regex_match(s, re);
}

// 浮動小数点判定
// NOTE: static regex の利用に関するスレッドセーフの注意はヘッダを参照
bool Tokenizer::isFloatingToken(const std::string& s) {
    if (s.empty()) return false;
    static const std::regex re(
        R"(^[+-]?(?:\d+\.\d*|\.\d+|\d+(?:\.\d*)?)(?:[eE][+-]?\d+)?$)"
    );
    return std::regex_match(s, re);
}

// UTF-8 バリデーション（インデックスベース）
bool Tokenizer::isValidString(const std::string& s) {
    size_t len = s.length();
    size_t i = 0;

    while (i < len) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        // 制御文字チェック (ASCII 0x00-0x1F, 0x7F)
        if (std::iscntrl(static_cast<int>(c))) {
            if (c != '\t' && c != '\n' && c != '\r') {
                return false;
            }
        }

        // ASCII (1バイト文字)
        if (c < 0x80) {
            i++;
            continue;
        }

        // マルチバイト文字のバイト数を特定
        int bytesExpected = 0;
        if ((c & 0xE0) == 0xC0) {
            bytesExpected = 2;
        } else if ((c & 0xF0) == 0xE0) {
            bytesExpected = 3;
        } else if ((c & 0xF8) == 0xF0) {
            bytesExpected = 4;
        } else {
            return false;
        }

        if (i + static_cast<size_t>(bytesExpected) > len) {
            return false;
        }

        for (int j = 1; j < bytesExpected; ++j) {
            unsigned char nextC = static_cast<unsigned char>(s[i + static_cast<size_t>(j)]);
            if ((nextC & 0xC0) != 0x80) {
                return false;
            }
        }

        i += static_cast<size_t>(bytesExpected);
    }

    return true;
}

// フィルタリング処理
std::vector<std::string> Tokenizer::filterValidTokens(const std::vector<std::string>& tokens) {
    std::vector<std::string> cleanTokens;
    cleanTokens.reserve(tokens.size());

    for (const auto& tk : tokens) {
        if (isValidString(tk)) {
            cleanTokens.push_back(tk);
        }
    }
    return cleanTokens;
}

// 解析処理
Tokenizer::ParsedVectors Tokenizer::parseTokens(const std::vector<std::string>& tokens) {
    ParsedVectors out;

    for (const auto& tk : tokens) {
        if (isIntegerToken(tk)) {
            try {
                long long v = std::stoll(tk);
                out.ints.push_back(v);
                continue;
            } catch (...) {
                try {
                    double d = std::stod(tk);
                    out.doubles.push_back(d);
                    continue;
                } catch (...) {}
            }
        }

        if (isFloatingToken(tk)) {
            try {
                double d = std::stod(tk);
                out.doubles.push_back(d);
                continue;
            } catch (...) {}
        }

        out.strings.push_back(tk);
    }
    return out;
}

// doubleの正規化
std::string Tokenizer::normalizeDoubleToString(double value) {
    std::ostringstream oss;
    oss << std::setprecision(17) << std::fixed << value;
    std::string s = oss.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }
    if (s == "-0") s = "0";
    return s;
}

// 文字列ベクタへの変換
std::vector<std::string> Tokenizer::parseToStrVector(
    const std::vector<std::string>& tokens,
    TagStyle tagStyle
) {
    std::vector<std::string> out;
    out.reserve(tokens.size());

    for (const auto& tk : tokens) {
        if (isIntegerToken(tk)) {
            try {
                long long v = std::stoll(tk);
                std::string val = std::to_string(v);
                out.push_back(tagStyle == TagStyle::Prefix ? "INT:" + val : val);
                continue;
            } catch (...) {
                try {
                    double d = std::stod(tk);
                    std::string val = normalizeDoubleToString(d);
                    out.push_back(tagStyle == TagStyle::Prefix ? "FLOAT:" + val : val);
                    continue;
                } catch (...) {}
            }
        }

        if (isFloatingToken(tk)) {
            try {
                double d = std::stod(tk);
                std::string val = normalizeDoubleToString(d);
                out.push_back(tagStyle == TagStyle::Prefix ? "FLOAT:" + val : val);
                continue;
            } catch (...) {}
        }

        out.push_back(tagStyle == TagStyle::Prefix ? "STR:" + tk : tk);
    }
    return out;
}

// 結果表示（iostream に統一）
void Tokenizer::printParsed(const ParsedVectors& pv) {
    std::cout << "[ints   ] size=" << pv.ints.size() << " : ";
    for (auto v : pv.ints) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    std::cout << "[doubles] size=" << pv.doubles.size() << " : ";
    for (auto v : pv.doubles) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    std::cout << "[strings] size=" << pv.strings.size() << " : ";
    for (const auto& v : pv.strings) {
        std::cout << v << " ";
    }
    std::cout << "\n";
}

} // namespace util
