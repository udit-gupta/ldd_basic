# set tabstop=2 number nohlsearch

ULIMIT:=$(shell ulimit -c)

all:
	-(ulimit -c 0; firefox https://wiki.archlinux.org/index.php/Kernels/Compilation/Traditional >/dev/null 2>&1) &
	-@clear
	-@echo "see https://wiki.archlinux.org/index.php/Kernels/Compilation/Traditional"
	
