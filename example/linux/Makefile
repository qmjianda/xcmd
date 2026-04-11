BIN = xcmder
CC = gcc
CFLAGS = -g -O0 -Wall -I./ -I../../inc -I../../extensions/vfs -DXCMD_USE_VFS

SRC_DIR = ../../src
EXT_DIR = ../../extensions
BUILD_DIR = build

SRC = $(SRC_DIR)/xcmd_io.c \
       $(SRC_DIR)/xcmd_session.c \
       $(SRC_DIR)/xcmd_line_editor.c \
       $(SRC_DIR)/xcmd_term.c \
       $(SRC_DIR)/xcmd_term_ansi.c \
       $(SRC_DIR)/xcmd_term_dumb.c \
       $(SRC_DIR)/xcmd_cmd.c \
       $(SRC_DIR)/xcmd_default_cmds.c

EXT_SRC = $(EXT_DIR)/test/test.c \
          $(EXT_DIR)/ex_list/ex_list.c \
          $(EXT_DIR)/ex_cmds/ex_cmds.c \
          $(EXT_DIR)/net_cmds/socket_cmds.c \
          $(EXT_DIR)/fs_cmds/FatFs/source/diskio.c \
          $(EXT_DIR)/fs_cmds/FatFs/source/ff.c \
          $(EXT_DIR)/fs_cmds/FatFs/source/ffunicode.c \
          $(EXT_DIR)/fs_cmds/fatfs_port.c \
          $(EXT_DIR)/vfs/xcmd_vfs.c \
          $(EXT_DIR)/vfs/xcmd_vfs_fatfs.c \
          $(EXT_DIR)/vfs/xcmd_vfs_cmds.c

EXT_INC = -I$(EXT_DIR)/test \
          -I$(EXT_DIR)/ex_list \
          -I$(EXT_DIR)/ex_cmds \
          -I$(EXT_DIR)/net_cmds \
          -I$(EXT_DIR)/fs_cmds/FatFs/source \
          -I$(EXT_DIR)/fs_cmds \
          -I$(EXT_DIR)/vfs

# Generate object file paths in build directory
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC)) \
      $(patsubst $(EXT_DIR)/%.c,$(BUILD_DIR)/ext/%.o,$(EXT_SRC)) \
      $(BUILD_DIR)/linux_main.o \
      $(BUILD_DIR)/fatfs_disk_mmc.o \
      $(BUILD_DIR)/fatfs_disk_ram.o

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/ext/test \
		$(BUILD_DIR)/ext/ex_list \
		$(BUILD_DIR)/ext/ex_cmds \
		$(BUILD_DIR)/ext/net_cmds \
		$(BUILD_DIR)/ext/fs_cmds/FatFs/source \
		$(BUILD_DIR)/ext/vfs

$(BIN): $(OBJ)
	$(CC) $^ -o $@ -Wl,-Map,$@.map

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ext/%.o: $(EXT_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EXT_INC) -c $< -o $@

$(BUILD_DIR)/linux_main.o: linux_main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EXT_INC) -I$(EXT_DIR)/fs_cmds -c $< -o $@

$(BUILD_DIR)/fatfs_disk_mmc.o: fatfs_disk_mmc.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EXT_INC) -c $< -o $@

$(BUILD_DIR)/fatfs_disk_ram.o: fatfs_disk_ram.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EXT_INC) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN) $(BIN).map

.PHONY: all clean
