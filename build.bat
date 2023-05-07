set CFLAGS=-Wall -Wextra -pedantic -std=c99 -I. -O2 -fno-builtin --target=wasm32 -mbulk-memory --no-standard-libraries "-Wl,--no-entry"

set EXPORTS="-Wl,--export=draw"^
 "-Wl,--export=__heap_base"^
 "-Wl,--export=resize"^
 "-Wl,--export=mouseDown"^
 "-Wl,--export=mouseUp"^
 "-Wl,--export=mouseMove"^
 "-Wl,--export=keyDown"^
 "-Wl,--export=contextMenu"

clang %CFLAGS% %EXPORTS% "-Wl,--allow-undefined" -g -o.\build\automota.wasm -DVC_PLATFORM=VC_WASM_PLATFORM main.c
