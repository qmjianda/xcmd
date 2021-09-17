OBJ += xcmd.o xcmd_default_keys.o xcmd_default_cmds.o test_cmds.o  linux_main.o 
BIN = xcmder
VPATH := ../../src
INC += ../../inc
OBJ_WITH_BUILD_DIR:=$(addprefix build/,$(OBJ))

all: mkbuilddir $(OBJ_WITH_BUILD_DIR)
	gcc $(OBJ_WITH_BUILD_DIR) -o $(BIN) -Wl,-Map,$(BIN).map

build/%.o:%.c
	gcc -g -Wall -c -I$(INC) -o $@ $<

.PHONY:mkbuilddir
mkbuilddir:
	mkdir -p build

.PHONY:clean 
clean:
	rm -rf build xcmder


