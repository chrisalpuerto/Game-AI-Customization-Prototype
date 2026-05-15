"use client";
import "./ai-character-studio.css";
import { useState, useEffect } from "react";
import Link from "next/link";

/* types */
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
  suffix?: string;
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

interface ReactionTimeProps {
  t: number;
  setT: (v: number) => void;
}

interface WeaponWeightProps {
  value: number;
  onChange: (v: number) => void;
}

interface ToggleBarProps {
  options: string[];
  value: string;
  onChange: (v: string) => void;
}

interface DiffBarProps {
  pct: number;
}

interface TabsProps {
  items: string[];
  active: string;
  onChange: (v: string) => void;
}

interface Preset {
  id: string;
  name: string;
  icon: React.ReactNode;
  desc: string;
  spec: string[];
}

/* ---------- Icons (inline strokes) ---------- */
function Icon({ d, size = 16, fill = "none", stroke = "currentColor", sw = 1.6 }: IconProps) {
  return (
    <svg width={size} height={size} viewBox="0 0 24 24" fill={fill} stroke={stroke} strokeWidth={sw} strokeLinecap="round" strokeLinejoin="round">{d}</svg>
  );
}

const I = {
  home:  <Icon d={<><path d="M3 11l9-7 9 7"/><path d="M5 10v10h14V10"/></>} />,
  bot:   <Icon d={<><rect x="4" y="7" width="16" height="12" rx="3"/><path d="M9 12v2M15 12v2M12 3v4"/><circle cx="12" cy="3" r="1"/></>} />,
  brain: <Icon d={<><path d="M9 4a3 3 0 0 0-3 3v.5A2.5 2.5 0 0 0 4 10v3a2.5 2.5 0 0 0 2 2.4V17a3 3 0 0 0 3 3h6a3 3 0 0 0 3-3v-1.6a2.5 2.5 0 0 0 2-2.4v-3A2.5 2.5 0 0 0 18 7.5V7a3 3 0 0 0-3-3h-1"/><path d="M12 4v16"/></>} />,
  flow:  <Icon d={<><circle cx="6" cy="6" r="2"/><circle cx="18" cy="18" r="2"/><circle cx="18" cy="6" r="2"/><path d="M8 6h8M6 8v8a2 2 0 0 0 2 2h8"/></>} />,
  beaker:<Icon d={<><path d="M9 3h6M10 3v6L5 19a2 2 0 0 0 1.7 3h10.6A2 2 0 0 0 19 19l-5-10V3"/></>} />,
  chart: <Icon d={<><path d="M4 20V8M10 20V4M16 20v-6M22 20H2"/></>} />,
  files: <Icon d={<><path d="M14 3H6a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z"/><path d="M14 3v6h6"/></>} />,
  cog:   <Icon d={<><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1A1.7 1.7 0 0 0 9 19.4a1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1A1.7 1.7 0 0 0 4.6 9a1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z"/></>} />,
  search:<Icon d={<><circle cx="11" cy="11" r="7"/><path d="M21 21l-4.3-4.3"/></>} />,
  bell:  <Icon d={<><path d="M18 8a6 6 0 1 0-12 0c0 7-3 8-3 8h18s-3-1-3-8"/><path d="M10 21a2 2 0 0 0 4 0"/></>} />,
  folder:<Icon d={<><path d="M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/></>} />,
  reset: <Icon d={<><path d="M3 12a9 9 0 1 0 3-6.7"/><path d="M3 4v5h5"/></>} />,
  play:  <Icon d={<><path d="M6 4l14 8-14 8z" fill="currentColor"/></>} />,
  save:  <Icon d={<><path d="M5 3h11l3 3v15H5z"/><path d="M8 3v6h8V3M8 21v-6h8v6"/></>} />,
  rush:  <Icon d={<><path d="M4 12h12M12 6l6 6-6 6"/></>} />,
  shield:<Icon d={<><path d="M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6z"/></>} />,
  scout: <Icon d={<><circle cx="12" cy="12" r="3"/><path d="M2 12h3M19 12h3M12 2v3M12 19v3"/></>} />,
  ghost: <Icon d={<><path d="M5 11a7 7 0 0 1 14 0v9l-2-2-2 2-2-2-2 2-2-2-2 2-2-2z"/><circle cx="9.5" cy="11" r=".8" fill="currentColor"/><circle cx="14.5" cy="11" r=".8" fill="currentColor"/></>} />,
  copy:  <Icon d={<><rect x="8" y="8" width="13" height="13" rx="2"/><path d="M16 8V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v9a2 2 0 0 0 2 2h3"/></>} />,
  more:  <Icon d={<><circle cx="5" cy="12" r="1.4" fill="currentColor"/><circle cx="12" cy="12" r="1.4" fill="currentColor"/><circle cx="19" cy="12" r="1.4" fill="currentColor"/></>} />,
  caret: <Icon d={<><path d="M6 9l6 6 6-6"/></>} sw={1.8} />,
  side:  <Icon d={<><rect x="3" y="4" width="18" height="16" rx="2"/><path d="M9 4v16"/></>} />,
  ask:   <Icon d={<><path d="M3 12a9 9 0 1 1 3.6 7.2L3 21l1.8-3.6A9 9 0 0 1 3 12z"/></>} />,
  spark: <Icon d={<><path d="M12 3v4M12 17v4M3 12h4M17 12h4M5.6 5.6l2.8 2.8M15.6 15.6l2.8 2.8M5.6 18.4l2.8-2.8M15.6 8.4l2.8-2.8"/></>} />,
};

/* ---------- Slider ---------- */
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
      <input
        className="slider"
        type="range"
        min={min} max={max} step={step}
        value={value}
        onChange={e => onChange(parseFloat(e.target.value))}
      />
    </div>
  );
}

/* ---------- Param 0–10 slider ---------- */
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

