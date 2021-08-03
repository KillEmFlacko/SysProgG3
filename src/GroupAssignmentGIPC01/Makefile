# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = dining_philosophers lamport

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

# ------------ CALCOLO DELLE DIPENDENZE ------------

# Prendo gli include in ogni source file
SRCS=$(shell find *.c 2>/dev/null)
MY_INCLUDES = \
	$(foreach src, $(SRCS), \
		$(filter $(INCLUDE)%,$(shell $(CC) -I $(INCLUDE) -M $(src)))\
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

CFLAGS += -DKEY_FILE=\"$(BUILD_DIR)/tmp.key\"

# Genero le path assolute degli object file e dell'eseguibile
OBJS_ABSPATH = $(addprefix $(BUILD_DIR)/,$(OBJS))
EXECS_ABSPATH = $(addprefix $(BIN_DIR)/,$(EXECS))

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(BUILD_DIR)/tmp.key $(EXECS_ABSPATH)

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

$(BIN_DIR)/dining_philosophers : $(BUILD_DIR)/dining_philosophers.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -pthread -o $@ $^

$(BIN_DIR)/lamport : $(BUILD_DIR)/main_lamport.o $(BUILD_DIR)/lamport.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D)) $@

$(BUILD_DIR)/tmp.key:
	-mkdir -p $(dir $@)
	echo "Ciccio" > $@

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

dining_philosophers.o: ../../include/CommonAssignmentIPC01/libsp.h
lamport.o: ../../include/GroupAssignmentGIPC01/lamport.h
lamport.o: ../../include/lib/lib.h
lamport.o: ../../include/CommonAssignmentIPC01/libsp.h
main_lamport.o: ../../include/GroupAssignmentGIPC01/lamport.h
main_lamport.o: ../../include/lib/lib.h
main_lamport.o: ../../include/CommonAssignmentIPC01/libsp.h
