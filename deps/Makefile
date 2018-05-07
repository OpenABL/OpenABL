.PHONY: all clean-mason clean-flame clean-flamegpu clean-dmason
all: mason flame flamegpu dmason

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

clean-flame:
	rm -rf flame

#
# FlameGPU
#

flamegpu: flamegpu/FLAMEGPU

flamegpu/FLAMEGPU:
	git clone https://github.com/FLAMEGPU/FLAMEGPU flamegpu/FLAMEGPU

clean-flamegpu:
	rm -rf flamegpu

#
# Mason
#

mason: mason/mason.jar

mason/mason.jar:
	mkdir -p mason
	wget https://cs.gmu.edu/~eclab/projects/mason/mason.19.jar \
		-O mason/mason.jar

clean-mason:
	rm -rf mason

#
# DMason
#

dmason: dmason/dmason/target/DMASON-3.2.jar

dmason/dmason/target: | dmason/dmason
	cd dmason/dmason; \
		mvn -Dmaven.test.skip=true -Dmaven.javadoc.skip=true package

dmason/dmason: dmason/dmason.zip
	cd dmason; \
		unzip -D dmason.zip; \
		mv dmason-master dmason

dmason/dmason.zip:
	mkdir -p dmason
	wget https://github.com/isislab-unisa/dmason/archive/master.zip -O dmason/dmason.zip

clean-dmason:
	rm -rf dmason
