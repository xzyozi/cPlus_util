# ファイル名     : Makefile
# 概要           : libArgs（コマンドライン引数解析ライブラリ）sample のビルド定義
#
#   libArgs/
#   ├── main.cpp                 … デモ兼テストドライバ
#   ├── lib/args.hpp, args.cpp   … ライブラリ本体
#   ├── run_tests.sh             … ケース表を1ケースずつ実行し 良/否 を判定
#   ├── cases/                   … ケース表
#   └── generated/               … 実行ファイル・証跡・結果（make clean で削除）
#
# 例:
#   make          # generated/libArgsSample を作る
#   make clean    # generated/ を削除（証跡も消える）

CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -Wconversion -g -I lib

GENDIR = generated
OBJDIR = $(GENDIR)/obj
TARGET = $(GENDIR)/libArgsSample

SRCS = main.cpp lib/args.cpp
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
HDRS = lib/args.hpp

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
