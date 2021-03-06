#====================================================================================
#----------------------------------USER VARIABLES------------------------------------
#====================================================================================
INSTALL_DIR=/usr/bin
INSTALL_NAME=pcapserver 

#====================================================================================
#-------------------------------------VARIABLES--------------------------------------
#====================================================================================

CC=gcc
CFLAGS=-c
WARNINGS=-W -Wall -Wextra -pedantic -Wno-write-strings -fno-strict-aliasing

SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=bin/obj

ifdef DEBUG
CFLAGS+=-ggdb -DDEBUG
else
CFLAGS+=-Os -DNDEBUG
endif

.PHONY: all init clean install uninstall

OBJS=main.o
LIBRARIES=-lpcap -lpthread

#====================================================================================
#-------------------------------------BUILD ALL--------------------------------------
#====================================================================================
all: server

init:
	mkdir -p bin/obj


#====================================================================================
#------------------------------------BKDOOR BUILD------------------------------------
#====================================================================================
server:  $(OBJS)
	cd $(OBJ_DIR); $(CC) -o ../../$(BIN_DIR)/$@ $^ $(LIBRARIES)
	
%.o: $(SRC_DIR)/%.c init
	$(CC) $(CFLAGS) $(WARNINGS) -o $(OBJ_DIR)/$@ $<


#====================================================================================
#----------------------------------------CLEAN---------------------------------------
#====================================================================================
clean:
	rm -rf $(BIN_DIR)


#====================================================================================
#---------------------------------------INSTALL--------------------------------------
#====================================================================================
install: server
	cp $(BIN_DIR)/server $(INSTALL_DIR)/$(INSTALL_NAME)
uninstall:
	rm -f $(INSTALL_DIR)/$(INSTALL_NAME)
