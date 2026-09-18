# ファイル名     : Makefile
# 概要           : libGdb エビデンス採取テスト用 Makefile
#
# ディレクトリ構成（このファイルは lib/ にある）:
#   libGdb/
#   ├── lib/        … ライブラリ本体（このファイル・header.h・gdb_debug.sh）
#   ├── src/        … 可変ファイル（sample: テスト対象を書く。*.cpp / *.hpp）
#   └── generated/  … 自動生成物（obj・実行ファイル・.gdb）。make で毎回作り直す
#
# ============================================================
# ビルドモード切り替え
#   USE_LIBS=0 (既定) : スタンドアローン。業務 INCLUDE/LIBS に依存せず、
#                       標準ライブラリのみでビルドする。
#   USE_LIBS=1        : 従来どおり業務 INCLUDE/LIBS をリンクする。
#
#   例:
#     make                # スタンドアローン（USE_LIBS=0）
#     make USE_LIBS=1      # 業務ライブラリを使用
#
# 生成物はビルド開始時に generated/ ごと削除して作り直す。
# ============================================================
USE_LIBS ?= 0

CC	= g++

# --- ディレクトリ（lib/ からの相対） ---
LIBDIR		= .
SRCDIR		= ../src
GENDIR		= ../generated
OBJDIR		= $(GENDIR)/obj

# 業務ライブラリ使用時のパス基準（従来どおり。lib/ からの相対に補正）
DEVHOME = ../../../../..
APLHOME = ../../../..

ifeq ($(USE_LIBS),1)
# ---------- 業務ライブラリ使用モード ----------
INCLUDE	=	\
		-I $(LIBDIR) \
		-I $(SRCDIR) \
 		-I /usr/pgsql-11/include \
		-I /opt/mel/modern/include \
		-I /opt/mel/modern/include/FILE \
		-I /usr/include/mcl  \
		-I /usr/include/mcl/kts \
		-I /usr/include/mcl/prc \
		-I $(DEVHOME)/usr/include/mcl \
		-I $(DEVHOME)/usr/include/mcl/opf \
		-I $(DEVHOME)/usr/include/mcl/kcl/Api \
		-I $(DEVHOME)/usr/include/mcl/kcl/FILE \
		-I $(DEVHOME)/usr/include/mcl/FILE \
		-I $(APLHOME)/cgt/include \
		-I $(APLHOME)/cmn/include \
		-I $(APLHOME)/cmn/include/cmn0030DbTable \
		-I $(APLHOME)/tpl/include \
		-I $(APLHOME)/cdh/include/cdh \

LIBS    = \
        $(APLHOME)/cgt/lib/libCgt0920.a \
        $(APLHOME)/cgt/lib/libCgt0910.a \
        $(APLHOME)/cgt/lib/libCgt0900.a \
        $(APLHOME)/cgt/lib/libCgt4900.a \
        $(APLHOME)/cmn/lib/libCmn0040.a \
        $(APLHOME)/tpl/lib/libTpl0020.a \
        $(APLHOME)/tpl/lib/libTpl0060.a \
        $(APLHOME)/tpl/lib/libTpl0040.a \
        $(APLHOME)/tpl/lib/libTpl0030.a \
        $(APLHOME)/cmn/lib/libCmn0030.a \
        $(APLHOME)/cmn/lib/libCmn0020.a \
        $(APLHOME)/cmn/lib/libCmn0030.a \
        $(APLHOME)/cmn/lib/libCmn0020.a \
        $(APLHOME)/cmn/lib/libCmn0010.a \
        /usr/lib/mcl/prc/libPrc.a \
        /usr/lib/mcl/ktp/libKTP.a \
        /usr/lib/mcl/Sdm/libSdm.a \
        /usr/lib/mcl/libMcl.a \
        /opt/mel/modern/lib64/libscn.so \
        /opt/mel/modern/lib64/libscn_W.so \
        /opt/mel/modern/lib64/libkihon.so \
        /opt/mel/modern/lib64/libets.a \
        /usr/pgsql-11/lib/libpq.so \

# 業務ライブラリ使用時のコンパイルフラグ（従来どおり）
CXXFLAGS	= -fpic -g3 -finline-functions -Wall -Wconversion $(INCLUDE)

else
# ---------- スタンドアローンモード（既定・依存なし） ----------
# 業務 INCLUDE/LIBS を一切使わない。標準ライブラリと自前ヘッダのみ。
INCLUDE	= -I $(LIBDIR) -I $(SRCDIR)
LIBS    =

# 観測点関数を確実に残すため -O0（-finline-functions は付けない）
CXXFLAGS	= -g3 -O0 -std=c++11 -Wall -Wextra -Wconversion $(INCLUDE)

endif

LDFLAGS	=

# 実行ファイルは generated/ 配下に置く
TARGET		= $(GENDIR)/testChildPrcKnr

# ソースは src/ から収集
SOURCES		= $(wildcard $(SRCDIR)/*.cpp)
OBJECTS		= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))

# ヘッダ依存（lib/ と src/）
HEADERS		= $(wildcard $(LIBDIR)/*.h) $(wildcard $(SRCDIR)/*.hpp)

# 生成物はビルド開始時に作り直す（generated/ を削除してから）
all:    regen $(TARGET)

# generated/ を丸ごと削除してから作り直す
regen:
	@rm -rf $(GENDIR)
	@mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

# 生成物のみ削除
clean:
	@rm -rf $(GENDIR)

.PHONY: all regen clean

##################################### EOF ######################################
