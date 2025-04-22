# Cetris

Tetris in C + Raylib (and WEB using emscripten and wasm)

![screenshot000](https://github.com/user-attachments/assets/08fe8475-2bf2-4355-bc63-cfb857be759e)

### Build and Run

I personally use Clang on both Linux and Macos (i'm pretty sure you can substitute clang with gcc in the nob.c and still compile)

Clone the repo and enter:

```
$ git clone https://github.com/lmarzocchetti/cetris
$ cd cetris
```

Build the nob:

```
$ clang -o nob nob.c
```

Compile the executable dynamic

```
$ <install raylib with homebrew or apt/dnf/pacman>
$ ./nob Debug|Release
```

or if you want to link statically

```
$ ./nob Static <path-to-libraylib.a>
```

Play:

```
$ ./cetris
```

### Emscripten build

1. Download and compile raylib targeting the web following the raylib guide (https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)) in a folder called raylib-5.5
2. Uncomment the line "// #define EMSCRIPTEN" in nob.c
3. In cetris folder: `$ ./nob`
4. `$ python -m http.server 8080`
5. Go in your browser to: "http://localhost:8080/cetris.html"
6. Play!

To play the music you need to click on the canvas!

#### Centering the Canvas

By default emscripten generate an html file which is not centered, if you interested copy this:

```
<style>
  body {
    margin: 0;
    overflow: hidden;
    background-color: #000;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
  }
  canvas.emscripten {
    border: 0 none;
    background-color: #000;
  }
</style>
```

and substitute to the <style> generated automatically by emscripten.

### Keybindings

- z -> Rotate the piece Clockwise
- x -> Rotate the piece Anticlockwise
- Left and Right key -> Move the piece
- Down key -> Speed up the piece
- m -> Mute/Unmute the music
- r -> Restart the game

### Further update

- [ ] Level selection
- [x] Next level after some deleted blocks
- [ ] Destroy animation
- [x] OpenGL Shaders
- [x] Web support (emscripten)

### Windows support

If you want to add windows support you can do push request. Need to add some preprocessor on nob.c and os independent path in main.c
