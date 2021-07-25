# Lista degli eseguibili da generare (vedi nelle recipe per la generazione)
EXECS = producer consumer producer_array consumer_array producer_queue consumer_queue #readwrite

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

# Recipe per gli eseguibili
#
#$(BIN_DIR)/nome_eseguibile : $(BUILD_DIR)/main_eseguibile.o $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/producer : $(BUILD_DIR)/main_prod.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/consumer : $(BUILD_DIR)/main_cons.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/producer_array : $(BUILD_DIR)/main_prod_array.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/consumer_array : $(BUILD_DIR)/main_cons_array.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/producer_queue : $(BUILD_DIR)/main_prod_queue.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

$(BIN_DIR)/consumer_queue : $(BUILD_DIR)/main_cons_queue.o $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

#$(BIN_DIR)/readwrite : $(BUILD_DIR)/readwrite_sem.o $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

$(BUILD_DIR)/tmp.key:
	-mkdir -p $(dir $@)
	echo "Ciccio" > $@

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D))

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXECS_ABSPATH)

# DO NOT DELETE

main_cons_array.o: ../../include/CommonAssignmentIPC01/libsp.h
main_cons.o: ../../include/CommonAssignmentIPC01/libsp.h
main_cons_queue.o: ../../include/CommonAssignmentIPC01/libsp.h
main_cons_queue.o: ../../include/lib/queue.h
main_prod_array.o: ../../include/CommonAssignmentIPC01/libsp.h
main_prod.o: ../../include/CommonAssignmentIPC01/libsp.h
main_prod_queue.o: ../../include/CommonAssignmentIPC01/libsp.h
main_prod_queue.o: ../../include/lib/queue.h
readwrite_file.o: ../../include/CommonAssignmentIPC01/libsp.h
readwrite_file.o: ../../include/lib/lib.h
readwrite_queue.o: ../../include/CommonAssignmentIPC01/libsp.h
readwrite_queue.o: ../../include/lib/lib.h
readwrite_var.o: ../../include/CommonAssignmentIPC01/libsp.h
readwrite_var.o: ../../include/lib/lib.h
