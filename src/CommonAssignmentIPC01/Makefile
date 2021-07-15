# Set executable name and dependencies
#EXEC = program

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
DEPS = $(filter-out /$(MODULE_NAME)/, $(dir $(patsubst $(INCLUDE)%,%,$(MY_INCLUDES))))

# Genero le path assolute dei relativi file oggetto
DEPS_ABSPATH = \
	$(foreach dep,$(DEPS), \
		$(patsubst $(SRC)%.c,$(BUILD)%.o,$(shell find $(SRC)$(dep)*.c)) \
	 )

# ------------ GENERAZIONE PATH ASSOLUTE ------------

# Genero i nomi dei file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

# Genero le path assolute degli object file e dell'eseguibile
OBJS_ABSPATH = $(addprefix $(BUILD)/$(MODULE_NAME)/,$(OBJS))
#EXEC_ABSPATH = $(BUILD)/$(MODULE_NAME)/bin/$(EXEC)

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(OBJS_ABSPATH) $(DEPS_ABSPATH)

depend:
	makedepend -I $(INCLUDE_REL) $(SRCS) -Y /usr/include
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Recipe per l'eseguibile
#$(EXEC_ABSPATH): $(OBJS_ABSPATH) $(DEPS_ABSPATH)
#	-mkdir -p $(dir $@)
#	$(CC) $(LFLAGS) -o $@ $^

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D))

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c $(MY_INCLUDES)
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXEC_ABSPATH)

# DO NOT DELETE

libsp.o: ../../include/CommonAssignmentIPC01/libsp.h
