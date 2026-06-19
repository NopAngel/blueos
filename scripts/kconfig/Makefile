# BlueOS - scripts/kconfig/Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../include
LEX = flex

TARGET = mconf
OBJS = mconf.o lex.yy.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	@echo "  KCONF      Built configuration tool successfully."

lex.yy.c: lex.lex
	$(LEX) lex.lex

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) lex.yy.c