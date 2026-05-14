export class Vector {
  x: number; y: number; z: number
  locX: number; locY: number; locZ: number

  constructor(x = 0, y = 0, z = 0) {
    this.x = x; this.y = y; this.z = z
    this.locX = 0; this.locY = 0; this.locZ = 0
  }

  static zero(): Vector { return new Vector(0, 0, 0) }
  static from(x: number, y: number, z = 0): Vector { return new Vector(x, y, z) }

  add(v: Vector): Vector { return new Vector(this.x + v.x, this.y + v.y, this.z + v.z) }
  sub(v: Vector): Vector { return new Vector(this.x - v.x, this.y - v.y, this.z - v.z) }
  scale(s: number): Vector { return new Vector(this.x * s, this.y * s, this.z * s) }
  divide(s: number): Vector { return new Vector(this.x / s, this.y / s, this.z / s) }

  dot(v: Vector): number { return this.x * v.x + this.y * v.y + this.z * v.z }

  cross(v: Vector): Vector {
    return new Vector(
      this.y * v.z - this.z * v.y,
      this.z * v.x - this.x * v.z,
      this.x * v.y - this.y * v.x,
    )
  }

  magnitude(): number {
    return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z)
  }

  normalize(): Vector {
    const m = this.magnitude()
    if (m === 0) return Vector.zero()
    return this.divide(m)
  }

  makeUnit(): void {
    const m = this.magnitude()
    if (m > 0) { this.x /= m; this.y /= m; this.z /= m }
  }

  clone(): Vector { return new Vector(this.x, this.y, this.z) }

  // Returns angle in [0, 2π) using x,y components — matches C++ get_angle()
  angle(): number {
    const x = this.x, y = this.y
    const pi = Math.PI
    if (x > 0) {
      if (y > 0) return Math.atan(y / x)
      if (y < 0) return 2 * pi + Math.atan(y / x)
      return 0
    }
    if (x < 0) return pi + Math.atan(y / x)
    if (x === 0) {
      if (y > 0) return pi / 2
      if (y < 0) return 3 * pi / 2
    }
    return -1000 // both zero
  }

  // Returns true if any component has opposite sign to the corresponding component of v
  isOpposite(v: Vector): boolean {
    let c = 0
    if ((this.x < 0 && v.x > 0) || (this.x > 0 && v.x < 0)) c++
    if ((this.y < 0 && v.y > 0) || (this.y > 0 && v.y < 0)) c++
    if ((this.z < 0 && v.z > 0) || (this.z > 0 && v.z < 0)) c++
    return c >= 1
  }

  // 2D rotation in XY plane by angle radians (matches C++ RotXY_3x3)
  rotate2D(angle: number): Vector {
    const cos = Math.cos(angle)
    const sin = Math.sin(angle)
    return new Vector(
      this.x * cos - this.y * sin,
      this.x * sin + this.y * cos,
      this.z,
    )
  }

  rotateInPlace2D(angle: number): void {
    const cos = Math.cos(angle)
    const sin = Math.sin(angle)
    const nx = this.x * cos - this.y * sin
    const ny = this.x * sin + this.y * cos
    this.x = nx; this.y = ny
  }
}
