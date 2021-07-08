SUBDIRS =  $(filter-out .,$(notdir $(shell find -maxdepth 1 -type d)))

.PHONY: all clean

all depend:

clean: 
	rm -f *~ *.bak
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done