/* ---------- Detection radius (slider + unit + range inputs) ---------- */
const UNIT_TO_M: Record<DetectionUnit, number> = { in: 0.0254, ft: 0.3048, yd: 0.9144, mi: 1609.34 };

function DetectionRadius({ unit, setUnit, rangeMin, rangeMax, value, setValue, setRange }: DetectionRadiusProps) {
  const mVal = (value * UNIT_TO_M[unit]).toFixed(1);
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Detection Radius</div>
        <div className="param-help">Maximum range at which the agent can sense players, sound events, and projectiles.</div>
      </div>
      <div className="slider-wrap">
        <Slider value={value} onChange={setValue} min={rangeMin} max={rangeMax} step={(rangeMax - rangeMin) / 200} ticks={false} />
        <div className="slider-readout">{value.toFixed(unit === 'mi' ? 2 : 1)}<span className="unit">{unit}</span></div>
      </div>
      <div className="slider-extremes" style={{ marginTop: 4 }}>
        <span>≈ {mVal} m equivalent</span>
        <span>{unit === 'mi' ? 'long-range scout' : unit === 'in' ? 'point-blank only' : 'standard sightline'}</span>
      </div>
      <div className="range-row">
        <span className="label">RANGE</span>
        <input className="num-input" type="number" value={rangeMin} onChange={e => setRange(parseFloat(e.target.value) || 0, rangeMax)} />
        <span className="label">→</span>
        <input className="num-input" type="number" value={rangeMax} onChange={e => setRange(rangeMin, parseFloat(e.target.value) || 0)} />
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

/* ---------- Reaction time (exponential 1.5) ---------- */
// t01 in [0..1] -> ms 50..3000 with exponent 1.5
function reactionFromT(t: number): number {
  const minMs = 50, maxMs = 3000;
  const eased = Math.pow(t, 1.5);
  return minMs + eased * (maxMs - minMs);
}

function ReactionTime({ t, setT }: ReactionTimeProps) {
  const ms = reactionFromT(t);
  const display = ms < 1000 ? `${ms.toFixed(0)} ms` : `${(ms / 1000).toFixed(2)} s`;
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Reaction Time</div>
        <div className="param-help">Latency between perceiving a stimulus and acting. Curve scales exponentially (γ=1.5) — slow zone grows fast at the upper end.</div>
      </div>
      <div className="slider-wrap">
        <Slider value={t} onChange={setT} min={0} max={1} step={0.001} ticks={false} />
        <div className="slider-readout">{display}</div>
      </div>
      <div className="slider-extremes"><span>50 ms — frame-perfect</span><span>3.0 s — sluggish</span></div>
    </div>
  );
}

/* ---------- Toggle switch ---------- */
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

/* ---------- Weapon weight (continuous toggle slider) ---------- */
function WeaponWeight({ value, onChange }: WeaponWeightProps) {
  const pct = (value + 1) * 50;  // -1..1 -> 0..100
  return (
    <div className="param">
      <div className="param-head">
        <div className="param-name">Weapon Preference Weight</div>
        <div className="param-help">Bias toward favoring particular engagement ranges when picking a weapon.</div>
      </div>
      <div className="slider-wrap">
        <div className="slider-track-wrap" style={{ height: 22 }}>
          <div className="slider-bg" />
          <div style={{
            position: 'absolute', left: '50%', top: '50%',
            transform: 'translate(-50%,-50%)', width: 1, height: 14, background: 'var(--ink-3)', opacity: .4
          }} />
          {/* fill from center */}
          <div style={{
            position: 'absolute', top: '50%', height: 4, transform: 'translateY(-50%)',
            background: 'var(--ink)', borderRadius: 2,
            left: value < 0 ? `${pct}%` : '50%',
            width: `${Math.abs(value) * 50}%`
          }} />
          <input
            className="slider"
            type="range" min={-1} max={1} step={0.01}
            value={value}
            onChange={e => onChange(parseFloat(e.target.value))}
          />
        </div>
        <div className="slider-readout">
          {value === 0 ? "neutral" : `${value > 0 ? "+" : ""}${(value * 100).toFixed(0)}%`}
        </div>
      </div>
      <div className="slider-extremes">
        <span>← Long range · sniper, DMR</span>
        <span>SMG, shotgun · Short range →</span>
      </div>
    </div>
  );
}

/* ---------- Diff bar ---------- */
function DiffBar({ pct }: DiffBarProps) { return <div className="diff-bar"><span style={{ width: pct + '%' }} /></div>; }

/* ---------- Tabs ---------- */
function Tabs({ items, active, onChange }: TabsProps) {
  return (
    <div className="tabs">
      {items.map(it => (
        <button key={it} className={"tab" + (it === active ? " on" : "")} onClick={() => onChange(it)}>{it}</button>
      ))}
    </div>
  );
}

