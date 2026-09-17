# ファイル名     : Makefile
# システム名     : 計算実行基盤
# サブシステム名 : cgt0130 混雑特定::短期・直前任意地点指定計算（親）
# 概要           : Makefile（子プロセス計算結果 読み込み検証テスト）
# 作成日         : 
# 作成者         : 
# バージョン     : 1.0
# 修正履歴       : 

# 共通設定
DEVHOME = ../../../..
APLHOME = ../../..
INCLUDE	=	\
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

CC	= g++

CXXFLAGS	= -fpic -g3 -finline-functions -Wall -Wconversion $(INCLUDE)

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

LDFLAGS	=

# 担当者別設定
INCDIR			= $(APLHOME)/include
INCDIR_DAO_DTO	= $(APLHOME)/cmn/include/cmn0030DbTable
INCDIR_FF		= $(DEVHOME)/usr/include/mcl/FILE
SRCDIR			= .
OBJDIR			= $(SRCDIR)/obj
BINDIR			= .

TARGET			= ./testChildPrcKnr

SOURCES			= $(wildcard *.cpp)
OBJECTS			= $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))

all:    init $(TARGET)

init:
	@mkdir -p $(dir $(OBJECTS))

$(TARGET):$(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o:	%.cpp $(HEADERS)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJECTS) $(TARGET)

##################################### EOF ######################################
