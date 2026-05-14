import { Vector } from './vector'
import { Attachment } from './types'

let _nextId = 0

const G = 200000.0
const ELECTRIC_NORMAL = 100.0
const ELECTRIC_GENTLE = 1.0

export class Particle {
  id: string
  radius: number
  mass: number
  charge: number
  maxSpeed: number

  x: number; y: number; z: number
  vx: number; vy: number; vz: number
  ax: number; ay: number; az: number

  gravityException: boolean
  ignoreCollision: boolean
  ignoreElectricity: boolean
  gentleElectricMode: boolean

  attachments: Attachment[]

  constructor() {
    this.id = String(++_nextId)
    this.radius = 10
    this.mass = 1
    this.charge = 1
    this.maxSpeed = 100

    this.x = 0; this.y = 0; this.z = 0
    this.vx = 0; this.vy = 0; this.vz = 0
    this.ax = 0; this.ay = 0; this.az = 0

    this.gravityException = false
    this.ignoreCollision = false
    this.ignoreElectricity = false
    this.gentleElectricMode = false

    this.attachments = []
  }

  get speed(): number {
    return Math.sqrt(this.vx ** 2 + this.vy ** 2 + this.vz ** 2)
  }

  getVel(): Vector { return new Vector(this.vx, this.vy, this.vz) }
  setVel(v: Vector): void { this.vx = v.x; this.vy = v.y; this.vz = v.z }
  getAcc(): Vector { return new Vector(this.ax, this.ay, this.az) }
  setAcc(v: Vector): void { this.ax = v.x; this.ay = v.y; this.az = v.z }

  move(dt: number): void {
    this.x += this.vx * dt
    this.y += this.vy * dt
    this.z += this.vz * dt
  }

  accelerate(dt: number): void {
    this.vx += this.ax * dt
    this.vy += this.ay * dt
    this.vz += this.az * dt
    this.ax = 0; this.ay = 0; this.az = 0

    for (const att of this.attachments) {
      const dxv = new Vector(att.px - this.x, att.py - this.y, att.pz - this.z)
      const dist = dxv.magnitude()
      if (dist >= att.length) {
        const R = dxv.normalize()
        const negR = R.scale(-1)
        const vel = this.getVel()
        const vopp = vel.dot(negR)
        const velOpp = negR.scale(vopp)
        const newVel = vel.sub(velOpp)
        this.setVel(newVel)
      }
    }
  }

  applyForce(fx: number, fy: number, fz: number): void {
    if (this.speed >= this.maxSpeed) return

    let force = new Vector(fx, fy, fz)
    const active: number[] = []

    for (let i = 0; i < this.attachments.length; i++) {
      const att = this.attachments[i]
      const dxv = new Vector(att.px - this.x, att.py - this.y, att.pz - this.z)
      const dist = dxv.magnitude()

      if (dist >= att.length) {
        const R = dxv.normalize()
        const negR = R.scale(-1)
        const fOpp = force.dot(negR)

        if (fOpp > 0) {
          const diff = att.strength - fOpp
          if (diff >= 0) {
            // attachment holds — cancel opposing component and halt velocity along axis
            force = force.add(R.scale(fOpp))
            const vel = this.getVel()
            const vopp = vel.dot(negR)
            this.setVel(vel.sub(negR.scale(vopp)))
            break
          } else {
            // attachment is overcome
            force = force.add(R.scale(att.strength))
            active.push(i)
          }
        }
      }
    }

    // break overcome attachments
    if (active.length > 0) {
      this.attachments = this.attachments.filter((_, i) => !active.includes(i))
    }

    const da = force.divide(this.mass)
    if (Math.abs(da.magnitude() * 1000) > 0) {
      this.ax += da.x; this.ay += da.y; this.az += da.z
    }
  }

  applyForceVec(f: Vector): void {
    this.applyForce(f.x, f.y, f.z)
  }

  forceStop(): void { this.vx = 0; this.vy = 0; this.vz = 0 }

  collide(massOther: number, vxOther: number, vyOther: number): void {
    if (this.ignoreCollision) return
    const m1 = this.mass, m2 = massOther
    this.vx = (this.vx * m1 + vxOther * m2 - m2 * (this.vx - vxOther)) / (m1 + m2)
    this.vy = (this.vy * m1 + vyOther * m2 - m2 * (this.vy - vyOther)) / (m1 + m2)
  }

  applyGravity(b: Particle): void {
    if (this.gravityException) return
    const dx = b.x - this.x, dy = b.y - this.y, dz = b.z - this.z
    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz)
    if (dist === 0) return
    const f = G * this.mass * b.mass / (dist * dist)
    const acc = f / this.mass
    this.ax += (dx / dist) * acc
    this.ay += (dy / dist) * acc
    this.az += (dz / dist) * acc
  }

  applyElectricity(b: Particle): void {
    if (this.ignoreElectricity) return
    const dx = b.x - this.x, dy = b.y - this.y, dz = b.z - this.z
    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz)
    if (dist === 0) return
    if (dist < (this.radius + b.radius)) {
      const cp = this.charge * b.charge
      if (cp > 0) {
        const E = this.gentleElectricMode ? ELECTRIC_GENTLE : ELECTRIC_NORMAL
        const aScalar = E * cp / (dist * dist) / this.mass
        if (this.gentleElectricMode) {
          this.vx = 0; this.vy = 0; this.vz = 0
        }
        this.ax += (dx / dist) * aScalar
        this.ay += (dy / dist) * aScalar
        this.az += (dz / dist) * aScalar
      }
    }
  }

  attachToPoint(px: number, py: number, pz: number, strength: number, length: number): void {
    this.attachments.push({ px, py, pz, strength, length })
  }

  containsPoint(px: number, py: number, pz = 0): boolean {
    const dx = px - this.x, dy = py - this.y, dz = pz - this.z
    return Math.sqrt(dx * dx + dy * dy + dz * dz) <= this.radius
  }
}