/* ---------- Animation library ---------- */
interface AnimEntry { id: string; name: string; icon: React.ReactNode; cat: string; dur: string; desc: string; }
const ANIM_DATA: AnimEntry[] = [
  { id: "idle-std", name: "Idle — Standard",  icon: I.scout,  cat: "Locomotion", dur: "loop",  desc: "Neutral standing idle with subtle weight shift." },
  { id: "walk",     name: "Walk Cycle",        icon: I.rush,   cat: "Locomotion", dur: "loop",  desc: "Full walk loop, blends with run at threshold." },
  { id: "sprint",   name: "Combat Sprint",     icon: I.rush,   cat: "Locomotion", dur: "loop",  desc: "Weapon-raised sprint, compressed stride." },
  { id: "wounded",  name: "Wounded Walk",      icon: I.shield, cat: "Locomotion", dur: "loop",  desc: "Limping gait triggered below 30% health." },
  { id: "draw",     name: "Draw Weapon",       icon: I.play,   cat: "Combat",     dur: "0.6 s", desc: "Holster-to-aim transition, right-hand draw." },
  { id: "cover",    name: "Take Cover",        icon: I.shield, cat: "Combat",     dur: "0.8 s", desc: "Low crouch slide into wall/pillar cover." },
  { id: "peek",     name: "Peek & Fire",       icon: I.scout,  cat: "Combat",     dur: "1.4 s", desc: "Lean out, aim, return — cover-bound." },
  { id: "melee",    name: "Melee Strike",      icon: I.rush,   cat: "Combat",     dur: "0.5 s", desc: "Right-hook punch, interrupts weapon anim." },
  { id: "reload",   name: "Reload — Rifle",    icon: I.reset,  cat: "Combat",     dur: "2.1 s", desc: "Full mag swap with bolt-charge finish." },
  { id: "death",    name: "Death Fall",        icon: I.ghost,  cat: "Reaction",   dur: "1.8 s", desc: "Ragdoll-blend collapse, direction-aware." },
  { id: "alert",    name: "Alert Scan",        icon: I.brain,  cat: "Reaction",   dur: "loop",  desc: "Head-sweep search behaviour, raised weapon." },
  { id: "victory",  name: "Victory Stance",    icon: I.spark,  cat: "Social",     dur: "2.5 s", desc: "Post-kill taunt; plays once then idles." },
];

function AnimationTab() {
  return (
    <div className="tab-content">
      <section className="section">
        <div className="section-head">
          <div className="section-num">12</div>
          <div>
            <h2 className="section-title">Animation Library</h2>
            <p className="section-desc">Assign animations to this character&apos;s state machine. Drag to reorder priority.</p>
          </div>
          <div className="section-tools">
            <button className="icon-btn">{I.reset}</button>
            <button className="icon-btn">{I.more}</button>
          </div>
        </div>
        <div className="preset-grid">
          {ANIM_DATA.map(a => (
            <button key={a.id} className="preset">
              <div className="preset-icon">{a.icon}</div>
              <div className="preset-name">{a.name}</div>
              <div className="preset-desc">{a.desc}</div>
              <div className="preset-spec">
                <span className="spec-pill mono">{a.cat}</span>
                <span className="spec-pill mono">{a.dur}</span>
              </div>
            </button>
          ))}
        </div>
      </section>
    </div>
  );
}

/* ---------- Character roster ---------- */
type CharStatus = "Active" | "Draft" | "Archived";
interface CharEntry { id: string; name: string; role: string; faction: string; status: CharStatus; level: number; gradient: string; }
const CHARACTER_DATA: CharEntry[] = [
  { id: "wraith",    name: "Wraith",    role: "Heavy Operator · Boss",      faction: "CORE",   status: "Active",   level: 12, gradient: "linear-gradient(135deg,#1F1D19,#3D3A33), radial-gradient(circle at 70% 30%,#F4B58A 0%,transparent 50%)" },
  { id: "phantom",   name: "Phantom",   role: "Ghost Agent",                 faction: "SHADOW", status: "Active",   level:  9, gradient: "linear-gradient(135deg,#1A1D2E,#2E3250)" },
  { id: "ironclad",  name: "Ironclad",  role: "Shield Vanguard · Elite",     faction: "CORE",   status: "Active",   level: 11, gradient: "linear-gradient(135deg,#232323,#4A4A4A)" },
  { id: "venom",     name: "Venom",     role: "Assassin",                    faction: "SHADOW", status: "Active",   level:  8, gradient: "linear-gradient(135deg,#1A2A1A,#2D4A2D)" },
  { id: "blaze",     name: "Blaze",     role: "Pyromaniac · Demolitions",    faction: "ROGUE",  status: "Active",   level:  7, gradient: "linear-gradient(135deg,#3A1A0A,#6B2A14)" },
  { id: "atlas",     name: "Atlas",     role: "Field Medic · Support",       faction: "CORE",   status: "Active",   level:  6, gradient: "linear-gradient(135deg,#0A2230,#1A4A6A)" },
  { id: "rook",      name: "Rook",      role: "Recon Scout",                 faction: "CORE",   status: "Active",   level:  5, gradient: "linear-gradient(135deg,#2A2018,#5A4A30)" },
  { id: "cipher",    name: "Cipher",    role: "Electronic Warfare",          faction: "TECH",   status: "Active",   level: 10, gradient: "linear-gradient(135deg,#0A1A2A,#0A3040)" },
  { id: "sentinel",  name: "Sentinel",  role: "Perimeter Defender",          faction: "CORE",   status: "Active",   level:  6, gradient: "linear-gradient(135deg,#1A1820,#3A3450)" },
  { id: "nova",      name: "Nova",      role: "Long-Range Sniper",           faction: "SHADOW", status: "Draft",    level:  4, gradient: "linear-gradient(135deg,#28202A,#503860)" },
  { id: "ember",     name: "Ember",     role: "Patrol Grunt",                faction: "ROGUE",  status: "Active",   level:  2, gradient: "linear-gradient(135deg,#2A1810,#503020)" },
  { id: "titan",     name: "Titan",     role: "Elite Guard · Boss",          faction: "CORE",   status: "Draft",    level: 13, gradient: "linear-gradient(135deg,#181818,#303030)" },
  { id: "specter",   name: "Specter",   role: "Shadow Infiltrator",          faction: "SHADOW", status: "Active",   level:  9, gradient: "linear-gradient(135deg,#101018,#202038)" },
  { id: "axiom",     name: "Axiom",     role: "AI Commander · Boss",         faction: "TECH",   status: "Draft",    level: 15, gradient: "linear-gradient(135deg,#0A1820,#103050)" },
];

