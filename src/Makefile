SRCS=$(shell find *.c 2>/dev/null)

# ------------ GENERAZIONE PATH ASSOLUTE ------------
SUBDIRS =  $(filter-out .,$(notdir $(shell find -maxdepth 1 -type d)))

# Genero nomi file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

# Genero le path assolute degli object file
OBJS_ABSPATH = $(addprefix $(BUILD)/,$(OBJS))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(OBJS_ABSPATH)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

depend:
	makedepend -I $(INCLUDE) $(SRCS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Recipe per gli object files
$(BUILD)/%.o: %.c
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# DO NOT DELETE
