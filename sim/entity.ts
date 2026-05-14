import { Particle } from './particle'
import { EntityType, EntityState } from './types'

export abstract class Entity extends Particle {
  type: EntityType
  alive: boolean

  constructor(type: EntityType) {
    super()
    this.type = type
    this.alive = true
  }

  viewMove(dt: number): void {
    this.move(dt)
  }

  toJSON(): EntityState {
    return {
      id: this.id,
      type: this.type,
      alive: this.alive,
      x: this.x,
      y: this.y,
      vx: this.vx,
      vy: this.vy,
      radius: this.radius,
      mass: this.mass,
    }
  }
}