function CharacterTab() {
  return (
    <div className="tab-content">
      <section className="section">
        <div className="section-head">
          <div className="section-num">14</div>
          <div>
            <h2 className="section-title">All Characters</h2>
            <p className="section-desc">Browse the full NPC roster. Select a character to configure its behavior profile.</p>
          </div>
        </div>
        <div className="character-grid">
          {CHARACTER_DATA.map(c => (
            <div key={c.id} className="character-card">
              <div className="char-card-portrait" style={{ background: c.gradient }} />
              <div>
                <div className="char-card-name">{c.name}</div>
                <div className="char-card-role">{c.role}</div>
              </div>
              <div className="char-card-meta">
                <span className={`char-card-badge${c.status === "Active" ? " active" : c.status === "Draft" ? " draft" : ""}`}>{c.status}</span>
                <span className="char-card-badge mono">{c.faction}</span>
                <span className="char-card-badge mono">Lv {c.level}</span>
              </div>
            </div>
          ))}
        </div>
      </section>
    </div>
  );
}

/* ---------- Memory log ---------- */
const MEMORY_LOG = [
  { t: "00:14:32", tag: "PERCEPT", msg: "Detected hostile — distance 47 ft, bearing NW" },
  { t: "00:14:35", tag: "COMBAT",  msg: "Engaged target: opened fire (3-round burst)" },
  { t: "00:14:41", tag: "MOVE",    msg: "Advanced to cover — pillar C-4" },
  { t: "00:14:48", tag: "PERCEPT", msg: "Lost visual on target" },
  { t: "00:14:52", tag: "WAIT",    msg: "Holding position — threat status: unknown" },
  { t: "00:15:01", tag: "PERCEPT", msg: "Re-acquired target — health estimate ~60%" },
  { t: "00:15:04", tag: "COMBAT",  msg: "Called for backup on squad channel" },
  { t: "00:15:09", tag: "MOVE",    msg: "Flanking maneuver — route via north corridor" },
  { t: "00:15:18", tag: "COMBAT",  msg: "Target eliminated" },
  { t: "00:15:19", tag: "MOVE",    msg: "Returning to patrol route" },
  { t: "00:15:24", tag: "STATE",   msg: "Aggression scaled down — area clear" },
  { t: "00:15:30", tag: "PERCEPT", msg: "Sound event detected — footsteps, ~80 ft NE" },
];

function MemoryTab() {
  return (
    <div className="tab-content">
      <section className="section">
        <div className="section-head">
          <div className="section-num">01</div>
          <div>
            <h2 className="section-title">Character Memory Log</h2>
            <p className="section-desc">Recorded perception and state events from the last sandbox session.</p>
          </div>
          <div className="section-tools">
            <button className="pill-btn">{I.save}<span>Export .json</span></button>
          </div>
        </div>
        <div className="upload-zone">
          <div className="upload-zone-icon">{I.files}</div>
          <div className="upload-zone-text">
            <span className="upload-zone-label">Upload memory snapshot</span>
            <span className="upload-zone-sub">Drop a .json file or click to browse</span>
          </div>
          <button className="pill-btn primary">Upload .json</button>
        </div>
        <div className="memory-log">
          {MEMORY_LOG.map((e, i) => (
            <div key={i} className="memory-entry">
              <span className="memory-time mono">{e.t}</span>
              <span className={"memory-tag mono " + e.tag.toLowerCase()}>{e.tag}</span>
              <span className="memory-msg">{e.msg}</span>
            </div>
          ))}
        </div>
      </section>
    </div>
  );
}

/* ---------- Combat role presets ---------- */
const PRESETS: Preset[] = [
  { id: "rusher",   name: "Aggressive Rusher",  icon: I.rush,   desc: "Closes distance fast. High aggression, low caution.", spec: ["AGGR 9.0", "CAUT 2.0", "RANGE -0.6"] },
  { id: "tactical", name: "Tactical Soldier",   icon: I.shield, desc: "Balanced, uses cover, coordinates with squad.",        spec: ["AGGR 5.5", "AWARE 7.5", "COORD 8"] },
  { id: "survivor", name: "Cautious Survivor",  icon: I.ghost,  desc: "Avoids fights unless cornered. Resource-efficient.",   spec: ["AGGR 2.0", "DEF 9.0", "HEALTH 9"] },
  { id: "scout",    name: "Recon Scout",        icon: I.scout,  desc: "Watches, marks targets, repositions before engaging.", spec: ["AWARE 9.5", "RANGE +0.4", "COORD 6"] },
];

/* ---------- FloatingActionBar ---------- */
function FloatingActionBar() {
  const [expanded, setExpanded] = useState(true);
  const cls = expanded ? "floater-wrap--expanded" : "floater-wrap--collapsed";
  return (
    <div className={"floater-wrap " + cls}>
      <div className="floater">
        <span className="status"><span className="pulse" />3 unsaved changes · live in sandbox</span>
        <span className="spacer" />
        <button className="pill-btn ghost">Discard</button>
        <button className="pill-btn ghost">{I.play}<span>Re-run sim</span></button>
        <button className="pill-btn accent">{I.save}<span>Save &amp; deploy</span></button>
        <button className="floater-toggle" onClick={() => setExpanded(false)} aria-label="Collapse">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <polyline points="2,5 7,10 12,5" />
          </svg>
        </button>
      </div>
      <button className="floater-expand" onClick={() => setExpanded(true)} aria-label="Expand">
        <svg width="14" height="14" viewBox="0 0 14 14" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
          <polyline points="2,9 7,4 12,9" />
        </svg>
      </button>
    </div>
  );
}

