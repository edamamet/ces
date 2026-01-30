
#!/bin/sh

DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT="$DIR/../src/unity_build.cpp"

clang $SCRIPT -o $DIR/../build/prog `pkg-config sdl3 sdl3-ttf --cflags --libs` -Werror -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -pedantic-errors -ffast-math -O2
