# Tinyspline examples

The [tinyspline](https://github.com/msteinbeck/tinyspline) cxx/fltk examples done in C, using [raylib](https://github.com/raysan5/raylib) + [nuklear](https://github.com/Immediate-Mode-UI/Nuklear). [cvector](https://github.com/eteran/c-vector) is in there too :)


## Building

You will need [meson](https://mesonbuild.com/Getting-meson.html) and [ninja](https://ninja-build.org/) to build this project.
You will also need to build raylib and put `libraylib.a` in external/raylib.

**Build**


```
meson setup build
meson compile -C build
```

This will create the '`build`' directory.
The executable `tinyspline` will be in the `build` directory.

This was only test on linux, so no idea if it works properly on anything else.


## Running

```
./build/tinyspline
```


## License

The code in this repository was directly derived from the tinyspline demo, so use their license, or the one here with my name in it? I don't know.

Licenses for included libraries are in their respective folders in 'external'.
