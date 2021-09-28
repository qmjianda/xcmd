OBJ += xcmd.o xcmd_default_keys.o xcmd_default_cmds.o linux_main.o test.o
BIN = xcmder
VPATH := ../../src \
		 ../../extensions/test

INC += -I../../inc \
	   -I../../extensions/test

OBJ_WITH_BUILD_DIR:=$(addprefix build/,$(OBJ))

all: mkbuilddir $(OBJ_WITH_BUILD_DIR)
	gcc $(OBJ_WITH_BUILD_DIR) -o $(BIN) -Wl,-Map,$(BIN).map

build/%.o:%.c
	gcc -g -Wall -c $(INC) -o $@ $<

.PHONY:mkbuilddir
mkbuilddir:
	mkdir -p build

.PHONY:clean 
clean:
	rm -rf build xcmder


