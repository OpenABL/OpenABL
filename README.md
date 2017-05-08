# OpenABL

OpenABL is a work-in-progress domain-specific language for agent based simulations. It it designed to compile to multiple backends.

## Installation

Requirements: `flex` and `bison`. (`sudo apt-get install flex bison`)

For an out-of-source build:

```sh
mkdir ./build
cd ./build
cmake ..
make -j4
```

## Running

Examples are located in the `examples` directory.

To compile the `examples/circle.abl` example using the C backend:

```sh
./OpenABL -i ../examples/circle.abl -o ./output --backend c
```

The result will be written into the `./output` directory. To run the generated code:

```sh
cd ./output
./build.sh
./a.out
```

For the `circle.abl` example, this will create a `points.out` file (JSON).

## Help

Output of `./OpenABL --help`:

```
Usage: ./OpenABL -i input.abl -o ./output-dir

Options:
  -A, --asset-dir    Asset directory (default: ./asset)
  -b, --backend      Backend (default: c)
  -h, --help         Display this help
  -i, --input        Input file
  -o, --output-dir   Output directory

Available backends:
 * c        (working)
 * flame    (partially working)
 * flamegpu (not working)
```
