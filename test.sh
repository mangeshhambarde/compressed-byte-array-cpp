#!/usr/bin/env bash

set -eu

(
    mkdir -p build
    cd build
    cmake ..
    make
)

run() {
  echo "-------------- Running test with $1 --------------"
  ./build/encode <"$1" >out.bin
  ./build/decode <out.bin >out.txt
  diff "$1" out.txt
}

test_example() {
  run example.dat
}

test_instructions() {
  run instructions.md
}

test_empty() {
  touch empty.dat
  run empty.dat
}

test_random() {
  dd if=/dev/urandom of=random.dat bs=1M count=10 2>/dev/null
  run random.dat
}

test_single_bit() {
  dd if=/dev/zero of=single.dat bs=1M count=1 2>/dev/null
  run single.dat
}

test_odd_bits() {
  cat /dev/urandom | tr -dc '01234' | head -c 1000 > odd.dat
  run odd.dat
}

cleanup() {
  rm -f empty.dat random.dat single.dat odd.dat out.bin out.txt
}

trap cleanup EXIT

if [ $# -ne 1 ]; then
  # run tests
  test_example
  test_instructions
  test_empty
  test_single_bit
  test_odd_bits
  test_random
else
  run "$1"
fi
