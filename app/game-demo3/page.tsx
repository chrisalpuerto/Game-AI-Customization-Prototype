"use client";

import { useEffect, useRef, useState } from "react";

const WS_URL = process.env.NEXT_PUBLIC_SIM_WS_URL ?? "ws://127.0.0.1:5001";
const GRID_SPACING = 32;
const CONE_LENGTH = 520;
const CONE_HALF_ANGLE = 50 * (Math.PI / 180);

type ConnectionStatus = "connecting" | "connected" | "disconnected" | "error";

type SnapshotEntity = {
  id: number;
  type: string;
  x: number;
  y: number;
  vx: number;
  vy: number;
  alive: boolean;
};

type SnapshotWorld = {
  width: number;
  height: number;
  cells: SnapshotEntity[];
  food: SnapshotEntity[];
  barriers: SnapshotEntity[];
};

type SnapshotMessage = {
  type: "snapshot";
  time: number;
  world: SnapshotWorld;
};

function normalize2D(x: number, y: number) {
  const length = Math.hypot(x, y);
  if (length <= 0.0001) {
    return { x: 1, y: 0 };
  }
  return { x: x / length, y: y / length };
}

function rotate2D(x: number, y: number, angle: number) {
  const c = Math.cos(angle);
  const s = Math.sin(angle);
  return {
    x: x * c - y * s,
    y: x * s + y * c,
  };
}

function drawBackground(ctx: CanvasRenderingContext2D, width: number, height: number) {
  ctx.fillStyle = "#7a4a14";
  ctx.fillRect(0, 0, width, height);

  ctx.fillStyle = "rgba(164, 112, 40, 0.42)";
  const bandWidth = width / 3;
  ctx.fillRect(bandWidth, 0, bandWidth, height);

  ctx.fillStyle = "#2041b6";
  ctx.fillRect(0, height - 22, width, 22);
}

function drawGrid(ctx: CanvasRenderingContext2D, width: number, height: number) {
  ctx.fillStyle = "#1227ff";
  for (let y = 10; y < height; y += GRID_SPACING) {
    for (let x = 10; x < width; x += GRID_SPACING) {
      ctx.beginPath();
      ctx.arc(x, y, 3, 0, Math.PI * 2);
      ctx.fill();
    }
  }
}

function drawBarrier(ctx: CanvasRenderingContext2D, entity: SnapshotEntity, sx: number, sy: number) {
  const x = entity.x * sx;
  const y = entity.y * sy;
  const length = 120;

  ctx.strokeStyle = "#111111";
  ctx.lineWidth = 4;
  ctx.beginPath();
  ctx.moveTo(x - length * 0.35, y - length);
  ctx.bezierCurveTo(x - 10, y - 34, x + 12, y + 24, x + length * 0.25, y + length);
  ctx.stroke();
}

function drawFood(ctx: CanvasRenderingContext2D, entity: SnapshotEntity, sx: number, sy: number) {
  const x = entity.x * sx;
  const y = entity.y * sy;

  ctx.beginPath();
  ctx.arc(x, y, 10, 0, Math.PI * 2);
  ctx.fillStyle = "#8cf04b";
  ctx.fill();
  ctx.strokeStyle = "#1f1f1f";
  ctx.lineWidth = 2;
  ctx.stroke();
}

function drawSightCone(
  ctx: CanvasRenderingContext2D,
  entity: SnapshotEntity,
  sx: number,
  sy: number,
  highlighted: boolean,
) {
  const cx = entity.x * sx;
  const cy = entity.y * sy;
  const direction = normalize2D(entity.vx, entity.vy);
  const left = rotate2D(direction.x, direction.y, -CONE_HALF_ANGLE);
  const right = rotate2D(direction.x, direction.y, CONE_HALF_ANGLE);
  const radius = CONE_LENGTH * Math.max(sx, sy);

  ctx.strokeStyle = highlighted ? "#f0ea60" : "rgba(240, 234, 96, 0.88)";
  ctx.lineWidth = highlighted ? 3 : 2;

  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.lineTo(cx + left.x * radius, cy + left.y * radius);
  ctx.moveTo(cx, cy);
  ctx.lineTo(cx + right.x * radius, cy + right.y * radius);
  ctx.stroke();

  const startAngle = Math.atan2(left.y, left.x);
  const endAngle = Math.atan2(right.y, right.x);
  ctx.beginPath();
  ctx.arc(cx, cy, radius, startAngle, endAngle);
  ctx.stroke();

  const centerLeg = {
    x: cx + direction.x * radius * 0.92,
    y: cy + direction.y * radius * 0.92,
  };
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.lineTo(centerLeg.x, centerLeg.y);
  ctx.stroke();
}

function drawCell(
  ctx: CanvasRenderingContext2D,
  entity: SnapshotEntity,
  sx: number,
  sy: number,
  highlighted: boolean,
) {
  const x = entity.x * sx;
  const y = entity.y * sy;
  const radius = highlighted ? 20 : 18;

  drawSightCone(ctx, entity, sx, sy, highlighted);

  ctx.beginPath();
  ctx.arc(x, y, radius, 0, Math.PI * 2);
  ctx.fillStyle = highlighted ? "#f2472d" : "#8cf04b";
  ctx.fill();
  ctx.strokeStyle = "#3b2508";
  ctx.lineWidth = 3;
  ctx.stroke();
}

