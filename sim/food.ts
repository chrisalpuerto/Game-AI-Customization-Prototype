import { Entity } from './entity'
import { EntityState } from './types'

export class Food extends Entity {
  energy: number

  constructor(x: number, y: number) {
    super('food')
    this.x = x; this.y = y
    this.radius = 10
    this.mass = 20
    this.energy = 1000
    this.charge = 1
    this.maxSpeed = 50
  }

  override toJSON(): EntityState {
    return { ...super.toJSON(), type: 'food', energy: this.energy }
  }
}
