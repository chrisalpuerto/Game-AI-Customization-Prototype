import { Entity } from './entity'
import { FruitNode, EntityState } from './types'
import { Vector } from './vector'

const PIXELS_PER_VINE = 10  // dxdv in C++

export class Plant extends Entity {
  // DNA
  growthDir: Vector
  segmentLength: number
  waterSuction: number
  nutrientSuction: number
  engGrReq: number       // energy required per vine length unit

  // Resources
  water: number
  nutrients: number
  energy: number

  // State
  vineLength: number
  numSegments: number
  mature: boolean

  landId: number | null
  fruitNodes: FruitNode[]

  constructor(x: number, y: number, dirX: number, dirY: number, segmentLength: number) {
    super('plant')
    this.x = x; this.y = y
    this.radius = 7
    this.mass = 100
    this.maxSpeed = 30

    const dir = new Vector(dirX, dirY, 0)
    dir.makeUnit()
    this.growthDir = dir
    this.segmentLength = segmentLength
    this.waterSuction = 5
    this.nutrientSuction = 3
    this.engGrReq = 2

    this.water = 0
    this.nutrients = 0
    this.energy = 0
    this.vineLength = 0
    this.numSegments = 0
    this.mature = false

    this.landId = null
    this.fruitNodes = []
  }

  grow(world: import('./world').World): void {
    this.osmosis(world)
    this.absorbNutrients(world)
    this.photosynthesis(world)

    if (!this.mature && this.energy >= this.engGrReq) {
      const nextVine = this.vineLength + 1
      const nx = this.x + this.growthDir.x * nextVine * PIXELS_PER_VINE
      const ny = this.y + this.growthDir.y * nextVine * PIXELS_PER_VINE

      if (nx >= 0 && nx < world.width && ny >= 0 && ny < world.height) {
        this.energy -= this.engGrReq
        this.vineLength++

        const segs = Math.floor(this.vineLength / this.segmentLength)
        if (segs > this.numSegments) {
          this.numSegments++
          const land = world.getLand(nx, ny)
          const fruit = world.spawnFood(nx, ny, true)
          this.fruitNodes.push({
            x: nx, y: ny,
            landId: land ? land.id : null,
            energyRequired: 600,
            energyInvested: 0,
            foodId: fruit.id,
          })
        }
      } else {
        this.mature = true
      }
    }

    for (const node of this.fruitNodes) {
      const fruit = world.getEntityById(node.foodId)
      if (!fruit || !fruit.alive) {
        if (this.energy >= 2) {
          this.energy -= 2
          node.energyInvested += 2
          if (node.energyInvested >= node.energyRequired) {
            node.energyInvested = 0
            const newFruit = world.spawnFood(node.x, node.y, true)
            node.foodId = newFruit.id
          }
        }
      }
    }
  }

  produceSpores(world: import('./world').World): void {
    const angleDeg = 30
    const max = Math.floor(360 / angleDeg)
    for (let i = 0; i < max; i++) {
      const angle = i * angleDeg * Math.PI / 180
      const cursor = new Vector(Math.cos(angle), Math.sin(angle), 0)
      const dx = cursor.scale(this.segmentLength)
      world.spawnSpore(this.x + dx.x, this.y + dx.y, cursor.x, cursor.y, 1)
    }
  }

  // Tip position of the vine (for rendering)
  getTipPoint(): { x: number; y: number } {
    return {
      x: this.x + this.growthDir.x * this.vineLength * PIXELS_PER_VINE,
      y: this.y + this.growthDir.y * this.vineLength * PIXELS_PER_VINE,
    }
  }

  private osmosis(world: import('./world').World): void {
    const land = world.getLandById(this.landId)
    if (land) {
      const before = land.moisture
      land.changeMoisture(-this.waterSuction)
      this.water += before - land.moisture
    }
    for (const node of this.fruitNodes) {
      const nLand = world.getLandById(node.landId)
      if (nLand) {
        const before = nLand.moisture
        nLand.changeMoisture(-this.waterSuction / 10)
        this.water += before - nLand.moisture
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
    for (const node of this.fruitNodes) {
      const nLand = world.getLandById(node.landId)
      if (nLand) {
        const before = nLand.nutrients
        nLand.changeNutrients(-this.nutrientSuction / 10)
        this.nutrients += before - nLand.nutrients
      }
    }
  }

  private photosynthesis(world: import('./world').World): void {
    const land = world.getLandById(this.landId)
    if (land && land.sunlight > 60) {
      if (this.water >= 3 && this.nutrients >= 5) {
        this.water -= 3
        this.nutrients -= 5
        this.energy += 10
      }
    }
  }

  override toJSON(): EntityState {
    return {
      ...super.toJSON(),
      type: 'plant',
      energy: this.energy,
      vineLength: this.vineLength,
      mature: this.mature,
    }
  }
}
