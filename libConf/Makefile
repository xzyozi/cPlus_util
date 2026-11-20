# ファイル名     : Makefile
# 概要           : libConf サンプル（confmng）のビルド
# 共通設定
INCLUDE	=
CC	= g++
CXXFLAGS	= -fpic -g3 -finline-functions -Wall -Wconversion $(INCLUDE)
LIBS	=
LDFLAGS	=
# 担当者別設定
SRCDIR			= .
OBJDIR			= $(SRCDIR)/obj
BINDIR			= .
TARGET			= ./confmng
SOURCES			= $(wildcard *.cpp) \
                  $(wildcard libConf/*.cpp)
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
