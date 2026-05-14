import { Entity } from './entity'
import { SporeMode, Root, EntityState } from './types'
import { Vector } from './vector'
import { Land } from './land'

export class Spore extends Entity {
  water: number
  nutrients: number
  energy: number

  readonly requiredEnergy = 250
  readonly rootEnergy = 100

  waterSuction: number
  nutrientSuction: number

  mode: SporeMode
  lifeTime: number

  growthDir: Vector
  segLength: number
  rootPropDist: number

  roots: Root[]

  landId: number | null
  shouldRoot: number
  attemptedRootFailed: boolean

  constructor(x: number, y: number) {
    super('spore')
    this.x = x; this.y = y
    this.radius = 5
    this.mass = 5
    this.maxSpeed = 30
    this.charge = 1

    this.water = 0
    this.nutrients = 0
    this.energy = 0
    this.waterSuction = 5
    this.nutrientSuction = 5

    this.mode = 'dormant'
    this.lifeTime = 0

    this.growthDir = new Vector(1, 1, 0)
    this.growthDir.makeUnit()
    this.segLength = 4
    this.rootPropDist = 10

    this.roots = []
    this.landId = null
    this.shouldRoot = 0
    this.attemptedRootFailed = false
  }

  simulate(world: import('./world').World): void {
    if (this.lifeTime > 1000) {
      this.alive = false
      return
    }

    if (this.mode === 'dormant') {
      const land = world.getLand(this.x, this.y)
      if (land && land.fertility > 0.7) {
        this.germinate(land)
      }
    } else {
      this.forceStop()
      this.absorbNutrients(world)
      this.osmosis(world)
      this.produceEnergy()

      if (this.energy >= this.requiredEnergy) {
        this.attemptedRootFailed = false
        world.spawnPlant(this.x, this.y, this.growthDir.x, this.growthDir.y, this.segLength)
        this.alive = false
        return
      }

      const land = world.getLand(this.x, this.y)
      if (land && land.fertility < 0) {
        const prop = this.energy / this.requiredEnergy
        if (prop < 0.5) this.shouldRoot += 0.05
        if (this.shouldRoot > 0.75) {
          if (!this.growRoot(world)) this.attemptedRootFailed = true
        }
      }
      if (this.attemptedRootFailed) {
        if (this.growRoot(world)) {
          this.attemptedRootFailed = false
          this.shouldRoot = 0
        }
      }
    }
  }

  private germinate(land: Land): void {
    if (this.mode !== 'dormant') return
    this.mode = 'growing'
    this.landId = land.id
    this.attachToPoint(this.x, this.y, 0, 1000, 0)

    const spd = this.speed
    if (spd > 0) {
      this.growthDir = this.getVel().normalize()
    }
    this.forceStop()
  }

  private osmosis(world: import('./world').World): void {
    const land = world.getLandById(this.landId)
    if (land) {
      const before = land.moisture
      land.changeMoisture(-this.waterSuction)
      this.water += before - land.moisture
    }
    for (const root of this.roots) {
      const rLand = world.getLandById(root.landId)
      if (rLand) {
        const before = rLand.moisture
        rLand.changeMoisture(-root.waterSuction)
        this.water += before - rLand.moisture
      }
    }
  }

  private absorbNutrients(world: import('./world').World): void {
    const land = world.getLandById(this.landId)
    if (land) {
      const before = land.nutrients
      land.changeNutrients(-this.nutrientSuction)
      this.nutrients += before - land.nutrients
    }
    for (const root of this.roots) {
      const rLand = world.getLandById(root.landId)
      if (rLand) {
        const before = rLand.nutrients
        rLand.changeNutrients(-root.nutrientSuction)
        this.nutrients += before - rLand.nutrients
      }
    }
  }

  private produceEnergy(): void {
    // 10 water + 15 nutrients -> 5 energy
    if (this.water >= 10 && this.nutrients >= 15) {
      this.water -= 10
      this.nutrients -= 15
      this.energy += 5
    }
  }

  private growRoot(world: import('./world').World): boolean {
    if (this.energy < this.rootEnergy) return false

    const dx = this.growthDir.scale(this.rootPropDist)
    const nx = this.x + dx.x
    const ny = this.y + dx.y

    if (nx < 0 || nx >= world.width || ny < 0 || ny >= world.height) {
      this.growthDir.rotateInPlace2D(5 * Math.PI / 180)
      return false
    }

    const land = world.getLand(nx, ny)
    if (!land) {
      this.growthDir.rotateInPlace2D(5 * Math.PI / 180)
      return false
    }

    this.energy -= this.rootEnergy
    this.roots.push({ x: nx, y: ny, landId: land.id, waterSuction: this.waterSuction, nutrientSuction: this.nutrientSuction })
    this.growthDir.rotateInPlace2D(5 * Math.PI / 180)
    return true
  }

  override toJSON(): EntityState {
    return {
      ...super.toJSON(),
      type: 'spore',
      energy: this.energy,
      mode: this.mode,
    }
  }
}
