$FLAME_XPARSER_DIR/xparser -f XMLModelFile.xml && \
  make && \
  gcc -O2 -std=c99 runner.c libabl.c -lm -o runner
