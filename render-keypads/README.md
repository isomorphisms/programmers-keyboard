# Idris2 keypad picture renderer

`src/Renderer.idr` contains the category names, semantic key labels, and sample table geometry. It uses ordinary Idris2 and does not use HTML or CSS.

From the repository root:

```sh
idris2 --build render-keypads/render-keypads.ipkg
./render-keypads/build/exec/render-keypads
bash render-keypads/convert-to-png.sh
```

The Idris2 program writes SVG because GitHub renders it directly and because text remains clear at any scale. The conversion script uses Debian's `rsvg-convert` and ImageMagick to write PNG previews and raw PGM (`P5`) files. PGM is the Netpbm greyscale format used in chapter 10 of *Real World Haskell*.
