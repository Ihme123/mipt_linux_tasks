DIRS = useless backup runsim

all:
	-for d in $(DIRS); do $(MAKE) -C "$$d"; done
