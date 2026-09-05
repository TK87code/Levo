# Levo

A modest, drop-in C utility library designed for writing small-scale programs and tools with zero friction.

## Features

It is designed to be as effortless as possible, so it's suitable to quickly code your small-scale program as soon as it popped in your head.

- No external dependencies. Just copy `levo.c` and `levo.h` into your project folder.
- You don't need to figure out complicated linker flags or compiler options.
- Written in pure C99.
- Works anywhere with a standard C compiler out of the box.
- Zero internal dynamic memory allocation. The caller always owns and allocates the memory.

But, Levo is not aimed at building massive, high-performance engines. It's a toolbox for when you want to quickly whip up a CLI tool, a tiny app, or a personal script etc.

## Getting Started

1. Download `levo.h` and `levo.c`.
2. Place them in your project directory.
3. `#include "levo.h"` in your source code.
4. Compile:
   ```bash
   gcc my_app.c levo.c -o my_app
   ```

## Available Modules
Please refer to "Quick Cheat sheet" in `levo.h` for the complete list of available functions and detailed documentation.

## License
This software is dual-licensed to the public domain and under the following license: you are granted a perpetual, irrevocable license to copy, modify, publish, and distribute this file as you see fit.
