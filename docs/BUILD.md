# Building the WASM demo

This demo compiles `CLI-Emulator/main.c` (the exact same SimpleRISC parser
and executor the native CLI uses) to WebAssembly with Emscripten, driven by
a plain HTML/JS UI in this folder. The WASM-specific code is a small block
at the bottom of `main.c`, guarded by `#ifdef __EMSCRIPTEN__` -- it never
touches or affects the native build.

## One-time setup: install Emscripten (skip if you already have `emcc`)

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

That last line (`source ./emsdk_env.sh`) only affects your current shell
session -- run it again (or add it to your `~/.zshrc`) in any new terminal
where you want to use `emcc`.

## Build

From the repo root (`CLI-Emulator/`):

```bash
mkdir -p docs/pkg
emcc CLI-Emulator/main.c -O2 \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s EXPORT_NAME=createInterpreterModule \
  -s EXPORTED_FUNCTIONS=_run_program,_get_reg,_get_flag,_get_mem \
  -s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
  -s ENVIRONMENT=web \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s INVOKE_RUN=0 \
  -o docs/pkg/interpreter.js
```

What the flags mean:
- `MODULARIZE=1` / `EXPORT_NAME=...`: wraps the output in a factory function
  (`createInterpreterModule()`) instead of dumping globals onto `window`.
- `EXPORT_ES6=1`: **important** -- without this, `MODULARIZE=1` alone
  produces a CommonJS-style file, not a real ES module. `app.js` uses
  `import createInterpreterModule from "./pkg/interpreter.js"`, which
  requires an actual `export default` -- only `EXPORT_ES6=1` generates that.
- `EXPORTED_FUNCTIONS`: the C functions we actually want callable from JS
  (note the leading underscore -- that's just Emscripten's C-symbol naming).
- `EXPORTED_RUNTIME_METHODS=ccall,cwrap`: exposes the helpers `app.js` uses
  to call those functions with normal JS strings/numbers.
- `INVOKE_RUN=0`: **important** -- without this, Emscripten auto-runs the
  real `main()` on load, which expects a filename in `argv[1]` and would
  crash immediately since there's no file to open in a browser.
- `ALLOW_MEMORY_GROWTH=1`: safety net in case a huge pasted program needs
  more memory than the initial allocation.

This produces `docs/pkg/interpreter.js` (glue code) and
`docs/pkg/interpreter.wasm` (the compiled binary) together.

## Run it

```bash
cd docs
python3 -m http.server 8080
```

Open `http://localhost:8080`. Try the example programs in the sidebar --
"Invalid instruction" is worth trying specifically to confirm error
handling works (it should print the error and leave the demo usable for
the next run, not freeze it).

## If the build fails

Paste me the exact error. Common ones:
- `emcc: command not found` -- you skipped/forgot `source ./emsdk_env.sh`
  in this terminal session.
- Errors mentioning `argv`/`main` -- almost certainly means `INVOKE_RUN=0`
  got dropped from the command.
- Browser console says `does not provide an export named 'default'` --
  `EXPORT_ES6=1` got dropped from the command.

## Deploying

Same as rust_spreadsheet: once `docs/` exists with a working build, GitHub
Pages can serve it directly -- Settings -> Pages -> Deploy from a branch ->
`main` -> `/docs`.