function renderWorld(ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement, world: SnapshotWorld) {
  const width = canvas.width;
  const height = canvas.height;
  const sx = width / Math.max(world.width, 1);
  const sy = height / Math.max(world.height, 1);

  drawBackground(ctx, width, height);
  drawGrid(ctx, width, height);

  for (const barrier of world.barriers) {
    drawBarrier(ctx, barrier, sx, sy);
  }

  for (const cell of world.cells) {
    drawSightCone(ctx, cell, sx, sy, false);
  }

  for (const food of world.food) {
    drawFood(ctx, food, sx, sy);
  }

  const highlightedCellId = world.cells.length > 1 ? world.cells[1].id : world.cells[0]?.id;
  for (const cell of world.cells) {
    drawCell(ctx, cell, sx, sy, cell.id === highlightedCellId);
  }
}

export default function GameDemo3Page() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const worldRef = useRef<SnapshotWorld | null>(null);
  const animationRef = useRef<number | null>(null);
  const socketRef = useRef<WebSocket | null>(null);

  const [status, setStatus] = useState<ConnectionStatus>("connecting");
  const [time, setTime] = useState(0);
  const [counts, setCounts] = useState({ cells: 0, food: 0, barriers: 0 });
  const [hasSnapshot, setHasSnapshot] = useState(false);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const resize = () => {
      canvas.width = canvas.clientWidth;
      canvas.height = canvas.clientHeight;
    };

    resize();
    const observer = new ResizeObserver(resize);
    observer.observe(canvas);

    return () => observer.disconnect();
  }, []);

  useEffect(() => {
    try {
      const socket = new WebSocket(WS_URL);
      socketRef.current = socket;

      socket.onopen = () => setStatus("connected");
      socket.onerror = () => setStatus("error");
      socket.onclose = () => setStatus("disconnected");

      socket.onmessage = (event) => {
        try {
          const message = JSON.parse(event.data as string) as SnapshotMessage;
          if (message.type !== "snapshot") return;

          worldRef.current = message.world;
          setHasSnapshot(true);
          setTime(message.time);
          setCounts({
            cells: message.world.cells.length,
            food: message.world.food.length,
            barriers: message.world.barriers.length,
          });
        } catch {
          // Ignore malformed packets.
        }
      };

      return () => {
        socketRef.current = null;
        socket.close();
      };
    } catch {
      const timeoutId = window.setTimeout(() => setStatus("error"), 0);
      return () => window.clearTimeout(timeoutId);
    }
  }, []);

  const handleCanvasClick = (event: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    const world = worldRef.current;
    const socket = socketRef.current;
    if (!canvas || !world || !socket || socket.readyState !== WebSocket.OPEN) {
      return;
    }

    const rect = canvas.getBoundingClientRect();
    const x = ((event.clientX - rect.left) / rect.width) * world.width;
    const y = ((event.clientY - rect.top) / rect.height) * world.height;

    socket.send(
      JSON.stringify({
        type: "spawn_food",
        x: Math.round(x),
        y: Math.round(y),
      }),
    );
  };

  useEffect(() => {
    const draw = () => {
      const canvas = canvasRef.current;
      const world = worldRef.current;
      if (canvas) {
        const context = canvas.getContext("2d");
        if (context) {
          if (world) {
            renderWorld(context, canvas, world);
          } else {
            drawBackground(context, canvas.width, canvas.height);
            drawGrid(context, canvas.width, canvas.height);
          }
        }
      }

      animationRef.current = requestAnimationFrame(draw);
    };

    animationRef.current = requestAnimationFrame(draw);
    return () => {
      if (animationRef.current !== null) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, []);

  return (
    <div
      style={{
        width: "100vw",
        height: "100vh",
        overflow: "hidden",
        background: "#7a4a14",
        position: "relative",
      }}
    >
      <canvas
        ref={canvasRef}
        onClick={handleCanvasClick}
        style={{
          width: "100%",
          height: "100%",
          display: "block",
          imageRendering: "auto",
          cursor: status === "connected" ? "crosshair" : "default",
        }}
      />

      <div
        style={{
          position: "fixed",
          top: 18,
          left: 18,
          zIndex: 20,
          padding: "10px 14px",
          borderRadius: 10,
          color: "#f6efcf",
          background: "rgba(40, 22, 5, 0.58)",
          border: "1px solid rgba(247, 232, 140, 0.18)",
          fontFamily: "var(--font-geist-mono), monospace",
          fontSize: 12,
          lineHeight: 1.65,
          letterSpacing: "0.04em",
          backdropFilter: "blur(10px)",
          WebkitBackdropFilter: "blur(10px)",
        }}
      >
        <div>STATUS {status.toUpperCase()}</div>
        <div>TIME {time.toFixed(1)}s</div>
        <div>CELLS {counts.cells}</div>
        <div>FOOD {counts.food}</div>
        <div>BARRIERS {counts.barriers}</div>
        <div>CLICK TO PLACE FOOD</div>
      </div>

      {status !== "connected" && !hasSnapshot && (
        <div
          style={{
            position: "absolute",
            inset: 0,
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            color: "#f6efcf",
            fontFamily: "var(--font-geist-mono), monospace",
            fontSize: 14,
            background: "rgba(25, 12, 3, 0.22)",
            letterSpacing: "0.05em",
          }}
        >
          Waiting for simulation server at {WS_URL}
        </div>
      )}
    </div>
  );
}
