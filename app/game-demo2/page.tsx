"use client";

import { Suspense, useEffect, useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
import { ContactShadows, useAnimations, useGLTF } from "@react-three/drei";
import * as THREE from "three";
import Link from "next/link";
import MiniDashboard from "../components/mini-dashboard";

const SOLDIER_SCALE = 1.8 / 184.81;
const SOLDIER_Y_OFFSET = 91.9 * SOLDIER_SCALE;
const CROSSFADE_DURATION = 0.2;
const MOVE_SPEED = 2.6;
const TURN_SPEED = 10;
const FLOATING_ORIGIN_THRESHOLD = 5000;
const FLOATING_ORIGIN_THRESHOLD_SQ = FLOATING_ORIGIN_THRESHOLD * FLOATING_ORIGIN_THRESHOLD;
const WORLD_SHIFT = new THREE.Vector3();
const NPC_SPAWN_OFFSET = new THREE.Vector3(0.9, 0, -0.9);

function LoadingFallback() {
  return (
    <mesh position={[0, 1, 0]}>
      <boxGeometry args={[0.45, 1.9, 0.4]} />
      <meshStandardMaterial color="#b6ab95" wireframe />
    </mesh>
  );
}

type SoldierAction = "Idle" | "Shoot" | "Run" | "Running";

function SoldierIdle({
  isShooting,
  keysRef,
  playerPositionRef,
}: {
  isShooting: boolean;
  keysRef: React.RefObject<Set<string>>;
  playerPositionRef: React.RefObject<THREE.Vector3>;
}) {
  const wrapper = useRef<THREE.Group>(null);
  const root = useRef<THREE.Group>(null);
  const { scene, animations } = useGLTF("/solderv2_gametest.glb");
  const { actions } = useAnimations(animations, root);
  const currentAction = useRef<SoldierAction>("Idle");

  const getRunActionName = (): "Run" | "Running" | null => {
    if (actions.Run) return "Run";
    if (actions.Running) return "Running";
    return null;
  };

  useFrame((_, delta) => {
    if (!wrapper.current || !root.current) return;

    const keys = keysRef.current;
    const w = keys.has("w");
    const a = keys.has("a");
    const s = keys.has("s");
    const d = keys.has("d");
    const moving = w || a || s || d;

    if (moving) {
      const dx = (a ? 1 : 0) - (d ? 1 : 0);
      const dz = (w ? 1 : 0) - (s ? 1 : 0);
      const length = Math.hypot(dx, dz) || 1;

      wrapper.current.position.x += (dx / length) * MOVE_SPEED * delta;
      wrapper.current.position.z += (dz / length) * MOVE_SPEED * delta;

      const targetRotation = Math.atan2(dx / length, dz / length) + Math.PI;
      let rotationDelta = targetRotation - root.current.rotation.y;
      while (rotationDelta > Math.PI) rotationDelta -= Math.PI * 2;
      while (rotationDelta < -Math.PI) rotationDelta += Math.PI * 2;

      root.current.rotation.y += rotationDelta * Math.min(TURN_SPEED * delta, 1);
    }

    if (
      wrapper.current.position.x * wrapper.current.position.x +
        wrapper.current.position.z * wrapper.current.position.z >
      FLOATING_ORIGIN_THRESHOLD_SQ
    ) {
      WORLD_SHIFT.set(wrapper.current.position.x, 0, wrapper.current.position.z);

      for (const child of _.scene.children) {
        child.position.sub(WORLD_SHIFT);
      }
      _.camera.position.sub(WORLD_SHIFT);
    }

    const runActionName = getRunActionName();
    const nextAction: SoldierAction = isShooting
      ? "Shoot"
      : moving && runActionName
        ? runActionName
        : "Idle";

    if (nextAction !== currentAction.current) {
      actions[currentAction.current]?.fadeOut(CROSSFADE_DURATION);
      actions[nextAction]?.reset().fadeIn(CROSSFADE_DURATION).play();
      currentAction.current = nextAction;
    }

    playerPositionRef.current.set(
      wrapper.current.position.x,
      wrapper.current.position.y,
      wrapper.current.position.z,
    );
  });

  useEffect(() => {
    scene.traverse((child) => {
      const mesh = child as THREE.Mesh;
      if (mesh.isMesh) {
        mesh.castShadow = true;
        mesh.receiveShadow = true;
      }
    });
  }, [scene]);

  useEffect(() => {
    const idleAction = actions.Idle;
    idleAction?.reset().fadeIn(CROSSFADE_DURATION).play();
    currentAction.current = "Idle";

    return () => {
      idleAction?.fadeOut(CROSSFADE_DURATION);
    };
  }, [actions]);

  return (
    <group ref={wrapper} position={[0, SOLDIER_Y_OFFSET, 0]} scale={SOLDIER_SCALE}>
      <group ref={root} rotation={[0, Math.PI, 0]}>
        <primitive object={scene} />
      </group>
    </group>
  );
}

function PassiveSoldier({
  playerPositionRef,
}: {
  playerPositionRef: React.RefObject<THREE.Vector3>;
}) {
  const wrapper = useRef<THREE.Group>(null);
  const root = useRef<THREE.Group>(null);
  const { scene, animations } = useGLTF("/solderv3_gametest.glb");
  const { actions } = useAnimations(animations, root);

  useEffect(() => {
    scene.traverse((child) => {
      const mesh = child as THREE.Mesh;
      if (mesh.isMesh) {
        mesh.castShadow = true;
        mesh.receiveShadow = true;
      }
    });
  }, [scene]);

  useEffect(() => {
    const idleAction = actions.Idle;
    idleAction?.reset().fadeIn(CROSSFADE_DURATION).play();

    return () => {
      idleAction?.fadeOut(CROSSFADE_DURATION);
    };
  }, [actions]);

  useEffect(() => {
    if (!wrapper.current || !root.current) return;

    const playerPosition = playerPositionRef.current;
    const lookTarget = new THREE.Vector3(
      playerPosition.x,
      wrapper.current.position.y,
      playerPosition.z,
    );

    root.current.lookAt(lookTarget);
    root.current.rotateY(Math.PI);
  }, [playerPositionRef]);

  return (
    <group
      ref={wrapper}
      position={[
        NPC_SPAWN_OFFSET.x,
        SOLDIER_Y_OFFSET,
        NPC_SPAWN_OFFSET.z,
      ]}
      scale={SOLDIER_SCALE}
    >
      <group ref={root}>
        <primitive object={scene} />
      </group>
    </group>
  );
}

function StudioBackdrop() {
  return (
    <>
      <mesh rotation={[-Math.PI / 2, 0, 0]} position={[0, 0, 0]} receiveShadow>
        <planeGeometry args={[24, 24]} />
        <shadowMaterial opacity={0.14} />
      </mesh>

      <mesh position={[0, -0.01, -3]} rotation={[-Math.PI / 2, 0, 0]} receiveShadow>
        <planeGeometry args={[40, 40]} />
        <meshStandardMaterial color="#f3f1ec" roughness={1} />
      </mesh>

      <mesh position={[0, 6, -8]}>
        <planeGeometry args={[40, 24]} />
        <meshStandardMaterial color="#f7f5f1" roughness={1} />
      </mesh>
    </>
  );
}

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

export default function GameDemo2Page() {
  const [isShooting, setIsShooting] = useState(false);
  const [dashOpen, setDashOpen] = useState(true);
  const keysRef = useRef<Set<string>>(new Set());
  const playerPositionRef = useRef(new THREE.Vector3(0, SOLDIER_Y_OFFSET, 0));

  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      const key = event.key.toLowerCase();
      if (!["w", "a", "s", "d", "f"].includes(key)) return;

      event.preventDefault();
      keysRef.current.add(key);

      if (key === "f") {
        setIsShooting(true);
      }
    };

    const onKeyUp = (event: KeyboardEvent) => {
      const key = event.key.toLowerCase();
      if (!["w", "a", "s", "d", "f"].includes(key)) return;

      keysRef.current.delete(key);

      if (key === "f") {
        setIsShooting(false);
      }
    };

    window.addEventListener("keydown", onKeyDown);
    window.addEventListener("keyup", onKeyUp);

    return () => {
      window.removeEventListener("keydown", onKeyDown);
      window.removeEventListener("keyup", onKeyUp);
    };
  }, []);

  return (
    <div
      style={{
        width: "100vw",
        height: "100vh",
        overflow: "hidden",
        background:
          "radial-gradient(circle at 50% 35%, #fbfaf8 0%, #f3f1ec 48%, #e8e3da 100%)",
      }}
    >
      <Canvas
        shadows
        dpr={[1, 2]}
        camera={{ position: [0, 4.1, 5.4], fov: 28, near: 0.1, far: 100 }}
        onCreated={({ camera }) => {
          camera.lookAt(0, 1.1, 0);
        }}
      >
        <color attach="background" args={["#f4f1eb"]} />
        <fog attach="fog" args={["#f4f1eb", 12, 24]} />

        <hemisphereLight args={["#fffaf2", "#d9d1c3", 1.4]} />
        <ambientLight intensity={0.75} />
        <directionalLight
          castShadow
          intensity={2}
          position={[4.5, 10, 6]}
          shadow-mapSize={[2048, 2048]}
          shadow-camera-near={1}
          shadow-camera-far={30}
          shadow-camera-left={-6}
          shadow-camera-right={6}
          shadow-camera-top={6}
          shadow-camera-bottom={-6}
        />
        <directionalLight intensity={0.7} position={[-5, 4, -2]} color="#efe5d5" />

        <StudioBackdrop />

        <Suspense fallback={<LoadingFallback />}>
          <SoldierIdle
            isShooting={isShooting}
            keysRef={keysRef}
            playerPositionRef={playerPositionRef}
          />
          <PassiveSoldier playerPositionRef={playerPositionRef} />
        </Suspense>

        <ContactShadows
          position={[0, 0.001, 0]}
          opacity={0.36}
          scale={8}
          blur={2.6}
          far={3.2}
          color="#8e8476"
        />
      </Canvas>

      <div style={{ position: "fixed", top: 24, left: 24, zIndex: 50 }}>
        <Link href="/" style={btnStyle}>← Dashboard</Link>
      </div>

      {!dashOpen && (
        <button
          onClick={() => setDashOpen(true)}
          style={{ ...btnStyle, position: "fixed", top: 24, right: 32, zIndex: 50 }}
        >
          Open Mini-Dashboard
        </button>
      )}
      <MiniDashboard open={dashOpen} onClose={() => setDashOpen(false)} />
    </div>
  );
}

useGLTF.preload("/solderv2_gametest.glb");
useGLTF.preload("/solderv3_gametest.glb");
