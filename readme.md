# Automota

## Quick Start

first you need to install olive.c from https://github.com/tsoding/olive.c
```
$ curl https://raw.githubusercontent.com/tsoding/olive.c/274eb615187415bf4603c79fb4b7458ff2a15811/olive.c -o olive.c
```

then you can complile and run the program

```
$ build.bat
```

this will spit out the wasm and then if you serve the directory you can see the automota

NOTE: if you get the error `you dont have d2sm. try turning debug mode off` this means you need to either get the dwarf2sourcemap library from https://github.com/jdmichaud/dwarf-2-sourcemap and compile the typescript file or you can turn of debug mode by changing the line 
`DEBUG = true;`
at the top of `script.js` to `DEBUG = false;`