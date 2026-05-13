"use client";
import "../ai-character-studio.css";
import "./mini-dashboard.css";
import { useState, useEffect, useRef } from "react";

/* ---- Types ---- */
interface IconProps {
  d: React.ReactNode;
  size?: number;
  fill?: string;
  stroke?: string;
  sw?: number;
}
interface SliderProps {
  value: number;
  onChange: (v: number) => void;
  min?: number;
  max?: number;
  step?: number;
  ticks?: boolean;
  accent?: string;
}
interface ParamSliderProps {
  name: string;
  help?: string;
  value: number;
  onChange: (v: number) => void;
  lowLabel?: string;
  highLabel?: string;
  accent?: string;
}
type DetectionUnit = "in" | "ft" | "yd" | "mi";
interface DetectionRadiusProps {
  unit: DetectionUnit;
  setUnit: (u: DetectionUnit) => void;
  rangeMin: number;
  rangeMax: number;
  value: number;
  setValue: (v: number) => void;
  setRange: (lo: number, hi: number) => void;
}
interface ReactionTimeProps { t: number; setT: (v: number) => void; }
interface WeaponWeightProps { value: number; onChange: (v: number) => void; }
interface ToggleBarProps { options: string[]; value: string; onChange: (v: string) => void; }
interface DiffBarProps { pct: number; }
interface Preset {
  id: string; name: string; icon: React.ReactNode; desc: string; spec: string[];
}

/* ---- Icons ---- */
function Icon({ d, size = 16, fill = "none", stroke = "currentColor", sw = 1.6 }: IconProps) {
  return (
    <svg width={size} height={size} viewBox="0 0 24 24" fill={fill} stroke={stroke} strokeWidth={sw} strokeLinecap="round" strokeLinejoin="round">{d}</svg>
  );
}
const I = {
  rush:  <Icon d={<><path d="M4 12h12M12 6l6 6-6 6"/></>} />,
  shield:<Icon d={<><path d="M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6z"/></>} />,
  scout: <Icon d={<><circle cx="12" cy="12" r="3"/><path d="M2 12h3M19 12h3M12 2v3M12 19v3"/></>} />,
  ghost: <Icon d={<><path d="M5 11a7 7 0 0 1 14 0v9l-2-2-2 2-2-2-2 2-2-2-2 2-2-2z"/><circle cx="9.5" cy="11" r=".8" fill="currentColor"/><circle cx="14.5" cy="11" r=".8" fill="currentColor"/></>} />,
};

/* ---- Helpers ---- */
const UNIT_TO_M: Record<DetectionUnit, number> = { in: 0.0254, ft: 0.3048, yd: 0.9144, mi: 1609.34 };

function reactionFromT(t: number): number {
  return 50 + Math.pow(t, 1.5) * (3000 - 50);
}

/* ---- Sub-components ---- */
function Slider({ value, onChange, min = 0, max = 10, step = 0.1, ticks = true, accent = "var(--accent)" }: SliderProps) {
  const pct = ((value - min) / (max - min)) * 100;
  return (
    <div className="slider-track-wrap">
      <div className="slider-bg" />
      <div className="slider-fill" style={{ width: pct + "%", background: accent }} />
      {ticks && (
        <div className="ticks">
          {Array.from({ length: 11 }).map((_, i) => <span className="tick" key={i} />)}
        </div>
      )}
      <input className="slider" type="range" min={min} max={max} step={step} value={value}
        onChange={e => onChange(parseFloat(e.target.value))} />
    </div>
  );
}

function ParamSlider({ name, help, value, onChange, lowLabel, highLabel, accent }: ParamSliderProps) {
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">{name}</div>
        {help && <div className="param-help">{help}</div>}
      </div>
      <div className="slider-wrap">
        <Slider value={value} onChange={onChange} min={0} max={10} step={0.1} accent={accent} />
        <div className="slider-readout">{value.toFixed(1)}<span className="unit">/10</span></div>
      </div>
      {(lowLabel || highLabel) && (
        <div className="slider-extremes"><span>{lowLabel}</span><span>{highLabel}</span></div>
      )}
    </div>
  );
}

