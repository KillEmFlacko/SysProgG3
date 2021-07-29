# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = hpfs

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

CFLAGS_hpfs.o := -Wno-error=unused-command-line-argument

# ------------ CALCOLO DELLE DIPENDENZE ------------

# Include files

# Prendo gli include in ogni source file
SRCS=$(shell find *.c 2>/dev/null)
MY_INCLUDES = $(wildcard $(INCLUDE)/$(MODULE_NAME)/*.h)

# ------------ GENERAZIONE PATH ASSOLUTE ------------

# Genero i nomi dei file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

BUILD_DIR = $(BUILD)/$(MODULE_NAME)
BIN_DIR = $(BUILD_DIR)/bin

# Genero le path assolute degli object file e dell'eseguibile
EXECS_ABSPATH = $(addprefix $(BIN_DIR)/,$(EXECS))

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

# Depends on bin/include bin/*.c and bin/Makefile
all: $(addprefix $(BUILD_DIR)/,$(MODULE_NAME)) $(addprefix $(BUILD_DIR)/,$(SRCS)) $(BUILD_DIR)/Makefile
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) modules

# Create a symlink from src to bin
$(BUILD_DIR)/%.c: $(SRC)/$(MODULE_NAME)/%.c
	-mkdir -p $(dir $@)
	ln -s $< $@

# Create a symlink from src to bin
$(BUILD_DIR)/$(MODULE_NAME): $(INCLUDE)/$(MODULE_NAME)
	-mkdir -p $(dir $@)
	ln -s $< $@

# Generate a Makefile with the needed obj-m and mymodule-objs set
$(BUILD_DIR)/Makefile:
	-mkdir -p $(dir $@)
	# TODO: command line argument fix
	printf "obj-m += $(EXECS).o\n$(EXECS)-objs := $(subst $(EXECS).o,, $(subst .c,.o,$(subst $(SRC)/,,$(SRCS))))\n" > $@

#depend:
#	makedepend -I $(INCLUDE_REL) $(SRCS) -Y /usr/include
#	for dir in $(SUBDIRS); do \
#		$(MAKE) -C $$dir $@; \
#	done

# Template recipe per gli eseguibili
#
#$(BIN_DIR)/nome_eseguibile : $(BUILD_DIR)/main_eseguibile.o $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

# Gestione delle dipendenze
#$(DEPS_ABSPATH):
#	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D)) $@

# Recipe per gli object files
#$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
#	-mkdir -p $(dir $@)
#	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

