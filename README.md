# Compressors
A collection of lossless compression algorithms implemented in C++.

## Testing
To run the test suite:
```bash
mkdir build && cd build
cmake ..
make
./run_tests
```

As it stands, this runs all implementations in `compressor_impls/` against all test files in `inputs/`. Most of the test inputs are from the [Canterbury Corpus](https://corpus.canterbury.ac.nz/), apart from the images which are custom made. 
Future work will ideally update this to target certain implementations.

## Benchmarking
TODO
