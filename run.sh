#!/bin/sh
set -eu
if [ "$#" -ne 2 ]; then
  echo "Usage: sh run.sh DATASET OUTPUT_DIRECTORY" >&2
  exit 1
fi
# Run from the repository root; never rewrite source to select a dataset.
make
mkdir -p "$2"
for program in kmeans-serial better-kmeans-serial kmeans-parallel; do
  "./bin/$program" "$1" > "$2/$program.txt"
done
