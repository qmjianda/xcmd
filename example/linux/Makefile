BIN = xcmder

OBJ += xcmd.o \
	   xcmd_default_keys.o \
	   xcmd_default_cmds.o \
	   linux_main.o \
	   test.o \
	   ex_keys.o \
	   ex_list.o \
	   ex_cmds.o \
	   socket_cmds.o \
	   diskio.o \
	   ff.o \
	   ffunicode.o \
	   fatfs_port.o \
	   fs_cmds.o\
	   fatfs_disk_mmc.o\
	   fatfs_disk_ram.o

VPATH := ../../src \
		 ../../extensions/test \
		 ../../extensions/ex_keys \
		 ../../extensions/ex_list \
		 ../../extensions/ex_cmds \
		 ../../extensions/net_cmds \
		 ../../extensions/fs_cmds \
		 ../../extensions/fs_cmds/FatFs/source \

INC += -I./ \
	   -I../../inc \
	   -I../../extensions/test \
	   -I../../extensions/ex_keys \
	   -I../../extensions/ex_list \
	   -I../../extensions/ex_cmds \
	   -I../../extensions/net_cmds \
	   -I../../extensions/fs_cmds/FatFs/source \
	   -I../../extensions/fs_cmds \

OBJ_WITH_BUILD_DIR:=$(addprefix build/,$(OBJ))

all: mkbuilddir $(OBJ_WITH_BUILD_DIR)
	gcc $(OBJ_WITH_BUILD_DIR) -o $(BIN) -Wl,-Map,$(BIN).map

build/%.o:%.c
	gcc -g -O0 -Wall -c $(INC) -o $@ $<

.PHONY:mkbuilddir
mkbuilddir:
	mkdir -p build

.PHONY:clean 
clean:
	rm -rf build xcmder fatfs.img xcmder.map


