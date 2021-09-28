OBJ += xcmd.o xcmd_default_keys.o xcmd_default_cmds.o linux_main.o test.o ex_keys.o ex_cmds.o
BIN = xcmder
VPATH := ../../src \
		 ../../extensions/test \
		 ../../extensions/ex_keys \
		 ../../extensions/ex_cmds

INC += -I../../inc \
	   -I../../extensions/test \
	   -I../../extensions/ex_keys \
	   -I../../extensions/ex_cmds

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


