# ファイル名     : Makefile
# 概要           : libConf サンプル（confmng）のビルド
#                  実行ファイル confmng と obj/ を libConf/ 直下に出力する
# 共通設定
CXX      = g++
CXXFLAGS = -g3 -std=c++11 -Wall -Wextra -Wconversion

OBJDIR   = obj
TARGET   = confmng
SOURCES  = $(wildcard *.cpp) \
           $(wildcard libConf/*.cpp)
OBJECTS  = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))
HDRS     = $(wildcard *.h) $(wildcard libConf/*.hpp)

all: init $(TARGET)

init:
	@mkdir -p $(dir $(OBJECTS))

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

$(OBJDIR)/%.o: %.cpp $(HDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR) $(TARGET)

.PHONY: all init clean

##################################### EOF ######################################
