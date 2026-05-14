import { Vector } from './vector'

// 2D rotation in the XY plane — replaces myMatrix RotXY_3x3.
// Angle in radians. Returns a new vector; does not mutate.
export function rotate2D(v: Vector, angle: number): Vector {
  return v.rotate2D(angle)
}

// Determinant of a 3×3 matrix supplied as a flat 9-element row-major array.
// Used by barrier's perpendicular-finding algorithm (Cramer's rule port).
export function det3x3(m: number[]): number {
  return (
    m[0] * (m[4] * m[8] - m[5] * m[7]) -
    m[1] * (m[3] * m[8] - m[5] * m[6]) +
    m[2] * (m[3] * m[7] - m[4] * m[6])
  )
}
