# Entify Julia

## Dependant packages

The following Julia packages are required by Entify.  You will need to Pkg.add()
them.

```
ArgParse
Images
StaticArrays
Parameters
ColorTypes
FixedPointNumbers
MacroTools
Memoize
Rotations
Distributions
HTTP
MbedTLS
ZipFile
ImageMagick
```

## Troubleshooting

On the Jetson Nano, when trying to run the app I get an error:

```
Building Arpack ──────────→ `~/.julia/packages/Arpack/UiiMc/deps/build.log`
┌ Error: Error building `Arpack`: 
│ [ Info: Downloading https://github.com/JuliaLinearAlgebra/ArpackBuilder/releases/download/v3.5.0-3/Arpack.v3.5.0-3.aarch64-linux-gnu-gcc7.tar.gz to /home/rogercritchlow/.julia/packages/Arpack/UiiMc/deps/usr/downloads/Arpack.v3.5.0-3.aarch64-linux-gnu-gcc7.tar.gz...
│ ERROR: LoadError: LibraryProduct(nothing, ["libarpack"], :libarpack, "Prefix(/home/rogercritchlow/.julia/packages/Arpack/UiiMc/deps/usr)") is not satisfied, cannot generate deps.jl!
```

This is documented at https://github.com/JuliaLinearAlgebra/Arpack.jl/issues/54 .

Seems like the solution is to:

```
sudo apt install libarpack2
cp /usr/lib/aarch64-linux-gnu/libarpack.so.2.0.0 ~/.julia/packages/Arpack/.../deps/usr/lib
```

