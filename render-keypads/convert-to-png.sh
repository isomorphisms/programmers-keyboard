#!/usr/bin/env bash
set -euo pipefail

picture_dir="pictures of the keypads"

find "$picture_dir" -maxdepth 1 -type f -name '*.svg' -print0 |
  while IFS= read -r -d '' svg; do
    stem="${svg%.svg}"
    rsvg-convert --background-color white --output "$stem.png" "$svg"
    convert "$stem.png" -colorspace Gray "$stem.pgm"
  done
