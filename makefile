# 使用 g++ 做為編譯器與連結器 
CXX := g++
CC  := g++
LD  := g++

# 使用者設定的 SystemC 路徑，從環境變數取得
SYSTEMC_HOME ?= /your/systemc/path

# 預設編譯模式：debug 或 release（可用 make MODE=release 指定）
MODE ?= debug

# 編譯模式判斷
ifeq ($(MODE),debug)
    BUILD_TYPE := Debug
    COMMON_FLAGS := -Wall -g -O0 -DDEBUG
else ifeq ($(MODE),release)
    BUILD_TYPE := Release
    COMMON_FLAGS := -Wall -O2 -DNDEBUG
else
    $(error ❌ MODE 只能是 debug 或 release，目前是 "$(MODE)")
endif

# 編譯參數
CXXFLAGS := $(COMMON_FLAGS) -std=c++17 -MMD -MP -I$(SYSTEMC_HOME)/include -Iinclude
CFLAGS   := $(COMMON_FLAGS) -std=c++17 -MMD -MP -I$(SYSTEMC_HOME)/include -Iinclude
LDFLAGS  := -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc -lm

# 資料夾路徑
SRCDIR   := source
INCDIR   := include
BUILDDIR := build
TARGET   := main

# 尋找所有 .cpp 和 .c 檔案（支援子資料夾）
CPPSOURCES := $(shell find $(SRCDIR) -name "*.cpp")
CSOURCES   := $(shell find $(SRCDIR) -name "*.c")
SOURCES    := $(CPPSOURCES) $(CSOURCES)

# 對應到 .o 檔的輸出路徑
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(CPPSOURCES)) \
           $(patsubst $(SRCDIR)/%.c,   $(BUILDDIR)/%.o, $(CSOURCES))

# 自動依賴檔（.d）
DEPS := $(OBJECTS:.o=.d)

# 預設目標
all: info $(TARGET)

# 顯示目前編譯模式
info:
	@echo "🔧 Build mode: $(BUILD_TYPE)"

# 連結執行檔
$(TARGET): $(OBJECTS)
	@printf "🔗 Linking $@\n    $(LD) $^ $(LDFLAGS) -o $@"
	@$(LD) $^ $(LDFLAGS) -o $@

# 編譯 .cpp → .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "🛠️  Compiling C++: $<\n    $(CXX) $(CXXFLAGS) -c $< -o $@\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# 編譯 .c → .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@printf "🛠️  Compiling C  : $<\n    $(CC) $(CFLAGS) -c $< -o $@\n"
	@$(CC) $(CFLAGS) -c $< -o $@

# 載入 .d 檔來追蹤 header 相依性
-include $(DEPS)

# 清除
clean:
	@rm -rf $(BUILDDIR) $(TARGET)
	@echo "🧹 Cleaned all build files."

.PHONY: all clean info
