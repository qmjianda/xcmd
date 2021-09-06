include example/Makefile
INC += ./
OBJ += xcmd.o
BIN = xcmder
VPATH := example
OBJ_WITH_BUILD_DIR:=$(addprefix build/,$(OBJ))

all: mkbuilddir $(OBJ_WITH_BUILD_DIR)
	gcc $(OBJ_WITH_BUILD_DIR) -o $(BIN)

build/%.o:%.c
	gcc -c -I$(INC) -o $@ $<

.PHONY:mkbuilddir
mkbuilddir:
	mkdir -p build

.PHONY:clean 
clean:
	rm -rf build