function DetectionRadius({ unit, setUnit, rangeMin, rangeMax, value, setValue, setRange }: DetectionRadiusProps) {
  const mVal = (value * UNIT_TO_M[unit]).toFixed(1);
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Detection Radius</div>
        <div className="param-help">Maximum range at which the agent can sense players and events.</div>
      </div>
      <div className="slider-wrap">
        <Slider value={value} onChange={setValue} min={rangeMin} max={rangeMax}
          step={(rangeMax - rangeMin) / 200} ticks={false} />
        <div className="slider-readout">{value.toFixed(unit === 'mi' ? 2 : 1)}<span className="unit">{unit}</span></div>
      </div>
      <div className="slider-extremes" style={{ marginTop: 4 }}>
        <span>≈ {mVal} m</span>
        <span>{unit === 'mi' ? 'long-range scout' : unit === 'in' ? 'point-blank' : 'standard sightline'}</span>
      </div>
      <div className="range-row">
        <span className="label">RANGE</span>
        <input className="num-input" type="number" value={rangeMin}
          onChange={e => setRange(parseFloat(e.target.value) || 0, rangeMax)} />
        <span className="label">→</span>
        <input className="num-input" type="number" value={rangeMax}
          onChange={e => setRange(rangeMin, parseFloat(e.target.value) || 0)} />
        <span className="label" style={{ marginLeft: 8 }}>UNIT</span>
        <select className="unit-select" value={unit} onChange={e => setUnit(e.target.value as DetectionUnit)}>
          <option value="in">inches</option>
          <option value="ft">feet</option>
          <option value="yd">yards</option>
          <option value="mi">miles</option>
        </select>
      </div>
    </div>
  );
}

function ReactionTime({ t, setT }: ReactionTimeProps) {
  const ms = reactionFromT(t);
  const display = ms < 1000 ? `${ms.toFixed(0)} ms` : `${(ms / 1000).toFixed(2)} s`;
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Reaction Time</div>
        <div className="param-help">Latency between perceiving a stimulus and acting (γ=1.5).</div>
      </div>
      <div className="slider-wrap">
        <Slider value={t} onChange={setT} min={0} max={1} step={0.001} ticks={false} />
        <div className="slider-readout">{display}</div>
      </div>
      <div className="slider-extremes"><span>50 ms — frame-perfect</span><span>3.0 s — sluggish</span></div>
    </div>
  );
}

function ToggleBar({ options, value, onChange }: ToggleBarProps) {
  const idx = options.indexOf(value);
  const w = 100 / options.length;
  return (
    <div className="toggle-bar">
      <div className="knob" style={{ left: `calc(${idx * w}% + 4px)`, width: `calc(${w}% - 8px)` }} />
      {options.map(o => (
        <button key={o} className={"toggle-opt" + (o === value ? " on" : "")} onClick={() => onChange(o)}>{o}</button>
      ))}
    </div>
  );
}

function WeaponWeight({ value, onChange }: WeaponWeightProps) {
  const pct = (value + 1) * 50;
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Weapon Preference Weight</div>
        <div className="param-help">Bias toward engagement range when picking a weapon.</div>
      </div>
      <div className="slider-wrap">
        <div className="slider-track-wrap" style={{ height: 22 }}>
          <div className="slider-bg" />
          <div style={{
            position: 'absolute', left: '50%', top: '50%',
            transform: 'translate(-50%,-50%)', width: 1, height: 14,
            background: 'var(--ink-3)', opacity: .4,
          }} />
          <div style={{
            position: 'absolute', top: '50%', height: 4, transform: 'translateY(-50%)',
            background: 'var(--ink)', borderRadius: 2,
            left: value < 0 ? `${pct}%` : '50%',
            width: `${Math.abs(value) * 50}%`,
          }} />
          <input className="slider" type="range" min={-1} max={1} step={0.01} value={value}
            onChange={e => onChange(parseFloat(e.target.value))} />
        </div>
        <div className="slider-readout">
          {value === 0 ? "neutral" : `${value > 0 ? "+" : ""}${(value * 100).toFixed(0)}%`}
        </div>
      </div>
      <div className="slider-extremes">
        <span>← Long range · sniper</span>
        <span>SMG, shotgun → Short</span>
      </div>
    </div>
  );
}

