# ファイル名     : Makefile
# 概要           : libGdb sample（子プロセス計算結果の読み込み）のビルド定義
#
# 構成は sclComBatTest / sclWebApiTest と同じフラット形式。
#   libGdb/
#   ├── main.cpp / sampleTarget.*      … sample のドライバと試験対象
#   ├── run_tests.sh                   … ケース表を1ケースずつ実行し 良/否 を判定
#   ├── cases/                         … ケース表
#   ├── lib/gdb_probe.hpp              … 共有資産の正本（各テストへコピーして使う）
#   └── generated/                     … 実行ファイル・証跡・結果（make clean で削除）
#
# 本 sample は業務ライブラリに依存しない（標準ライブラリのみ）。
# 実ライブラリを使うテストは sclComBatTest の Makefile を雛形にする。
#
# 例:
#   make          # generated/libGdbSample を作る
#   make clean    # generated/ を削除（証跡も消える）

CXX = g++

# 観測点関数を確実に残すため -O0（-finline-functions は付けない）
CXXFLAGS = -g3 -O0 -std=c++11 -Wall -Wextra -Wconversion -I . -I lib

GENDIR = generated
OBJDIR = $(GENDIR)/obj
TARGET = $(GENDIR)/libGdbSample

SRCS = main.cpp sampleTarget.cpp
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
HDRS = $(wildcard *.hpp) lib/gdb_probe.hpp

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(GENDIR)
	$(CXX) -o $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp $(HDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成物（実行ファイル・証跡・結果）を削除する
clean:
	@rm -rf $(GENDIR)

.PHONY: all clean

##################################### EOF ######################################
