# Dinap Release Note Generator

## Build
```shell
> git submodule update --init --recursive
> mkdir build && cd build
> cmake ..
> cmake --build . --target dinapRelNotes
```

## Usage
```shell 
> dinapRelNotes --help

A simple release note generator
Usage:
  DinapRelNotes [OPTION...]

  -r, --root arg    Root for files & references (default: .)
  -c, --config arg  Config file (default: ./config.yml)
  -o, --out arg     Output directory for html files (default: ./output)
  -s, --silent      Hide banner
  -h, --help        Print usage
```

## Using examples
```shell
$ dinapRelNotes -r example -c example/configs/DinapRelNote-1.0.0.yml
```