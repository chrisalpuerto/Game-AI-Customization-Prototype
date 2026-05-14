import { spawn } from "node:child_process";
import { access } from "node:fs/promises";
import net from "node:net";
import path from "node:path";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

type SimServerState = {
  pid: number | null;
};

declare global {
  // eslint-disable-next-line no-var
  var __simServerState: SimServerState | undefined;
}

const simServerState: SimServerState = globalThis.__simServerState ?? { pid: null };
globalThis.__simServerState = simServerState;

function resolveWsTarget() {
  const wsUrl = process.env.NEXT_PUBLIC_SIM_WS_URL ?? "ws://127.0.0.1:5001";
  const url = new URL(wsUrl);
  return {
    host: url.hostname || "127.0.0.1",
    port: Number(url.port || "80"),
    url: wsUrl,
  };
}

function isPortOpen(host: string, port: number) {
  return new Promise<boolean>((resolve) => {
    const socket = new net.Socket();

    const finish = (value: boolean) => {
      socket.removeAllListeners();
      socket.destroy();
      resolve(value);
    };

    socket.setTimeout(250);
    socket.once("connect", () => finish(true));
    socket.once("timeout", () => finish(false));
    socket.once("error", () => finish(false));
    socket.connect(port, host);
  });
}

async function waitForPort(host: string, port: number, timeoutMs: number) {
  const start = Date.now();
  while (Date.now() - start < timeoutMs) {
    if (await isPortOpen(host, port)) {
      return true;
    }
    await new Promise((resolve) => setTimeout(resolve, 150));
  }
  return false;
}

export async function POST() {
  const target = resolveWsTarget();
  if (await isPortOpen(target.host, target.port)) {
    return Response.json({
      ok: true,
      alreadyRunning: true,
      pid: simServerState.pid,
      url: target.url,
    });
  }

  const src2Dir = path.join(process.cwd(), "game_cpp", "src2");
  const binaryPath = path.join(src2Dir, "build", "behavior_driven_cells_ws");

  try {
    await access(binaryPath);
  } catch {
    return Response.json(
      {
        ok: false,
        error: `Missing WebSocket binary at ${binaryPath}. Build game_cpp/src2 first.`,
      },
      { status: 500 },
    );
  }

  try {
    const child = spawn("zsh", ["-lc", "cd game_cpp/src2 && ./build/behavior_driven_cells_ws"], {
      cwd: process.cwd(),
      detached: true,
      stdio: "ignore",
      env: process.env,
      windowsHide: true,
    });

    simServerState.pid = child.pid ?? null;

    const started = await waitForPort(target.host, target.port, 5000);
    child.unref();

    if (!started) {
      return Response.json(
        {
          ok: false,
          error: `Started process${child.pid ? ` ${child.pid}` : ""}, but ${target.url} did not open in time.`,
        },
        { status: 504 },
      );
    }

    return Response.json({
      ok: true,
      alreadyRunning: false,
      pid: child.pid ?? null,
      url: target.url,
    });
  } catch (error) {
    const message = error instanceof Error ? error.message : "Unknown error";
    return Response.json(
      {
        ok: false,
        error: `Failed to start simulation server: ${message}`,
      },
      { status: 500 },
    );
  }
}
