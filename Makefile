CXX  = x86_64-w64-mingw32-g++
MKDIR = mkdir -p

BIN := bin/

DEP_DEST := $(BIN)dep/
OBJ_DEST := $(BIN)obj/

CXXFILES  := $(shell find . -type f -name '*.cpp')
HXXFILES  := $(shell find . -type f -name '*.hpp')

OBJS := $(patsubst %.o, $(OBJ_DEST)%.o, $(CXXFILES:.cpp=.cpp.o))

INCLUDES := -I. -Iinclude/

CXXFLAGS :=    \
	-O2 	   \
	-pipe	   \
	-std=c++17 \
	-lgcc 	   \
	-lws2_32   \
	-static -static-libgcc -static-libstdc++ \
	$(INCLUDES)

format:
	@$(MKDIR) $(ISO) $(SYS)
	clang-format -style=file -i ${CXXFILES} ${HXXFILES}

all: $(OBJS)
	$(CXX) $(OBJS) -o $(BIN)dhcpbong $(CXXFLAGS) 

include $(shell find $(DEP_DEST) -type f -name *.d)

clean:
	rm -rf $(DEP_DEST)
	rm -rf $(OBJ_DEST)

$(OBJ_DEST)%.cpp.o : %.cpp
	@$(MKDIR) ${@D}
	@$(MKDIR) $(dir $(DEP_DEST)$*)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MT $@ -MF $(DEP_DEST)$*.cpp.d