function DiffBar({ pct }: DiffBarProps) {
  return <div className="diff-bar"><span style={{ width: pct + '%' }} /></div>;
}

/* ---- Presets ---- */
const PRESETS: Preset[] = [
  { id: "rusher",   name: "Aggressive Rusher",  icon: I.rush,   desc: "Closes distance fast. High aggression, low caution.", spec: ["AGGR 9.0", "CAUT 2.0", "RANGE -0.6"] },
  { id: "tactical", name: "Tactical Soldier",   icon: I.shield, desc: "Balanced, uses cover, coordinates with squad.",        spec: ["AGGR 5.5", "AWARE 7.5", "COORD 8"] },
  { id: "survivor", name: "Cautious Survivor",  icon: I.ghost,  desc: "Avoids fights unless cornered. Resource-efficient.",   spec: ["AGGR 2.0", "DEF 9.0", "HEALTH 9"] },
  { id: "scout",    name: "Recon Scout",        icon: I.scout,  desc: "Watches, marks targets, repositions before engaging.", spec: ["AWARE 9.5", "RANGE +0.4", "COORD 6"] },
];

const SECTION_TITLES = [
  "Personality & Behavior Biases",
  "Perceptual & Performance Limits",
  "Resource & Combat Priorities",
  "Difficulty Presets",
];

