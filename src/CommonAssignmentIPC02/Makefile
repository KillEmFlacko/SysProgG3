# Set executable name and dependencies
EXEC = program

# ------------ CALCOLO DELLE DIPENDENZE ------------

# Prendo gli include in ogni source file
SRCS=$(shell find *.c 2>/dev/null)
MY_INCLUDES = \
	$(foreach src, $(SRCS), \
		$(filter $(INCLUDE)%,$(shell $(CC) -I $(INCLUDE) -M $(src)))\
	 )

# Trasformo gli include in moduli di dipendenza 
DEPS = $(dir $(patsubst $(INCLUDE)%,%,$(MY_INCLUDES)))

# Genero le path assolute dei relativi file oggetto
DEPS_ABSPATH = \
	$(foreach dep,$(DEPS), \
		$(patsubst $(SRC)%.c,$(BUILD)%.o,$(shell find $(SRC)$(dep)*.c)) \
	 )

# ------------ GENERAZIONE PATH ASSOLUTE ------------

# Recupero il nome del modulo in analisi
MODULE_NAME = $(notdir $(shell pwd))

# Genero i nomi dei file oggetto
OBJS = $(patsubst %.c,%.o,$(SRCS))

# Genero le path assolute degli object file e dell'eseguibile
OBJS_ABSPATH = $(addprefix $(BUILD)/$(MODULE_NAME)/,$(OBJS))
EXEC_ABSPATH = $(BUILD)/$(MODULE_NAME)/bin/$(EXEC)

# Genero path relativa per gli include file del modulo
INCLUDE_REL = $(shell realpath --relative-to . $(INCLUDE))

# ------------ DEFINIZIONE DELLE RECIPES ------------

.PHONY: all clean

all: $(EXEC_ABSPATH)

depend:
	makedepend -I $(INCLUDE_REL) $(SRCS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

# Recipe per l'eseguibile
$(EXEC_ABSPATH): $(OBJS_ABSPATH) $(DEPS_ABSPATH)
	-mkdir -p $(dir $@)
	$(CC) $(LFLAGS) -o $@ $^

# Gestione delle dipendenze
$(DEPS_ABSPATH):
	$(MAKE) -C $(patsubst $(BUILD)%,$(SRC)%,$(@D))

# Recipe per gli object files
$(BUILD)/$(MODULE_NAME)/%.o: %.c
	-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE) -c -o $@ $(notdir $(basename $@)).c

clean: 
	rm -f *~ *.bak $(BUILD)/$(MODULE_NAME)/*.o $(EXEC_ABSPATH)

# DO NOT DELETE

main.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
main.o: /usr/include/features.h /usr/include/stdc-predef.h
main.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
main.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
main.o: /usr/include/bits/types.h /usr/include/bits/timesize.h
main.o: /usr/include/bits/typesizes.h /usr/include/bits/time64.h
main.o: /usr/include/bits/types/__fpos_t.h
main.o: /usr/include/bits/types/__mbstate_t.h
main.o: /usr/include/bits/types/__fpos64_t.h /usr/include/bits/types/__FILE.h
main.o: /usr/include/bits/types/FILE.h /usr/include/bits/types/struct_FILE.h
main.o: /usr/include/bits/stdio_lim.h /usr/include/bits/floatn.h
main.o: /usr/include/bits/floatn-common.h /usr/include/stdlib.h
main.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
main.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
main.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
main.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/bits/endianness.h /usr/include/bits/byteswap.h
main.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
main.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
main.o: /usr/include/bits/types/__sigset_t.h
main.o: /usr/include/bits/types/struct_timeval.h
main.o: /usr/include/bits/types/struct_timespec.h
main.o: /usr/include/bits/pthreadtypes.h
main.o: /usr/include/bits/thread-shared-types.h
main.o: /usr/include/bits/pthreadtypes-arch.h
main.o: /usr/include/bits/struct_mutex.h /usr/include/bits/struct_rwlock.h
main.o: /usr/include/alloca.h /usr/include/bits/stdlib-float.h
main.o: ../../include/lib/lib.h
