# Cetris
Tetris in C + Raylib

![screenshot000](https://github.com/user-attachments/assets/0869febe-4d5b-4a98-ba35-87a05cd635fe)

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

### Keybindings
- z -> Rotate the piece Clockwise
- x -> Rotate the piece Anticlockwise
- Left and Right key -> Move the piece
- Down key -> Speed up the piece
- m -> Mute/Unmute the music
- r -> Restart the game

### Further update
1. Level selection
2. Next level after some deleted blocks
3. Destroy animation
4. OpenGL Shaders