/* ---------- Home ---------- */
function HomeTab({ onNavigate }: { onNavigate: (t: string) => void }) {
  const NAV_CARDS = [
    { icon: I.bot,   title: "Characters", desc: "Browse and configure all 14 NPCs in the roster.",       badge: "14 NPCs",  action: () => onNavigate("Characters") },
    { icon: I.brain, title: "Behavior",   desc: "Tune perception, aggression, and decision thresholds.",  badge: "Active",   action: () => onNavigate("Behavior")   },
    { icon: I.play,  title: "Animation",  desc: "Assign motion clips to each character state.",           badge: "12 clips", action: () => onNavigate("Animation")  },
    { icon: I.chart, title: "Memory Log", desc: "Inspect recorded events and upload memory snapshots.",   badge: null,       action: () => onNavigate("Memory")     },
    { icon: I.cog,   title: "Settings",   desc: "Profile, preferences, API keys, and quick launch.",      badge: null,       action: () => onNavigate("Settings")   },
  ];
  const LAUNCH_CARDS = [
    { icon: I.beaker, title: "Open Sandbox",  desc: "Interactive NPC test environment.",             href: "/sandbox"    },
    { icon: I.ghost,  title: "Launch Game",   desc: "Latest build — boss encounter (Game Demo 3).", href: "/game-demo3" },
    { icon: I.spark,  title: "Cloud View",    desc: "Volumetric cloud simulation renderer.",         href: "/cloudview"  },
  ];
  return (
    <div className="tab-content">
      <div className="home-hero">
        <div className="home-eyebrow">CPSC 490 · Group 4 · Game Dev</div>
        <h1 className="home-greeting">Welcome back,<br /><em>Alex.</em></h1>
        <p className="home-tagline">Your NPC configuration workspace. Pick up where you left off or explore a new area below.</p>
        <div className="home-meta-row">
          <span className="home-meta-chip"><span className="home-meta-dot" />Live in sandbox</span>
          <span className="home-meta-chip">Last active 12 min ago</span>
          <span className="home-meta-chip">3 unsaved changes</span>
          <span className="home-meta-chip">v0.8.3 · build 4127</span>
        </div>
      </div>
      <div className="home-section-label">Workspace</div>
      <div className="home-grid">
        {NAV_CARDS.map(c => (
          <button key={c.title} className="home-card" onClick={c.action} style={{ border: "1px solid var(--line)" }}>
            <div className="home-card-icon">{c.icon}</div>
            <div className="home-card-title">{c.title}</div>
            <div className="home-card-desc">{c.desc}</div>
            {c.badge && <div className="home-card-badge"><span className="spec-pill mono">{c.badge}</span></div>}
          </button>
        ))}
      </div>
      <div className="home-section-label">Launch</div>
      <div className="home-grid">
        {LAUNCH_CARDS.map(c => (
          <Link key={c.href} href={c.href} className="home-card" target="_blank" rel="noopener noreferrer" style={{ border: "1px solid var(--line)" }}>
            <div className="home-card-icon">{c.icon}</div>
            <div className="home-card-title">{c.title}</div>
            <div className="home-card-desc">{c.desc}</div>
          </Link>
        ))}
      </div>
    </div>
  );
}

