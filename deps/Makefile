.PHONY: all
all: mason flame flamegpu

#
# Flame
#

flame: flame/xparser/xparser flame/libmboard-install

flame/libmboard-install: flame/libmboard/Makefile
	cd flame/libmboard; \
		make && make install

flame/libmboard/Makefile: flame/libmboard/configure
	cd flame/libmboard; \
		./configure \
		--prefix=`pwd`/../libmboard-install \
		--disable-tests \
		--disable-parallel

flame/libmboard/configure: | flame/libmboard
	cd flame/libmboard; \
		./autogen.sh

flame/libmboard:
	git clone https://github.com/FLAME-HPC/libmboard.git flame/libmboard

flame/xparser/xparser: | flame/xparser
	cd flame/xparser; \
		make

flame/xparser:
	git clone https://github.com/FLAME-HPC/xparser.git flame/xparser

#
# FlameGPU
#

flamegpu: flamegpu/FLAMEGPU

flamegpu/FLAMEGPU:
	git clone https://github.com/FLAMEGPU/FLAMEGPU flamegpu/FLAMEGPU

#
# Mason
#

mason: mason.zip
	unzip -D mason.zip

mason.zip:
	wget https://cs.gmu.edu/~eclab/projects/mason/mason.zip
