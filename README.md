# OpenABL

OpenABL is a work-in-progress domain-specific language for agent based simulations. It it designed to compile to multiple backends targeting different computing architectures, including single CPUs,
GPUs and clusters.

## Installation

The build requires `flex`, `bison`, `cmake` and a C++11 compatible C++
compiler. The build requirements can be installed using:

```sh
sudo apt-get install flex bison cmake g++
```

An out-of-source build can be performed using:

```sh
mkdir ./build
cmake -Bbuild -H.
make -C build -j4
```

## Installation of backend libraries

OpenABL supports a number of backend libraries, which need to be installed
separately. For convenience a script to download and build the different
backend libraries is provided.

Some of the backends have additional build or runtime dependencies. Most of them
can be installed by running:

```sh
sudo apt-get install git autoconf libtool libxml2-utils xsltproc default-jdk \
                     libgl1-mesa-dev libglu1-mesa-dev libglew-dev freeglut3-dev
```

FlameGPU additionally requires a CUDA installation.

The backends can then be downloaded and built using the following command:

```sh
# To build all
make -C deps

# To build only a specific one
make -C deps mason
make -C deps flame
make -C deps flamegpu
make -C deps dmason
```

## Running

Examples are located in the `examples` directory.

To compile the `examples/circle.abl` example using the Mason backend:

```sh
build/OpenABL -i examples/circle.abl -o ./output -b mason
```

The result will be written into the `./output` directory. To run the generated code:

```sh
cd ./output
./build.sh
./run.sh
```

For the `circle.abl` example, this will create a `points.json` file.

You can also automatically build and run the generated code (if this is supported by the backend):

```sh
# Generate + Build
build/OpenABL -i examples/circle.abl -o ./output -b mason -B
# Generate + Build + Run
build/OpenABL -i examples/circle.abl -o ./output -b mason -R
```

If the backend supports it, it is also possible to run with visualization:

```sh
build/OpenABL -i examples/circle.abl -b mason -C visualize=true -R
```

If `-R` is used, the output directory can be omitted. In this case a temporary directory will be
used.

## Running benchmarks

To run benchmarks for the different backends against our samples models, the
`bench/bench.py` script can be used. The script requires Python 2.7 or
Python >= 3.2. Usage summary:

```
usage: bench.py [-h] [-b BACKENDS] [-m MODELS] [-n NUM_AGENTS] [-r RESULT_DIR]
                [-M SEC]

optional arguments:
  -h, --help            show this help message and exit
  -b BACKENDS, --backends BACKENDS
                        Backends to benchmark (comma separated)
  -m MODELS, --models MODELS
                        Models to benchmark (comma separated)
  -n NUM_AGENTS, --num-agents NUM_AGENTS
                        Number of agent range (min-max)
  -r RESULT_DIR, --result-dir RESULT_DIR
                        Directory for benchmark results
  -M SEC, --max-time SEC
                        (Apprimate) maximal time per backend per model
```

Some example usages:

```sh
# Benchmark default backends against default models with default agent numbers
# Write results to results/ directory
python bench/bench.py -r results/

# Do the same, but limit time spend on each backend+model combination to
# approximately 30 seconds. With the default benchmark configuration of
# 4 backends and 6 models this will take approximately 4*6*30s = 12min
python bench/bench.py -r results/ --max-time 30

# Run circle and boids2d models only with 250 to 64000 agents
python bench/bench.py -r results/ -b mason -m circle,boids2d \
       --num-agents=250-64000
```

Benchmark results are both written to stdout and the specified results
directory. Subsequently it is possible to plot the obtained runtimes. This
requires matplotlib to be installed:

```sh
sudo apt-get install python-matplotlib
```

Plotting can then be performed by calling:

```sh
python bench/plot.py results/
```

## Help

Output of `OpenABL --help`:

```
Usage: ./OpenABL -i input.abl -o ./output-dir -b backend

Options:
  -A, --asset-dir    Asset directory (default: ./asset)
  -b, --backend      Backend
  -B, --build        Build the generated code
  -C, --config       Specify a configuration value (name=value)
  -D, --deps         Deps directory (default: ./deps)
  -h, --help         Display this help
  -i, --input        Input file
  -o, --output-dir   Output directory
  -P, --param        Specify a simulation parameter (name=value)
  -R, --run          Build and run the generated code

Available backends:
 * c
 * flame
 * flamegpu
 * mason
 * dmason

Available configuration options:
 * bool use_float (default: false, flame/gpu only)
 * bool visualize (default: false, d/mason only)
```

### Configuration options

 * `bool use_float = false`: By default models are compiled to use double-precision floating point
   numbers, as some backends only support doubles. For the Flame and FlameGPU backends this option
   may be enabled to use single-precision floating point numbers instead.
 * `bool visualize = false`: Display a graphical visualization of the model. This option is
   currently only supported by the Mason and DMason backends.

## Environment configuration

To use the automatic build and run scripts, some environment variables have to
be set for the different backends. If you are using the `deps` Makefile, then
OpenABL will automatically set these environment variables when building and
running. However, you need to set these environment variables either if you are
using a non-standard configuration, or want to manually invoke the build and
run scripts.

 * `c` backend:
   * None.
 * `flame` backend:
   * `FLAME_XPARSER_DIR` must be set to the xparser directory.
   * `LIBMBOARD_DIR` must be set to the libmboard
     directory.
 * `flamegpu` backend:
   * `FLAMEGPU_DIR` must be set to the FLAMEGPU directory.
   * CUDA must be in `PATH` and `LD_LIBRARY_PATH`.
   * `SMS` can be used to specify the SM architecture. This defaults to
     `"30 35 37 50 60"`
 * `mason` backend:
   * `MASON_JAR` must be set to the MASON Jar file.
 * `dmason` backend:
   * `DMASON_JAR` must be set to the DMASON Jar file.
   * `DMASON_RESOURCES` must be set to the DMASON `resources` directory.
