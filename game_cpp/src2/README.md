# `src2` headless refactor

This directory is a headless refactor target for the original Win32 desktop prototype in `game_cpp/src`.

Current scope:
- removes `Windows.h`, GDI drawing, and the message loop
- keeps portable math and simulation primitives
- restores original barrier collision and land simulation semantics in portable form
- provides a headless `World` with cells, food, barriers, land, and environment boxes
- adds a console `main.cpp` demo plus a `snapshot()` API
- includes a separate `server.cpp` WebSocket entrypoint for streaming snapshots to a frontend

Intentional limitations in this first pass:
- `plant` and `spore` are not ported yet
- advanced `cell` cognition such as push/observe/analyze is still deferred
- snapshots are serialized manually to JSON for the WebSocket server

Build example:

```bash
cd game_cpp/src2
cmake -S . -B build
cmake --build build
./build/behavior_driven_cells_headless
./build/behavior_driven_cells_parity_tests
```

WebSocket server dependency install on macOS/Homebrew:

```bash
brew install cmake boost
```

Then rebuild and run the socket server:

```bash
cd game_cpp/src2
cmake -S . -B build
cmake --build build
./build/behavior_driven_cells_ws
```

The server listens on:

```text
ws://127.0.0.1:5001
```

Example frontend message shape:

```json
{
  "type": "snapshot",
  "time": 12.3,
  "world": {
    "width": 1280,
    "height": 720,
    "cells": [],
    "food": [],
    "barriers": []
  }
}
```
