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
Usage: compressor [--help] [--version] {compress,decompress,list}

Optional arguments:
  -h, --help     shows help message and exits 
  -v, --version  prints version information and exits 

Subcommands:
  compress      
  decompress    
  list          List all available compression algorithms
```

Compression and decompression require three arguments: `algo`, `input` and `output`.

## Testing
Once built, the test suite can be run from **inside `build/`**.
```bash
Usage: run_tests [--help] [--algo=<name>] [doctest_options]

Optional arguments:
  -h, --help        shows help message and exits 
  --algo=<name>     filters the test suite to run only the specified algorithm

Doctest options:
  [doctest_options] passes through native doctest options
```

By default (i.e. without the `algo` argument), this runs all implementations in `compressor_impls/` against all test files in `inputs/`.  Most of the test inputs are from the [Canterbury Corpus](https://corpus.canterbury.ac.nz/), apart from the images which are custom made.

## Benchmarking
`benchmark.py` is provided as a fairly rudimentary benchmarking script. It runs all implementations in `compressor_impls/` against all test files in `inputs/`, recording time taken and compression ratio for each. Importantly, the project must be built before the benchmark script can be run, to allow the compression executable to be created.
