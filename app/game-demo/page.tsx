"use client";
import React, { Suspense, useEffect, useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
import { useGLTF } from "@react-three/drei";
import * as THREE from "three";
import Link from "next/link";
import MiniDashboard from "../components/mini-dashboard";

/* ---- Controller constants ---- */
// GLB measured: model height 184.81 units, root-node Y = −91.90
const SOLDIER_SCALE  = 1.8 / 184.81;          // ≈ 0.00974 — 1.8 world-unit height
const SOLDIER_Y_OFF  = 91.90 * SOLDIER_SCALE;  // ≈ 0.895  — lifts feet to y=0

const MOVE_SPEED         = 4;
const ROTATE_SPEED       = 10;
const CROSSFADE_DURATION = 0.2;
const CAM_OFFSET         = new THREE.Vector3(0, 4, 7);   // aerial-behind
const CAM_LOOK_OFFSET    = new THREE.Vector3(0, 0.9, 0); // aim at torso
const CAM_SMOOTH         = 8;

/* Module-level dummy — reused every frame, zero GC pressure */
const _camDummy = new THREE.Object3D();

type AnimState = "Idle" | "Run" | "Shoot";

/* ---- Load-error boundary ---- */
class LoadErrorBoundary extends React.Component<
  { children: React.ReactNode },
  { error: Error | null }
> {
  state = { error: null };
  static getDerivedStateFromError(e: Error) { return { error: e }; }
  componentDidCatch(e: Error) {
    console.error("[GLB] Failed to load /solderv2_gametest.glb:", e.message);
  }
  render() {
    return this.state.error ? <LoadErrorFallback /> : this.props.children;
  }
}

function LoadErrorFallback() {
  return (
    <mesh position={[0, 0.9, 0]}>
      <boxGeometry args={[0.4, 1.8, 0.3]} />
      <meshStandardMaterial color="#cc4444" wireframe />
    </mesh>
  );
}

function SoldierFallback() {
  return (
    <mesh position={[0, 0.9, 0]}>
      <boxGeometry args={[0.4, 1.8, 0.3]} />
      <meshStandardMaterial color="#4466aa" wireframe />
    </mesh>
  );
}

/* ---- Soldier ---- */
interface SoldierProps {
  keysRef: React.RefObject<Set<string>>;
  resetCount: number;
}

function Soldier({ keysRef, resetCount }: SoldierProps) {
  const group        = useRef<THREE.Group>(null!);
  const mixerRef     = useRef<THREE.AnimationMixer | null>(null);
  const actionsRef   = useRef<Record<AnimState, THREE.AnimationAction | null>>({
    Idle: null, Run: null, Shoot: null,
  });
  const currentAnim  = useRef<AnimState>("Idle");
  const cameraReady  = useRef(false);

  const { scene, animations } = useGLTF("/solderv2_gametest.glb");

  /* 1 — Enable shadows on every mesh in the model */
  useEffect(() => {
    scene.traverse(child => {
      const mesh = child as THREE.Mesh;
      if (mesh.isMesh) {
        mesh.castShadow    = true;
        mesh.receiveShadow = true;
      }
    });
  }, [scene]);

  /* 2 — AnimationMixer setup */
  useEffect(() => {
    const root  = group.current;
    const mixer = new THREE.AnimationMixer(root);
    mixerRef.current = mixer;

    console.log("[GLB] Animation clips found:", animations.map(a => a.name));

    for (const clip of animations) {
      const key = clip.name as AnimState;
      if (key in actionsRef.current) {
        actionsRef.current[key] = mixer.clipAction(clip);
      } else {
        console.warn(`[GLB] Unexpected clip "${clip.name}" — expected: Idle, Run, Shoot`);
      }
    }

    const bound = (Object.keys(actionsRef.current) as AnimState[])
      .filter(k => actionsRef.current[k] !== null);
    console.log("[GLB] Actions bound:", bound);

    actionsRef.current.Idle?.play();
    currentAnim.current = "Idle";

    return () => {
      mixer.stopAllAction();
      mixer.uncacheRoot(root);
      mixerRef.current = null;
    };
  }, [animations]);

  /* 3 — Reset */
  useEffect(() => {
    if (resetCount === 0) return;
    group.current.position.set(0, 0, 0);
    group.current.rotation.set(0, 0, 0);
    cameraReady.current = false; // re-snap camera to new position
    const mixer = mixerRef.current;
    if (mixer) {
      mixer.stopAllAction();
      actionsRef.current.Idle?.reset().play();
      currentAnim.current = "Idle";
    }
  }, [resetCount]);

  /* 4 — Frame loop */
  useFrame((state, delta) => {
    if (!group.current || !mixerRef.current) return;

    function readInput() {
      const keys = keysRef.current;
      const w = keys.has("w"), a = keys.has("a"), s = keys.has("s"), d = keys.has("d");
      return {
        moving:   w || a || s || d,
        shooting: keys.has("f"),
        dx: (d ? 1 : 0) - (a ? 1 : 0),
        dz: (s ? 1 : 0) - (w ? 1 : 0),
      };
    }

    function updateMovement(moving: boolean, shooting: boolean, dx: number, dz: number) {
      if (!moving || shooting) return;
      const len = Math.sqrt(dx * dx + dz * dz) || 1;
      const targetAngle = Math.atan2(dx / len, dz / len) + Math.PI;
      let diff = targetAngle - group.current.rotation.y;
      while (diff >  Math.PI) diff -= 2 * Math.PI;
      while (diff < -Math.PI) diff += 2 * Math.PI;
      group.current.rotation.y += diff * Math.min(ROTATE_SPEED * delta, 1);
      group.current.position.x += (dx / len) * MOVE_SPEED * delta;
      group.current.position.z += (dz / len) * MOVE_SPEED * delta;
    }

    function updateAnimations(moving: boolean, shooting: boolean) {
      mixerRef.current!.update(delta);
      const target: AnimState = shooting ? "Shoot" : moving ? "Run" : "Idle";
      if (target === currentAnim.current) return;
      actionsRef.current[currentAnim.current]?.fadeOut(CROSSFADE_DURATION);
      actionsRef.current[target]?.reset().fadeIn(CROSSFADE_DURATION).play();
      currentAnim.current = target;
    }

    function updateCamera() {
      // First frame after load/reset: snap instantly, then lerp
      const t = cameraReady.current ? 1 - Math.exp(-CAM_SMOOTH * delta) : 1;
      cameraReady.current = true;

      // group.current is the inner (movement) group; add outer Y-offset for world position
      const worldPos = group.current.position.clone();
      worldPos.y += SOLDIER_Y_OFF;

      const desiredPos = worldPos.clone().add(CAM_OFFSET);
      state.camera.position.lerp(desiredPos, t);

      const lookTarget = worldPos.clone().add(CAM_LOOK_OFFSET);
      _camDummy.position.copy(state.camera.position);
      _camDummy.lookAt(lookTarget);
      state.camera.quaternion.slerp(_camDummy.quaternion, t);
    }

    const { moving, shooting, dx, dz } = readInput();
    updateMovement(moving, shooting, dx, dz);
    updateAnimations(moving, shooting);
    updateCamera();
  });

  /* Outer group: static scale + Y-offset applied before first render
     Inner group (ref): WASD movement (XZ) and rotation only             */
  return (
    <group scale={SOLDIER_SCALE} position-y={SOLDIER_Y_OFF}>
      <group ref={group}>
        <primitive object={scene} />
      </group>
    </group>
  );
}

/* ---- Ground ---- */
function Ground() {
  return (
    <mesh rotation={[-Math.PI / 2, 0, 0]} receiveShadow>
      <planeGeometry args={[60, 60]} />
      <meshStandardMaterial color="#c4c4c4" />
    </mesh>
  );
}

/* ---- Overlay button style ---- */
const btnStyle: React.CSSProperties = {
  padding: "8px 16px",
  borderRadius: 999,
  border: "1px solid rgba(0,0,0,0.12)",
  background: "rgba(255,255,255,0.88)",
  color: "#14130F",
  fontFamily: "'Inter', system-ui, sans-serif",
  fontSize: 13,
  fontWeight: 500,
  cursor: "pointer",
  boxShadow: "0 2px 12px rgba(0,0,0,0.15)",
  backdropFilter: "blur(8px)",
  WebkitBackdropFilter: "blur(8px)",
  textDecoration: "none",
  display: "inline-flex",
  alignItems: "center",
  gap: 6,
};

/* ---- Page ---- */
export default function GameDemoPage() {
  const keysRef = useRef<Set<string>>(new Set());
  const [resetCount, setResetCount] = useState(0);
  const [dashOpen, setDashOpen] = useState(true);

  useEffect(() => {
    const onDown = (e: KeyboardEvent) => {
      const tag = (e.target as HTMLElement).tagName;
      if (tag === "INPUT" || tag === "SELECT" || tag === "TEXTAREA") return;
      const k = e.key.toLowerCase();
      if (["w", "a", "s", "d", "f"].includes(k)) e.preventDefault();
      keysRef.current.add(k);
    };
    const onUp = (e: KeyboardEvent) => {
      keysRef.current.delete(e.key.toLowerCase());
    };
    window.addEventListener("keydown", onDown);
    window.addEventListener("keyup", onUp);
    return () => {
      window.removeEventListener("keydown", onDown);
      window.removeEventListener("keyup", onUp);
    };
  }, []);

  return (
    <div style={{ width: "100vw", height: "100vh", position: "relative", overflow: "hidden", background: "#c4c4c4" }}>
      <Canvas
        shadows
        camera={{ position: [0, 5, 8], fov: 50, near: 0.1, far: 200 }}
        onCreated={({ camera }) => camera.lookAt(0, 0.9, 0)}
        style={{ width: "100%", height: "100%" }}
      >
        <color attach="background" args={["#c4c4c4"]} />

        <ambientLight intensity={0.8} />
        <directionalLight
          position={[4, 10, 6]}
          intensity={1.6}
          castShadow
          shadow-mapSize={[2048, 2048]}
          shadow-camera-near={0.5}
          shadow-camera-far={100}
          shadow-camera-left={-10}
          shadow-camera-right={10}
          shadow-camera-top={10}
          shadow-camera-bottom={-10}
        />
        <directionalLight position={[-4, 6, -4]} intensity={0.6} />

        <LoadErrorBoundary>
          <Suspense fallback={<SoldierFallback />}>
            <Soldier keysRef={keysRef} resetCount={resetCount} />
          </Suspense>
        </LoadErrorBoundary>

        <Ground />
      </Canvas>

      {/* Top-left overlay */}
      <div style={{ position: "fixed", top: 24, left: 24, display: "flex", gap: 10, zIndex: 50 }}>
        <Link href="/" style={btnStyle}>
          ← Dashboard
        </Link>
        <button style={btnStyle} onClick={() => setResetCount(c => c + 1)}>
          Reset
        </button>
      </div>

      {/* Controls hint */}
      <div style={{
        position: "fixed",
        bottom: 24,
        left: "50%",
        transform: "translateX(-50%)",
        zIndex: 50,
        color: "rgba(20,19,15,0.4)",
        fontSize: 12,
        fontFamily: "'JetBrains Mono', monospace",
        letterSpacing: "0.05em",
        pointerEvents: "none",
        userSelect: "none",
      }}>
        WASD — Run &nbsp;·&nbsp; F — Shoot
      </div>

      {/* Mini dashboard toggle */}
      {!dashOpen && (
        <button
          onClick={() => setDashOpen(true)}
          style={{ ...btnStyle, position: "fixed", top: 24, right: 32, zIndex: 50 }}
        >
          Open Dashboard
        </button>
      )}
      <MiniDashboard open={dashOpen} onClose={() => setDashOpen(false)} />
    </div>
  );
}

useGLTF.preload("/solderv2_gametest.glb");
