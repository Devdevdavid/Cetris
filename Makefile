EXEC = Cetris
PATH_SRC = src/
HEADER = include/
CC = gcc
CCFLAGS = -Wall -std=c99 -Os -D _BSD_SOURCE
CLIBS = -I$(HEADER) -DLOG_USE_COLOR
LDFLAGS = -lcurses -lpthread

SRC = $(wildcard $(PATH_SRC)*.c)
OBJ = $(SRC:.c=.o)

all: release

spymode: CCFLAGS += -DAI_SPY_MODE
spymode: debug
debug: CCFLAGS += -DDEBUG -g
debug: $(EXEC)
release: $(EXEC)

$(EXEC): $(OBJ)
	@$(CC) -o $(EXEC) $(LDFLAGS) $(OBJ)

%.o: %.c
	$(CC) -o $@ -c $< $(CLIBS) $(CCFLAGS)

clean:
	rm -f $(OBJ)

mrproper: clean
	rm -f $(EXEC)