/* ---- MiniDashboard ---- */
export default function MiniDashboard({ open, onClose }: { open: boolean; onClose: () => void }) {
  const [sectionIdx, setSectionIdx] = useState(0);
  const [pos, setPos] = useState({ x: 0, y: 0 });

  // Section 1
  const [aggression, setAggression] = useState(6.4);
  const [defensive, setDefensive]   = useState(3.8);
  const [tactical, setTactical]     = useState(7.2);
  const [decision, setDecision]     = useState(5.5);

  // Section 2
  const [unit, setUnit] = useState<DetectionUnit>("ft");
  const [rng, setRng]   = useState<{ min: number; max: number }>({ min: 0, max: 200 });
  const [radius, setRadius] = useState(85);
  const setRange = (lo: number, hi: number) => setRng({ min: lo, max: hi });
  useEffect(() => {
    if (radius < rng.min) setRadius(rng.min);
    if (radius > rng.max) setRadius(rng.max);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [rng.min, rng.max]);
  const [reactT, setReactT] = useState(0.32);
  const [moveSpd, setMoveSpd] = useState(6.0);

  // Section 3
  const [healthW, setHealthW]     = useState(7.5);
  const [ammoW, setAmmoW]         = useState(4.0);
  const [weaponBias, setWeaponBias] = useState(-0.25);
  const [coord, setCoord]         = useState("Squad");

  // Section 4
  const [diff, setDiff]     = useState(6.0);
  const [preset, setPreset] = useState("tactical");

  const reactMs = reactionFromT(reactT) * (1 - (diff - 5) * 0.04);
  const aware   = Math.min(10, tactical * (0.7 + diff * 0.06));
  const decQ    = Math.min(10, decision * (0.7 + diff * 0.06));

  function applyPreset(id: string) {
    setPreset(id);
    if (id === "rusher")   { setAggression(9);   setDefensive(2);   setTactical(5);   setDecision(7); setReactT(.18); setMoveSpd(8.5); setHealthW(3); setAmmoW(5); setWeaponBias(-.6);  setCoord("Lone wolf"); }
    if (id === "tactical") { setAggression(5.5); setDefensive(5.5); setTactical(7.5); setDecision(6); setReactT(.32); setMoveSpd(6);   setHealthW(7); setAmmoW(6); setWeaponBias(-.05); setCoord("Squad"); }
    if (id === "survivor") { setAggression(2);   setDefensive(9);   setTactical(6.5); setDecision(4); setReactT(.45); setMoveSpd(4.5); setHealthW(9); setAmmoW(8); setWeaponBias(.2);   setCoord("Lone wolf"); }
    if (id === "scout")    { setAggression(3.5); setDefensive(6);   setTactical(9.5); setDecision(7); setReactT(.22); setMoveSpd(7);   setHealthW(6); setAmmoW(5); setWeaponBias(.4);   setCoord("Pair"); }
  }

  // Dragging
  const isDragging = useRef(false);
  const dragStart  = useRef({ mx: 0, my: 0, px: 0, py: 0 });

  useEffect(() => {
    const onMove = (e: MouseEvent) => {
      if (!isDragging.current) return;
      setPos({
        x: dragStart.current.px + e.clientX - dragStart.current.mx,
        y: dragStart.current.py + e.clientY - dragStart.current.my,
      });
    };
    const onUp = () => { isDragging.current = false; };
    document.addEventListener("mousemove", onMove);
    document.addEventListener("mouseup", onUp);
    return () => {
      document.removeEventListener("mousemove", onMove);
      document.removeEventListener("mouseup", onUp);
    };
  }, []);

  const onHeaderMouseDown = (e: React.MouseEvent) => {
    if ((e.target as HTMLElement).closest("button")) return;
    isDragging.current = true;
    dragStart.current = { mx: e.clientX, my: e.clientY, px: pos.x, py: pos.y };
    e.preventDefault();
  };

  return (
    <div className="mini-popup-anchor" style={{ transform: `translate(${pos.x}px, ${pos.y}px)` }}>
      <div className={`mini-popup ${open ? "mini-popup--open" : "mini-popup--closed"}`}>

        {/* Header */}
        <div className="mini-popup-header" onMouseDown={onHeaderMouseDown}>
          <button
            className="mini-popup-nav-btn nav-arrow-left"
            onClick={() => setSectionIdx(i => Math.max(0, i - 1))}
            disabled={sectionIdx === 0}
            aria-label="Previous section"
          >‹</button>

          <div className="mini-popup-title-area">
            <span className="mini-popup-title">{SECTION_TITLES[sectionIdx]}</span>
            <span className="mini-popup-pager">{String(sectionIdx + 1).padStart(2, "0")} / 04</span>
          </div>

          <button
            className="mini-popup-nav-btn nav-arrow-right"
            onClick={() => setSectionIdx(i => Math.min(3, i + 1))}
            disabled={sectionIdx === 3}
            aria-label="Next section"
          >›</button>

          <button className="mini-popup-close" onClick={onClose} aria-label="Close">
            <svg width="12" height="12" viewBox="0 0 12 12" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round">
              <line x1="1" y1="1" x2="11" y2="11" />
              <line x1="11" y1="1" x2="1"  y2="11" />
            </svg>
          </button>
        </div>

        {/* Carousel body */}
        <div className="mini-popup-body">
          <div className="mini-popup-track" style={{ transform: `translateX(-${sectionIdx * 100}%)` }}>

            {/* 01 — Personality */}
            <div className="mini-popup-section">
              <div className="param-grid">
                <ParamSlider name="Aggression Level"           help="Eagerness to initiate and press attacks."               value={aggression} onChange={setAggression} lowLabel="passive"   highLabel="bloodthirsty" />
                <ParamSlider name="Defensiveness / Passiveness" help="Tendency to seek cover and avoid exposure."             value={defensive}  onChange={setDefensive}  lowLabel="reckless"  highLabel="turtle" />
                <ParamSlider name="Tactical Awareness"         help="Map reading, flanking, line-of-sight management."       value={tactical}   onChange={setTactical}   lowLabel="oblivious" highLabel="general" />
                <ParamSlider name="Decision Threshold"         help="Confidence required before committing to an action."    value={decision}   onChange={setDecision}   lowLabel="impulsive" highLabel="deliberate" />
              </div>
            </div>

            {/* 02 — Perception */}
            <div className="mini-popup-section">
              <div className="param-grid one">
                <DetectionRadius
                  unit={unit} setUnit={setUnit}
                  rangeMin={rng.min} rangeMax={rng.max} setRange={setRange}
                  value={radius} setValue={setRadius}
                />
              </div>
              <div className="param-grid">
                <ReactionTime t={reactT} setT={setReactT} />
                <ParamSlider name="Movement Speed" help="Top traversal speed multiplier." value={moveSpd} onChange={setMoveSpd} lowLabel="hobble" highLabel="sprint" />
              </div>
            </div>

            {/* 03 — Resources & Combat */}
            <div className="mini-popup-section">
              <div className="param-grid">
                <ParamSlider name="Health Importance" help="Likelihood to break engagement to heal or retreat." value={healthW} onChange={setHealthW} lowLabel="ignore"  highLabel="self-preserve" />
                <ParamSlider name="Ammo Importance"   help="Conservatism of fire — bursts vs. spray."         value={ammoW}   onChange={setAmmoW}   lowLabel="spray"   highLabel="conserve" />
                <WeaponWeight value={weaponBias} onChange={setWeaponBias} />
                <div className="param">
                  <div className="param-head">
                    <div className="param-name">Coordination Tendency</div>
                    <div className="param-help">Willingness to sync moves with allies.</div>
                  </div>
                  <ToggleBar options={["Lone wolf", "Pair", "Squad", "Hivemind"]} value={coord} onChange={setCoord} />
                  <div className="slider-extremes" style={{ marginTop: 8 }}>
                    <span>independently</span>
                    <span>fully networked</span>
                  </div>
                </div>
              </div>
            </div>

            {/* 04 — Difficulty */}
            <div className="mini-popup-section">
              <div className="param-grid one">
                <ParamSlider
                  name="Difficulty Scaling"
                  help="Globally scales reaction speed, awareness, and decision quality."
                  value={diff} onChange={setDiff}
                  lowLabel="story mode" highLabel="ironman"
                  accent="var(--ink)"
                />
              </div>
              <div className="diff-preview">
                <div className="diff-stat">
                  <div className="diff-stat-label">Eff. reaction</div>
                  <div className="diff-stat-row">
                    <span className="diff-stat-val">{reactMs < 1000 ? reactMs.toFixed(0) : (reactMs / 1000).toFixed(2)}</span>
                    <span className="diff-stat-unit">{reactMs < 1000 ? "ms" : "s"}</span>
                  </div>
                  <DiffBar pct={Math.max(0, Math.min(100, 100 - (reactMs / 3000) * 100))} />
                </div>
                <div className="diff-stat">
                  <div className="diff-stat-label">Awareness</div>
                  <div className="diff-stat-row">
                    <span className="diff-stat-val">{aware.toFixed(2)}</span>
                    <span className="diff-stat-unit">/ 10</span>
                  </div>
                  <DiffBar pct={aware * 10} />
                </div>
                <div className="diff-stat">
                  <div className="diff-stat-label">Decision Q.</div>
                  <div className="diff-stat-row">
                    <span className="diff-stat-val">{decQ.toFixed(2)}</span>
                    <span className="diff-stat-unit">/ 10</span>
                  </div>
                  <DiffBar pct={decQ * 10} />
                </div>
                <div className="diff-stat">
                  <div className="diff-stat-label">TTK Δ</div>
                  <div className="diff-stat-row">
                    <span className="diff-stat-val">{diff < 5 ? "+" : "−"}{Math.abs((diff - 5) * 7).toFixed(0)}%</span>
                    <span className="diff-stat-unit">vs base</span>
                  </div>
                  <DiffBar pct={Math.abs(diff - 5) * 20} />
                </div>
              </div>
              <div style={{ margin: "18px 0 10px", display: "flex", alignItems: "baseline", gap: 8 }}>
                <div style={{ fontSize: 12, fontWeight: 600 }}>Combat Role Presets</div>
                <div style={{ fontSize: 11, color: "var(--ink-3)" }}>One-click archetypes</div>
              </div>
              <div className="preset-grid">
                {PRESETS.map(p => (
                  <button key={p.id} className={"preset" + (preset === p.id ? " selected" : "")} onClick={() => applyPreset(p.id)}>
                    <div className="preset-icon">{p.icon}</div>
                    <div className="preset-name">{p.name}</div>
                    <div className="preset-desc">{p.desc}</div>
                    <div className="preset-spec">{p.spec.map(s => <span className="spec-pill mono" key={s}>{s}</span>)}</div>
                  </button>
                ))}
              </div>
            </div>

          </div>
        </div>
      </div>
    </div>
  );
}
