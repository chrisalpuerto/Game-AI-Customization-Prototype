# Game AI Customization UI Mockup

NOTE: This mockup is in Next.js (built on react) using HTML, TS etc.
This needs to eventually and will be migrated to a Lua UI framework to be compatible for Roblox


## Getting Started

First, install dependencies
```npm install```
or whichever dependency installation command your OS uses for npm stuff

Next, run the development server:

```bash
npm run dev
# or
yarn dev
# or
pnpm dev
# or
bun dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser to see the result.

You can start editing the page by modifying `app/page.tsx`. The page auto-updates as you edit the file.

## Getting Started on C++ Engine Prototype

The headless C++ simulation lives in `game_cpp/src2` and streams world snapshots to the Next.js frontend over WebSocket.

### 1. Install C++ build dependencies

On macOS with Homebrew:

```bash
brew install cmake boost
```

### 2. Build the WebSocket simulation server

```bash
cd game_cpp/src2
cmake -S . -B build
cmake --build build
```

### 3. Start the C++ WebSocket server

```bash
./build/behavior_driven_cells_ws
```

The simulation server listens on:

```text
ws://127.0.0.1:5001
```

### 4. Start the Next.js app

From the repo root:

```bash
npm install
npm run dev
```

### 5. Open the simulation page

Visit:

```text
http://localhost:3000/game-demo3
```

`/game-demo3` connects to the C++ WebSocket server and renders the live simulation.

### 6. Interact with the simulation

- Click anywhere on the canvas in `/game-demo3` to spawn food.
- The frontend sends a WebSocket message to the C++ server.
- The C++ world spawns food at that position, similar to the original desktop prototype's left-click behavior.

### Restarting the WebSocket server

If you stop the server, restart it with:

```bash
cd game_cpp/src2
./build/behavior_driven_cells_ws
```

If you changed the C++ source first, rebuild before starting it again:

```bash
cd game_cpp/src2
cmake --build build
./build/behavior_driven_cells_ws
```
