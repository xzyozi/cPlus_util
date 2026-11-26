# ファイル名 : Makefile
# 概要       : CUI メニューライブラリ ビルド設定

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wconversion -g
LDFLAGS  =

TARGET   = menuTest
SOURCES  = $(wildcard *.cpp) $(wildcard libMenu/*.cpp)
OBJDIR   = obj
OBJECTS  = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))

all: init $(TARGET)

init:
	@mkdir -p $(dir $(OBJECTS))

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all init clean
