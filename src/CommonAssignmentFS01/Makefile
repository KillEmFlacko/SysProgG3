# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = dnfs

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

# Additional compile flags
CFLAGS += -D_FILE_OFFSET_BITS=64

# Additional linking flags
LFLAGS += -D_FILE_OFFSET_BITS=64

# ------------ CALCOLO DELLE DIPENDENZE ------------

# Prendo gli include in ogni source file
SRCS=$(shell find *.c 2>/dev/null)
MY_INCLUDES = \
	$(foreach src, $(SRCS), \
		$(filter $(INCLUDE)%,$(shell $(CC) $(CFLAGS) -I $(INCLUDE) -M $(src)))\
	 )

# Trasformo gli include in moduli di dipendenza 
DEPS = $(sort $(filter-out /$(MODULE_NAME)/, $(dir $(patsubst $(INCLUDE)%,%,$(MY_INCLUDES)))))

# Genero le path assolute dei relativi file oggetto
DEPS_ABSPATH = \
	$(foreach dep,$(DEPS), \
		$(patsubst $(SRC)%.c,$(BUILD)%.o,$(shell find $(SRC)$(dep)*.c)) \
	 )

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

all: $(EXECS_ABSPATH)

depend:
	makedepend -I $(INCLUDE_REL) $(SRCS) -Y /usr/include
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Template recipe per gli eseguibili
#
#$(BIN_DIR)/nome_eseguibile : $(BUILD_DIR)/main_eseguibile.o $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/dnfs : $(BUILD_DIR)/dnfs.o $(BUILD_DIR)/log.o #$(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^ -lfuse -pthread

# Gestione delle dipendenze
#$(DEPS_ABSPATH):
#	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D))

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

dnfs.o: ../../include/CommonAssignmentFS01/p.h
dnfs.o: ../../include/CommonAssignmentFS01/log.h
log.o: ../../include/CommonAssignmentFS01/p.h
log.o: ../../include/CommonAssignmentFS01/log.h