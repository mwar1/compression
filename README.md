# Compressors
A collection of lossless compression algorithms implemented in C++.

## Building
Building is done with CMake:
```bash
mkdir build && cd build
cmake ..
make
```

## Running
Once built, the compressor executable can be run from **inside `build/`**.
```bash
Usage: compressor [--help] [--version] [--list] [--decompress] algo input output

Positional arguments:
  algo              Compression algorithm to use [nargs=0..1] [default: ""]
  input             Input file path [nargs=0..1] [default: ""]
  output            Output file path [nargs=0..1] [default: ""]

Optional arguments:
  -h, --help        shows help message and exits 
  -v, --version     prints version information and exits 
  --list            List all available compression algorithms 
  -d, --decompress  Run in decompression mode
```


## Testing
Once built, the test suite can be run from **inside `build/`** with:
```bash
./run_tests
```

As it stands, this runs all implementations in `compressor_impls/` against all test files in `inputs/`. Most of the test inputs are from the [Canterbury Corpus](https://corpus.canterbury.ac.nz/), apart from the images which are custom made. 
Future work will ideally update this to target certain implementations.

## Benchmarking
`benchmark.py` is provided as a fairly rudimentary benchmarking script. It runs all implementations in `compressor_impls/` against all test files in `inputs/`, recording time taken and compression ratio for each. Importantly, the project must be built before the benchmark script can be run, to allow the compression executable to be created.