/* ---------- Settings ---------- */
function SettingsTab() {
  return (
    <div className="tab-content">

      {/* 01 Profile */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">01</div>
          <div><h2 className="section-title">Profile</h2><p className="section-desc">Your identity within this project workspace.</p></div>
        </div>
        <div className="profile-hero">
          <div className="profile-avatar-lg" />
          <div>
            <div className="profile-hero-name">Alex Chen</div>
            <div className="profile-hero-role">Game Developer · Group 4</div>
            <div className="profile-hero-badge">{I.shield}<span>Admin</span></div>
          </div>
          <button className="pill-btn" style={{ marginLeft: "auto" }}>{I.copy}<span>Change photo</span></button>
        </div>
        <div>
          {([
            { label: "Display Name", sub: "Shown in exports and sim logs",  ctrl: <input className="settings-input" defaultValue="Alex Chen" /> },
            { label: "Email",        sub: "Notifications go here",          ctrl: <input className="settings-input" defaultValue="alex.chen@studio.dev" /> },
            { label: "Role",         sub: "Your function on this project",  ctrl: <select className="settings-select"><option>Game Developer</option><option>Designer</option><option>QA Engineer</option></select> },
            { label: "Team",         sub: "Workspace group",                ctrl: <input className="settings-input" defaultValue="Group 4" /> },
            { label: "Time Zone",    sub: "Used for scheduled sims",        ctrl: <select className="settings-select"><option>UTC−5 (Eastern)</option><option>UTC−8 (Pacific)</option><option>UTC+0 (London)</option></select> },
          ] as { label: string; sub: string; ctrl: React.ReactNode }[]).map(r => (
            <div key={r.label} className="settings-row">
              <div className="settings-label">{r.label}<span className="settings-label-sub">{r.sub}</span></div>
              <div className="settings-control">{r.ctrl}</div>
            </div>
          ))}
        </div>
      </section>

      {/* 02 Preferences */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">02</div>
          <div><h2 className="section-title">Preferences</h2><p className="section-desc">Application-level display and behaviour settings.</p></div>
        </div>
        <div>
          {([
            { label: "Theme",         sub: "Interface colour scheme",        ctrl: <select className="settings-select"><option>Light</option><option>Dark</option><option>System</option></select> },
            { label: "Default Tab",   sub: "Tab shown on character open",    ctrl: <select className="settings-select"><option>Behavior</option><option>Animation</option><option>Memory</option></select> },
            { label: "Autosave",      sub: "Save changes every 30 s",        ctrl: <button className="tog on" /> },
            { label: "Show Tooltips", sub: "Inline parameter descriptions",  ctrl: <button className="tog on" /> },
            { label: "Compact Mode",  sub: "Reduce padding in sections",     ctrl: <button className="tog" /> },
            { label: "Language",      sub: "UI display language",            ctrl: <select className="settings-select"><option>English (US)</option><option>English (UK)</option><option>Español</option></select> },
          ] as { label: string; sub: string; ctrl: React.ReactNode }[]).map(r => (
            <div key={r.label} className="settings-row">
              <div className="settings-label">{r.label}<span className="settings-label-sub">{r.sub}</span></div>
              <div className="settings-control">{r.ctrl}</div>
            </div>
          ))}
        </div>
      </section>

      {/* 03 Quick Launch */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">03</div>
          <div><h2 className="section-title">Quick Launch</h2><p className="section-desc">Open any project environment directly.</p></div>
        </div>
        <div className="launch-grid">
          {[
            { href: "/sandbox",    icon: I.beaker, name: "Sandbox",        desc: "Interactive NPC test environment",   path: "/sandbox"    },
            { href: "/game-demo",  icon: I.play,   name: "Game Demo",      desc: "First gameplay demo build",          path: "/game-demo"  },
            { href: "/game-demo3", icon: I.ghost,  name: "Game Demo 3",    desc: "Latest build — boss encounter",      path: "/game-demo3" },
            { href: "/cloudview",  icon: I.spark,  name: "Cloud View",     desc: "Cloud simulation renderer",          path: "/cloudview"  },
            { href: "/cloudview2", icon: I.spark,  name: "Cloud View 2",   desc: "Updated volumetric renderer",        path: "/cloudview2" },
            { href: "/threejs",    icon: I.scout,  name: "Three.js Scene", desc: "Raw Three.js debug scene",           path: "/threejs"    },
          ].map(c => (
            <Link key={c.href} href={c.href} className="launch-card" target="_blank" rel="noopener noreferrer">
              <div className="launch-card-icon">{c.icon}</div>
              <div className="launch-card-name">{c.name}</div>
              <div className="launch-card-desc">{c.desc}</div>
              <div className="launch-card-path">{c.path}</div>
            </Link>
          ))}
        </div>
      </section>

      {/* 04 Notifications */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">04</div>
          <div><h2 className="section-title">Notifications</h2><p className="section-desc">Choose which events trigger an alert.</p></div>
        </div>
        <div>
          {[
            { label: "Simulation complete", sub: "When a sandbox run finishes",      on: true  },
            { label: "Build errors",         sub: "Compiler or deploy failures",      on: true  },
            { label: "New character added",  sub: "When a teammate adds an NPC",      on: false },
            { label: "Weekly digest",        sub: "Summary email every Monday",       on: true  },
          ].map(r => (
            <div key={r.label} className="settings-row">
              <div className="settings-label">{r.label}<span className="settings-label-sub">{r.sub}</span></div>
              <div className="settings-control"><button className={"tog" + (r.on ? " on" : "")} /></div>
            </div>
          ))}
        </div>
      </section>

      {/* 05 API & Project */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">05</div>
          <div><h2 className="section-title">API &amp; Project</h2><p className="section-desc">Project identifiers and integration settings.</p></div>
        </div>
        <div>
          {([
            { label: "Project ID",   sub: "Read-only workspace identifier", ctrl: <input className="settings-input" readOnly defaultValue="cpsc490-group4" style={{ fontFamily: "'JetBrains Mono',monospace", fontSize: 12 }} /> },
            { label: "API Key",      sub: "Rotate every 90 days",           ctrl: <input className="settings-input" readOnly defaultValue="sk-••••••••••••••••4f2a" style={{ fontFamily: "'JetBrains Mono',monospace", fontSize: 12 }} /> },
            { label: "Build Target", sub: "Export compilation target",      ctrl: <select className="settings-select"><option>WebGL</option><option>Native (macOS)</option><option>Native (Win64)</option></select> },
            { label: "Max Sessions", sub: "Parallel sandbox instances",     ctrl: <select className="settings-select"><option>1</option><option>3</option><option>5</option><option>10</option></select> },
            { label: "SDK Version",  sub: "Simulation core library",        ctrl: <span className="spec-pill mono" style={{ fontSize: 12 }}>v2.4.1-stable</span> },
          ] as { label: string; sub: string; ctrl: React.ReactNode }[]).map(r => (
            <div key={r.label} className="settings-row">
              <div className="settings-label">{r.label}<span className="settings-label-sub">{r.sub}</span></div>
              <div className="settings-control">{r.ctrl}</div>
            </div>
          ))}
        </div>
      </section>

      {/* 06 Danger Zone */}
      <section className="section">
        <div className="section-head">
          <div className="section-num">06</div>
          <div><h2 className="section-title">Danger Zone</h2><p className="section-desc">Irreversible actions — proceed with caution.</p></div>
        </div>
        <div>
          {[
            { label: "Reset preferences",     sub: "Restore all settings to factory defaults" },
            { label: "Clear sandbox data",    sub: "Delete all recorded session logs and snapshots" },
            { label: "Delete all characters", sub: "Permanently remove all 14 NPCs and their profiles" },
          ].map(r => (
            <div key={r.label} className="danger-row">
              <div className="danger-row-text">
                <div className="danger-row-label">{r.label}</div>
                <div className="danger-row-sub">{r.sub}</div>
              </div>
              <button className="btn-danger">Delete</button>
            </div>
          ))}
        </div>
      </section>

    </div>
  );
}

/* ---------- App ---------- */
function App() {
  const [tab, setTab] = useState("Behavior");

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
  // keep radius inside range
  useEffect(() => {
    if (radius < rng.min) setRadius(rng.min);
    if (radius > rng.max) setRadius(rng.max);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [rng.min, rng.max]);

  const [reactT, setReactT] = useState(0.32);
  const [moveSpd, setMoveSpd] = useState(6.0);

  // Section 3
  const [healthW, setHealthW] = useState(7.5);
  const [ammoW, setAmmoW]     = useState(4.0);
  const [weaponBias, setWeaponBias] = useState(-0.25);
  const [coord, setCoord] = useState("Squad");

  // Section 4
  const [diff, setDiff] = useState(6.0);
  const [preset, setPreset] = useState("tactical");

  // Apply preset
  function applyPreset(id: string) {
    setPreset(id);
    if (id === "rusher")   { setAggression(9); setDefensive(2); setTactical(5); setDecision(7); setReactT(0.18); setMoveSpd(8.5); setHealthW(3); setAmmoW(5); setWeaponBias(-0.6); setCoord("Lone wolf"); }
    if (id === "tactical") { setAggression(5.5); setDefensive(5.5); setTactical(7.5); setDecision(6); setReactT(0.32); setMoveSpd(6); setHealthW(7); setAmmoW(6); setWeaponBias(-0.05); setCoord("Squad"); }
    if (id === "survivor") { setAggression(2); setDefensive(9); setTactical(6.5); setDecision(4); setReactT(0.45); setMoveSpd(4.5); setHealthW(9); setAmmoW(8); setWeaponBias(0.2); setCoord("Lone wolf"); }
    if (id === "scout")    { setAggression(3.5); setDefensive(6); setTactical(9.5); setDecision(7); setReactT(0.22); setMoveSpd(7); setHealthW(6); setAmmoW(5); setWeaponBias(0.4); setCoord("Pair"); }
  }

  // Difficulty-derived stats
  const reactMs = reactionFromT(reactT) * (1 - (diff - 5) * 0.04);
  const aware = Math.min(10, tactical * (0.7 + diff * 0.06));
  const decQ  = Math.min(10, decision * (0.7 + diff * 0.06));

  return (
    <div className="app">
      {/* SIDEBAR */}
      <aside className="sidebar">
        <div className="brand">
          <div className="brand-mark" />
          <div className="brand-name">CPSC 490</div>
          <span className="brand-caret">{I.caret}</span>
        </div>

        <div className="nav-group">
          <button className={"nav-item" + (tab === "Home" ? " active" : "")} onClick={() => setTab("Home")}>{I.home}<span>Home</span></button>
          <button className={"nav-item" + (tab === "Characters" ? " active" : "")} onClick={() => setTab("Characters")}>{I.bot}<span>Characters</span><span className="badge">14</span></button>
          <button className={"nav-item" + (tab === "Behavior" ? " active" : "")} onClick={() => setTab("Behavior")}>{I.brain}<span>Behavior</span></button>
          <button className={"nav-item" + (tab === "Animation" ? " active" : "")} onClick={() => setTab("Animation")}>{I.play}<span>Animation</span></button>
          <button className={"nav-item" + (tab === "Memory" ? " active" : "")} onClick={() => setTab("Memory")}>{I.chart}<span>Memory</span></button>
          <button className="nav-item">{I.beaker}<span>Sandbox</span></button>
          <button className="nav-item">{I.files}<span>Exports</span></button>
        </div>

        <div className="nav-label">Pinned</div>
        <div className="nav-group">
          <button className="nav-item">{I.spark}<span>Recently played</span></button>
          <button className="nav-item">{I.folder}<span>Wraith — boss</span></button>
          <button className="nav-item">{I.folder}<span>Patrol grunts</span></button>
          <button className={"nav-item" + (tab === "Settings" ? " active" : "")} onClick={() => setTab("Settings")}>{I.cog}<span>Settings</span></button>
        </div>

        <div className="sidebar-foot">
          <div className="avatar" />
          <div className="who">
            <span className="who-name">Group 4</span>
            <span className="who-role">Game Dev · Alex</span>
          </div>
        </div>
      </aside>

      {/* MAIN */}
      <main className="main">
        <div className="topbar">
          <button className="icon-btn" title="Collapse sidebar">{I.side}</button>
          <div className="crumbs">
            <span>Characters</span>
            <span className="sep">›</span>
            <span>Wraith</span>
            <span className="sep">›</span>
            <span className="here">Behavior Profile</span>
          </div>
          <div className="topbar-actions">
            <button className="pill-btn">{I.ask}<span>Ask</span></button>
            <button className="pill-btn">{I.copy}<span>Duplicate</span></button>
            <button className="icon-btn">{I.bell}</button>
            <Link href="/sandbox" className="pill-btn">{I.play}<span>Test in Sandbox</span></Link>
            <Link href="/game-demo3" className="pill-btn" target="_blank" rel="noopener noreferrer">{I.ghost}<span>Launch Game</span></Link>
            <button className="pill-btn primary">{I.save}<span>Save profile</span><span className="kbd">⌘S</span></button>
          </div>
        </div>

        <div className="page">
          {tab === "Home" && <HomeTab onNavigate={setTab} />}

          {/* HEADER */}
          {tab !== "Home" && <div className="page-head">
            <div>
              <h1 className="page-title">Behavior Profile</h1>
              <p className="page-sub">Tune perception, judgement, and combat priorities for this NPC. Changes propagate live to running sandbox sessions.</p>
            </div>
            <div className="profile-meta">
              <Tabs items={["Behavior", "Animation", "Characters", "Memory"]} active={tab} onChange={setTab} />
              <div className="char-card">
                <div className="char-portrait" />
                <div>
                  <div className="char-name">Wraith — Heavy Operator</div>
                  <div className="char-meta">v0.8.3 · build 4127 · last sim 12m ago</div>
                </div>
              </div>
            </div>
          </div>}

          {tab === "Characters" && <CharacterTab />}
          {tab === "Animation" && <AnimationTab />}
          {tab === "Memory" && <MemoryTab />}
          {tab === "Settings" && <SettingsTab />}

          {tab === "Behavior" && <div className="tab-content">
          {/* SECTION 1 — Personality */}
          <section className="section">
            <div className="section-head">
              <div className="section-num">01</div>
              <div>
                <h2 className="section-title">Personality &amp; Behavior Biases</h2>
                <p className="section-desc">High-level dispositions that color decision-making across all encounters.</p>
              </div>
              <div className="section-tools">
                <button className="icon-btn" title="Reset">{I.reset}</button>
                <button className="icon-btn" title="More">{I.more}</button>
              </div>
            </div>
            <div className="param-grid">
              <ParamSlider name="Aggression Level" help="Eagerness to initiate and press attacks." value={aggression} onChange={setAggression} lowLabel="passive" highLabel="bloodthirsty" />
              <ParamSlider name="Defensiveness / Passiveness" help="Tendency to seek cover and avoid exposure." value={defensive} onChange={setDefensive} lowLabel="reckless" highLabel="turtle" />
              <ParamSlider name="Tactical Awareness" help="Map reading, flanking, line-of-sight management." value={tactical} onChange={setTactical} lowLabel="oblivious" highLabel="general" />
              <ParamSlider name="Decision Threshold" help="Confidence required before committing to an action." value={decision} onChange={setDecision} lowLabel="impulsive" highLabel="deliberate" />
            </div>
          </section>

          {/* SECTION 2 — Perception */}
          <section className="section">
            <div className="section-head">
              <div className="section-num">02</div>
              <div>
                <h2 className="section-title">Perceptual &amp; Performance Limits</h2>
                <p className="section-desc">Hard ceilings on what the agent can sense and how fast it can act.</p>
              </div>
              <div className="section-tools">
                <button className="icon-btn">{I.reset}</button>
                <button className="icon-btn">{I.more}</button>
              </div>
            </div>
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
          </section>

          {/* SECTION 3 — Resources & Combat */}
          <section className="section">
            <div className="section-head">
              <div className="section-num">03</div>
              <div>
                <h2 className="section-title">Resource &amp; Combat Priorities</h2>
                <p className="section-desc">How much the agent values upkeep versus output.</p>
              </div>
              <div className="section-tools">
                <button className="icon-btn">{I.reset}</button>
                <button className="icon-btn">{I.more}</button>
              </div>
            </div>
            <div className="param-grid">
              <ParamSlider name="Health Importance" help="Likelihood to break engagement to heal or retreat." value={healthW} onChange={setHealthW} lowLabel="ignore" highLabel="self-preserve" />
              <ParamSlider name="Ammo Importance" help="Conservatism of fire — bursts vs. spray." value={ammoW} onChange={setAmmoW} lowLabel="spray" highLabel="conserve" />
              <WeaponWeight value={weaponBias} onChange={setWeaponBias} />
              <div className="param">
                <div className="param-head">
                  <div className="param-name">Coordination Tendency</div>
                  <div className="param-help">How willing the agent is to share intel and synchronize moves with allies.</div>
                </div>
                <ToggleBar options={["Lone wolf", "Pair", "Squad", "Hivemind"]} value={coord} onChange={setCoord} />
                <div className="slider-extremes" style={{ marginTop: 8 }}>
                  <span>operates independently</span>
                  <span>fully networked</span>
                </div>
              </div>
            </div>
          </section>

          {/* SECTION 4 — Difficulty */}
          <section className="section">
            <div className="section-head">
              <div className="section-num">04</div>
              <div>
                <h2 className="section-title">Difficulty Presets</h2>
                <p className="section-desc">Global scalers and one-click archetypes that overwrite the values above.</p>
              </div>
              <div className="section-tools">
                <button className="icon-btn">{I.reset}</button>
                <button className="icon-btn">{I.more}</button>
              </div>
            </div>

            <div className="param-grid one">
              <ParamSlider
                name="Difficulty Scaling"
                help="Globally scales reaction speed, awareness, and decision quality. Stacks multiplicatively with the values above."
                value={diff} onChange={setDiff}
                lowLabel="story mode" highLabel="ironman"
                accent="var(--ink)"
              />
            </div>

            <div className="diff-preview">
              <div className="diff-stat">
                <div className="diff-stat-label">Effective reaction</div>
                <div className="diff-stat-row">
                  <span className="diff-stat-val">{reactMs < 1000 ? reactMs.toFixed(0) : (reactMs / 1000).toFixed(2)}</span>
                  <span className="diff-stat-unit">{reactMs < 1000 ? "ms" : "s"}</span>
                </div>
                <DiffBar pct={Math.max(0, Math.min(100, 100 - (reactMs / 3000) * 100))} />
              </div>
              <div className="diff-stat">
                <div className="diff-stat-label">Awareness factor</div>
                <div className="diff-stat-row">
                  <span className="diff-stat-val">{aware.toFixed(2)}</span>
                  <span className="diff-stat-unit">/ 10</span>
                </div>
                <DiffBar pct={aware * 10} />
              </div>
              <div className="diff-stat">
                <div className="diff-stat-label">Decision quality</div>
                <div className="diff-stat-row">
                  <span className="diff-stat-val">{decQ.toFixed(2)}</span>
                  <span className="diff-stat-unit">/ 10</span>
                </div>
                <DiffBar pct={decQ * 10} />
              </div>
              <div className="diff-stat">
                <div className="diff-stat-label">Estimated TTK Δ</div>
                <div className="diff-stat-row">
                  <span className="diff-stat-val">{diff < 5 ? "+" : "−"}{Math.abs((diff - 5) * 7).toFixed(0)}%</span>
                  <span className="diff-stat-unit">vs. baseline</span>
                </div>
                <DiffBar pct={Math.abs(diff - 5) * 20} />
              </div>
            </div>

            <div style={{ margin: '24px 0 12px', display: 'flex', alignItems: 'baseline', gap: 10 }}>
              <div style={{ fontSize: 13, fontWeight: 600 }}>Combat Role Presets</div>
              <div style={{ fontSize: 12, color: 'var(--ink-3)' }}>One-click archetypes — overwrite the values in sections 01–03.</div>
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
          </section>
          </div>}

          {/* Floating action bar */}
          <FloatingActionBar />
        </div>
      </main>
    </div>
  );
}

export default App;
