BIN = xcmder

OBJ += xcmd.o \
	   xcmd_keys.o \
	   xcmd_cmds.o \
	   linux_main.o \
	   xnr_io.o \
	   xnr_line.o \
	   xnr_history.o \
	   xnr_key.o \
	   xnr_var.o \
	   linux_cmds.o \
	   linux_keys.o \
	   test.o \
	   ex_keys.o \
	   ex_list.o \
	   ex_cmds.o \
	   socket_cmds.o \

VPATH := ../../src \
		 ../../src/xnr \
		 ../../extensions/test \
		 ../../extensions/ex_keys \
		 ../../extensions/ex_list \
		 ../../extensions/ex_cmds \
		 ../../extensions/net_cmds \

INC += -I./ \
	   -I../../src \
	   -I../../src/xnr \
	   -I../../third_party/XNanoReadLine \
	   -I../../extensions/test \
	   -I../../extensions/ex_keys \
	   -I../../extensions/ex_list \
	   -I../../extensions/ex_cmds \
	   -I../../extensions/net_cmds \

OBJ_WITH_BUILD_DIR:=$(addprefix build/,$(OBJ))

all: mkbuilddir $(OBJ_WITH_BUILD_DIR)
	gcc $(OBJ_WITH_BUILD_DIR) -o $(BIN) -Wl,-Map,$(BIN).map

build/%.o:%.c
	gcc -g -O0 -Wall -c $(INC) -o $@ $<

.PHONY:mkbuilddir
mkbuilddir:
	mkdir -p build

.PHONY:clean distclean
clean:
	rm -rf build xcmder xcmder.map

distclean:
	rm -rf build xcmder fatfs.img xcmder.map


