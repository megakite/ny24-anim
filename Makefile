CFLAGS	= -Iinclude -std=c11 -w 
LDFLAGS = -lm

ifeq ($(DEBUG), 1)
	CFLAGS	+= -g -Og -fsanitize=address -fsanitize=leak
	LDFLAGS	:= -lasan $(LDFLAGS)
else
	CFLAGS	+= -O3
endif

SRC_DIR	= ./src
LIB_DIR	= ./lib
OBJ_DIR	= ./obj
BIN_DIR	= .

SRCS	= $(wildcard $(SRC_DIR)/*.c)
SRC_OBJ	= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXEC	= $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%)
LIBS	= $(wildcard $(LIB_DIR)/*.c)
LIB_OBJ	= $(LIBS:$(LIB_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean

all: $(EXEC)

$(BIN_DIR)/%: $(SRC_DIR)/%.o $(LIB_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@rm -rvf $(OBJ_DIR) $(EXEC)

-include $(SRC_OBJ:.o=.d)
-include $(LIB_OBJ:.o=.d)

