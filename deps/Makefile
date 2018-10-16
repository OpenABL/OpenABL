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

flamegpu/FLAMEGPU: flamegpu/FLAMEGPU.zip
	cd flamegpu; \
		unzip -D FLAMEGPU.zip; \
		mv FLAMEGPU-master FLAMEGPU

flamegpu/FLAMEGPU.zip:
	mkdir -p flamegpu;
	wget https://github.com/FLAMEGPU/FLAMEGPU/archive/master.zip \
		-O flamegpu/FLAMEGPU.zip

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

dmason: dmason/DMASON-3.2.jar

dmason/DMASON-3.2.jar: dmason/DMASON-3.2.zip
	cd dmason; \
		unzip -DD DMASON-3.2.zip

dmason/DMASON-3.2.zip:
	mkdir -p dmason;
	wget https://github.com/OpenABL/OpenABL/releases/download/v0.0.1alpha3/DMASON-3.2.zip \
		-O dmason/DMASON-3.2.zip

# Instructions for a from-scratch build of DMason
# Normally the JAR variant above should be used
dmason-build: dmason-build/dmason/target/DMASON-3.2.zip

dmason-build/dmason/target/DMASON-3.2.zip: dmason-build/dmason/target/DMASON-3.2.jar
	cd dmason-build/dmason/target; \
		rm resources/image/*.gif; \
		zip -r DMASON-3.2.zip DMASON-3.2.jar DMASON-3.2-lib.jar resources/

dmason-build/dmason/target/DMASON-3.2.jar: | dmason-build/dmason
	cd dmason-build/dmason; \
		mvn -Dmaven.test.skip=true -Dmaven.javadoc.skip=true package

dmason-build/dmason: dmason-build/dmason.zip
	cd dmason-build; \
		unzip -D dmason.zip; \
		mv dmason-master dmason

dmason-build/dmason.zip:
	mkdir -p dmason-build
	wget https://github.com/isislab-unisa/dmason/archive/master.zip \
		-O dmason-build/dmason.zip

clean-dmason:
	rm -rf dmason
