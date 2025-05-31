##
# Project Lumiere
#
# @file
# @version 0.1

CC = gcc

SRCDIR = src
OBJDIR = obj
BINDIR = bin

TARGET = lumerie

INCLUDE_DIR = ./include/
LIB_DIR = ./libs/

LUAJIT_INCLUDE = ./vendor/luajit/src/
LUAJIT_LIB = ./vendor/luajit/src/

CFLAGS = -Wall -Wextra -O0 -DDEBUG -g -std=gnu11 -I$(INCLUDE_DIR) -I$(LUAJIT_INCLUDE)

LDFLAGS = -L$(LIB_DIR) -L$(LUAJIT_LIB)

LDFLAGS += -lwayland-client -lwayland-cursor -lxkbcommon
LDFLAGS += -lpthread -ldl -lrt -lm
LDFLAGS += -lluajit

SOURCES = $(shell find $(SRCDIR) -name "*.c")
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

.PHONY: all clean run

all: $(BINDIR) $(OBJDIR) $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)

run: all
	$(BINDIR)/$(TARGET)


# end
