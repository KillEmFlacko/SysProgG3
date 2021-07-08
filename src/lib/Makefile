SRCS=$(shell find *.c 2>/dev/null)
# ------------ GENERAZIONE PATH ASSOLUTE ------------

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

# Genero i nomi dei file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

# Genero le path assolute degli object file e dell'eseguibile
OBJS_ABSPATH = $(addprefix $(BUILD)/$(MODULE_NAME)/,$(OBJS))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(OBJS_ABSPATH)

depend:
	makedepend -I $(INCLUDE) $(SRCS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o 

# DO NOT DELETE

lib.o: /home/armando/git/SysProgG3/include/lib/lib.h
