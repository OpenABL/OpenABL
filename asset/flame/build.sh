$FLAME_XPARSER_DIR/xparser XMLModelFile.xml && \
  make && \
  gcc -O2 -std=c99 runner.c libabl.c -lm -o runner
