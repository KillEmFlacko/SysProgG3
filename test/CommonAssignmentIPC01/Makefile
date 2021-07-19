# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = CAIPC01_shm_test CAIPC01_sem_test CAIPC01_monitor_test CAIPC01_msgq_test

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))_test

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

# Genero le path assolute degli object file e dell'eseguibile
EXECS_ABSPATH = $(addprefix $(BIN_DIR)/,$(EXECS))

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean test

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

$(BIN_DIR)/CAIPC01_sem_test : $(BUILD_DIR)/main_sem.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/CAIPC01_shm_test : $(BUILD_DIR)/main_shm.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/CAIPC01_monitor_test : $(BUILD_DIR)/main_monitor.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/CAIPC01_msgq_test : $(BUILD_DIR)/main_msgq.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D))

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

test:
	for e in $(EXECS_ABSPATH); do \
		$$e; \
	done

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

main_monitor.o: ../../include/CommonAssignmentIPC01/libsp.h
main_msgq.o: ../../include/CommonAssignmentIPC01/libsp.h
main_sem.o: ../../include/CommonAssignmentIPC01/libsp.h
main_shm.o: ../../include/lib/lib.h
main_shm.o: ../../include/CommonAssignmentIPC01/libsp.h