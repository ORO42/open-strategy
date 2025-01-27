# Open-Strategy

A cross-platform 2D turn-based strategy game

## Features

### Technical Features

- P2P Multiplayer
- Grid-based spatial culling
- Sparse data storage
- Supports very large maps with high performance
- Composition pattern
- Observer pattern
- Entity Component System
- 3D projection collisions
- Elevation-aware line-of-sight
- Polygonal collisions
- Polygon rotation
- AABB collisions
- Standalone map editor
- Built-in mod support

## Supported Platforms

Quickstart supports the main 3 desktop platforms:

- Windows
- Linux
- MacOS

# VSCode Users (all platforms)

_Note_ You must have a compiler toolchain installed in addition to vscode.

- Download the quickstart
- Rename the folder to your game name
- Open the folder in VSCode
- Press F5 to build
- You are good to go

# Windows Users

There are two compiler toolchains available for windows, MinGW-W64 (a free compiler using GCC), and Microsoft Visual Studio

## Using MinGW-W64

- Double click the `build-MinGW-W64.bat` file
- CD into the folder in your terminal
- run `make`
- You are good to go

### Note on MinGW-64 versions

Make sure you have a modern version of MinGW-W64 (not mingw).
The best place to get it is from the W64devkit from
https://github.com/skeeto/w64devkit/releases
or the version installed with the raylib installer

#### If you have installed raylib from the installer

Make sure you have added the path

`C:\raylib\w64devkit\bin`

To your path environment variable so that the compiler that came with raylib can be found.

DO NOT INSTALL ANOTHER MinGW-W64 from another source such as msys2, you don't need it.

## Microsoft Visual Studio

- Run `build-VisualStudio2022.bat`
- double click the `.sln` file that is generated
- develop your game
- you are good to go

# Linux Users

- CD into the build folder
- run `./premake5 gmake2`
- CD back to the root
- run `make`
- you are good to go

# MacOS Users

- CD into the build folder
- run `./premake5.osx gmake2`
- CD back to the root
- run `make`
- you are good to go

# Output files

The built code will be in the bin dir

# Working directories and the resources folder

The example uses a utility function from `path_utils.h` that will find the resources dir and set it as the current working directory. This is very useful when starting out. If you wish to manage your own working directory you can simply remove the call to the function and the header.

# Building for other OpenGL targets

If you need to build for a different OpenGL version than the default (OpenGL 3.3) you can specify an OpenGL version in your premake command line. Just modify the bat file or add the following to your command line

## For OpenGL 1.1

`--graphics=opengl11`

## For OpenGL 2.1

`--graphics=opengl21`

## For OpenGL 4.3

`--graphics=opengl43`

## For OpenGLES 2.0

`--graphics=opengles2`

## For OpenGLES 3.0

`--graphics=opengles3`

# Thanks

Special thanks to the massively helpful Raylib and EnTT communities. Thanks to Jeffery Meyers for providing the build boilerplate
