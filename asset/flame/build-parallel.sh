$FLAME_XPARSER_DIR/xparser -p -f XMLModelFile.xml && \
  LIBS="-pthread" make && \
  ./build_runner.sh
