#include <iostream>
#include "ParserContext.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Missing filename argument." << std::endl;
    return 1;
  }

  char *fileName = argv[1];
  FILE *file = fopen(fileName, "r");
  if (!file) {
    std::cerr << "File " << fileName << " could not be opened." << std::endl;
    return 1;
  }

  OpenABL::ParserContext ctx(file);
  ctx.parse();

  fclose(file);
  return 0;
}
