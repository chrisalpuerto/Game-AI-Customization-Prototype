import { Entity } from './entity'
import { EntityState } from './types'

export class Barrier extends Entity {
  coefficients: number[]
  degree: number
  xMin: number; xMax: number
  precision: number
  padding: number
  originX: number; originY: number

  constructor(
    coefficients: number[],
    degree: number,
    xMin: number, xMax: number,
    precision: number,
    padding: number,
    originX: number, originY: number,
  ) {
    super('barrier')
    this.coefficients = coefficients
    this.degree = degree
    this.xMin = xMin; this.xMax = xMax
    this.precision = precision
    this.padding = padding
    this.originX = originX; this.originY = originY
    this.x = originX; this.y = originY
    this.mass = 500
    this.ignoreCollision = true
    this.ignoreElectricity = true
    this.gravityException = true
  }

  // Evaluate polynomial at x: sum of coefficients[i] * x^i
  f(x: number): number {
    let y = 0
    for (let i = 0; i <= this.degree; i++) {
      y += this.coefficients[i] * Math.pow(x, i)
    }
    return y
  }

  getDerivative(x: number): number {
    let dydx = 0
    for (let i = 1; i <= this.degree; i++) {
      dydx += this.coefficients[i] * i * Math.pow(x, i - 1)
    }
    return dydx
  }

  // Translates test point to barrier-local coordinates, then searches for
  // the curve point closest (perpendicular) to the test point.
  // Matches C++ barrier::isCollision() logic exactly.
  isCollision(px: number, py: number): boolean {
    const lx = px - this.originX
    const ly = py - this.originY

    const prec = 0.1
    let found = false
    let curvePx = 0, curvePy = 0

    for (let x = this.xMin; x < this.xMax; x += prec) {
      const dydx = this.getDerivative(x)
      const y = this.f(x)
      const diff = dydx + (x - lx) / (y - ly)
      if (Math.abs(diff) < 0.1) {
        curvePx = x
        curvePy = y
        found = true
        break
      }
    }

    if (found) {
      const dx = curvePx - lx
      const dy = curvePy - ly
      const dist = Math.sqrt(dx * dx + dy * dy)
      return dist <= this.padding
    }
    return false
  }

  // Returns evenly-spaced sample points along the curve (for frontend rendering)
  getCurvePoints(step = 1): Array<{ x: number; y: number }> {
    const pts: Array<{ x: number; y: number }> = []
    for (let x = this.xMin; x < this.xMax; x += step) {
      pts.push({ x: x + this.originX, y: this.f(x) + this.originY })
    }
    return pts
  }

  override toJSON(): EntityState {
    return { ...super.toJSON(), type: 'barrier' }
  }
}
