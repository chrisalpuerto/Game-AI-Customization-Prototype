import { Land } from './land'
import { Barrier } from './barrier'
import { Food } from './food'
import { Cell } from './cell'
import { Plant } from './plant'
import { Spore } from './spore'
import { Entity } from './entity'
import { WeatherState, WorldState } from './types'
import { Vector } from './vector'

export interface WorldConfig {
  width: number
  height: number
  numColumns: number
  numRows: number
  deltaTime?: number
}

interface WindState {
  x: number; y: number; width: number; height: number
  dirX: number; dirY: number
  magnitude: number
  maxMagnitude: number
}

interface RainState {
  x: number; y: number; width: number; height: number
  magnitude: number
  maxMagnitude: number
}

interface SunState {
  x: number; y: number; width: number; height: number
  magnitude: number
}

const MAX_TEMPERATURE = 100

function isOverlapping(
  ax: number, ay: number, aw: number, ah: number,
  bx: number, by: number, bw: number, bh: number,
): boolean {
  const p = ax <= bx && bx <= ax + aw
  const q = ay <= by + bh && by + bh <= ay + ah
  const r = ay <= by && by <= ay + ah
  const s = ax <= bx + bw && bx + bw <= ax + aw
  return (q || r) && (p || s)
}

function calcWind(time: number, worldWidth: number, worldHeight: number, wind: WindState): void {
  const interval = 60, halfInterval = 30
  const approxLoc = Math.trunc(time * 10) % (interval * 10)
  const hshift = approxLoc - halfInterval * 10
  const prop = 1 - Math.abs(hshift) / (halfInterval * 10)

  wind.x = (worldWidth - wind.width) * prop
  wind.y = (Math.trunc(time / halfInterval) % (Math.trunc(worldHeight / wind.height) + 1)) * wind.height
  wind.magnitude = wind.maxMagnitude * prop
  wind.dirX = 1; wind.dirY = 0
}

function calcRain(time: number, worldWidth: number, worldHeight: number, rain: RainState): void {
  const interval = 60, halfInterval = 30
  const approxLoc = Math.trunc(time * 10) % (interval * 10)
  const hshift = approxLoc - halfInterval * 10
  const prop = 1 - Math.abs(hshift) / (halfInterval * 10)

  rain.x = (Math.trunc(time / halfInterval) % (Math.trunc(worldWidth / rain.width) + 1)) * rain.width
  rain.y = (worldHeight - rain.height) * prop
  rain.magnitude = rain.maxMagnitude * prop
}

function calcSun(time: number, worldWidth: number, sun: SunState): void {
  const interval = 240
  const sunWidth = 2 * worldWidth
  const dxdt = (worldWidth + sunWidth) / interval
  sun.x = worldWidth - (Math.trunc(time) % interval) * dxdt
  sun.y = 0
  sun.width = sunWidth
  sun.magnitude = 80
}

export class World {
  readonly width: number
  readonly height: number
  readonly numColumns: number
  readonly numRows: number
  readonly landWidth: number
  readonly landHeight: number
  readonly deltaTime: number

  worldTime: number

  lands: Land[]
  landById: Map<number, Land>

  entities: Map<string, Entity>
  barriers: Barrier[]

  wind: WindState
  rain: RainState
  sun: SunState

  constructor(config: WorldConfig) {
    this.width = config.width
    this.height = config.height
    this.numColumns = config.numColumns
    this.numRows = config.numRows
    this.deltaTime = config.deltaTime ?? 1 / 30

    this.worldTime = 0
    this.entities = new Map()
    this.barriers = []
    this.landById = new Map()

    const colRem = this.width % this.numColumns
    const rowRem = this.height % this.numRows
    const plotW = Math.floor((this.width - colRem) / this.numColumns)
    const plotH = Math.floor((this.height - rowRem) / this.numRows)
    this.landWidth = plotW
    this.landHeight = plotH

    this.lands = []
    let landId = 0
    for (let col = 0; col < this.numColumns; col++) {
      for (let row = 0; row < this.numRows; row++) {
        const w = plotW + (col === this.numColumns - 1 ? colRem : 0)
        const h = plotH + (row === this.numRows - 1 ? rowRem : 0)
        const land = new Land(landId++, col, row, col * plotW, row * plotH, w, h)
        land.nutrients = 100
        land.temperature = 50
        land.moisture = 50
        land.nutRegen = 5.5
        land.midTemp = row % 2 === 0 ? 30 : 50
        land.tempRadius = col % 2 === 0 ? 30 : 10
        land.updateFertility()
        this.lands.push(land)
        this.landById.set(land.id, land)
      }
    }

    this.wind = { x: 0, y: 0, width: 200, height: 200, dirX: 1, dirY: 0, magnitude: 0, maxMagnitude: 0 }
    this.rain = { x: 0, y: 0, width: 200, height: 200, magnitude: 0, maxMagnitude: 100 }
    this.sun = { x: 0, y: 0, width: 2 * this.width, height: this.height, magnitude: 0 }
  }

  getLand(px: number, py: number): Land | null {
    if (px < 0 || px >= this.width || py < 0 || py >= this.height) return null
    let col = Math.floor(px / this.landWidth)
    let row = Math.floor(py / this.landHeight)
    if (col >= this.numColumns) col = this.numColumns - 1
    if (row >= this.numRows) row = this.numRows - 1
    const idx = col * this.numRows + row
    return this.lands[idx] ?? null
  }

  getLandById(id: number | null): Land | null {
    if (id === null) return null
    return this.landById.get(id) ?? null
  }

  getEntityById(id: string | null): Entity | null {
    if (!id) return null
    return this.entities.get(id) ?? null
  }

  // ── Spawn methods ─────────────────────────────────────────────────────────

  spawnFood(x: number, y: number, attach = false): Food {
    const food = new Food(x, y)
    if (attach) food.attachToPoint(x, y, 0, 1000, 25)
    this.entities.set(food.id, food)
    return food
  }

  spawnCell(x: number, y: number): Cell {
    const cell = new Cell(x, y, this.width, this.height)
    this.entities.set(cell.id, cell)
    return cell
  }

  spawnPlant(x: number, y: number, dirX: number, dirY: number, segLength: number): Plant {
    const plant = new Plant(x, y, dirX, dirY, segLength)
    plant.attachToPoint(x, y, 0, 1000, 1)
    const land = this.getLand(x, y)
    if (land) plant.landId = land.id
    this.entities.set(plant.id, plant)
    return plant
  }

  spawnSpore(x: number, y: number, dirX: number, dirY: number, initialSpeed: number): Spore {
    const dir = new Vector(dirX, dirY, 0)
    const mag = dir.magnitude()
    if (Math.round(mag * 10) > 10 && mag !== 0) return this.spawnSpore(x, y, 0, 0, 0)

    const spore = new Spore(x, y)
    spore.growthDir = mag > 0 ? dir.normalize() : new Vector(1, 1, 0).normalize()
    if (mag > 0) {
      spore.vx = spore.growthDir.x * initialSpeed
      spore.vy = spore.growthDir.y * initialSpeed
    }
    this.entities.set(spore.id, spore)
    return spore
  }

  spawnLinearBarrier(originX: number, originY: number, c0: number, c1: number, xMin: number, xMax: number): Barrier {
    const b = new Barrier([c0, c1], 1, xMin, xMax, 5, 20, originX, originY)
    this.entities.set(b.id, b)
    this.barriers.push(b)
    return b
  }

  spawnParabolicBarrier(originX: number, originY: number, c0: number, c1: number, c2: number, xMin: number, xMax: number): Barrier {
    const b = new Barrier([c0, c1, c2], 2, xMin, xMax, 5, 20, originX, originY)
    this.entities.set(b.id, b)
    this.barriers.push(b)
    return b
  }

  spawnCubicBarrier(originX: number, originY: number, c0: number, c1: number, c2: number, c3: number, xMin: number, xMax: number): Barrier {
    const b = new Barrier([c0, c1, c2, c3], 3, xMin, xMax, 5, 20, originX, originY)
    this.entities.set(b.id, b)
    this.barriers.push(b)
    return b
  }

  // ── Step ──────────────────────────────────────────────────────────────────

  step(dt?: number): void {
    const stepTime = dt ?? this.deltaTime
    const finalTime = this.worldTime + stepTime

    while (this.worldTime < finalTime) {
      this.worldTime += this.deltaTime
      this.simulateEnvironment()
      this.simulateEntities()
    }
  }

  private simulateEnvironment(): void {
    calcWind(this.worldTime, this.width, this.height, this.wind)
    calcRain(this.worldTime, this.width, this.height, this.rain)
    calcSun(this.worldTime, this.width, this.sun)
    this.simulateLand()
  }

  private simulateLand(): void {
    for (const land of this.lands) {
      land.updateTemperature(this.worldTime)

      // Rain moisture
      if (isOverlapping(this.rain.x, this.rain.y, this.rain.width, this.rain.height, land.x, land.y, land.width, land.height)) {
        land.changeMoisture(this.rain.magnitude)
      }

      // Evaporation
      if (land.temperature > 70) {
        land.changeMoisture(-0.005 * (land.temperature / MAX_TEMPERATURE))
      }
      if (land.temperature > 95) {
        land.changeMoisture(-1 * (land.temperature / MAX_TEMPERATURE))
      }

      // Nutrients regen
      land.changeNutrients(land.nutRegen * this.deltaTime)

      // Sunlight
      if (isOverlapping(this.sun.x, this.sun.y, this.sun.width, this.sun.height, land.x, land.y, land.width, land.height)) {
        land.setSunlight(this.sun.magnitude)
      } else {
        land.setSunlight(50)
      }

      land.updateFertility()
    }
  }

  private simulateEntities(): void {
    const toDelete: string[] = []

    for (const [id, entity] of this.entities) {
      if (!entity.alive) {
        toDelete.push(id)
        continue
      }

      entity.accelerate(this.deltaTime)
      entity.viewMove(this.deltaTime)

      // Barrier collision
      for (const bar of this.barriers) {
        if (bar.alive && bar.isCollision(entity.x, entity.y)) {
          entity.collide(50000, 0, 0)
          if (entity.type === 'cell') (entity as Cell).feelBarrier()
          break
        }
      }

      // World boundary collision
      if (entity.x < 0 || entity.x >= this.width || entity.y < 0 || entity.y >= this.height) {
        entity.collide(300, 0, 0)
        // Clamp position inside bounds
        entity.x = Math.max(0, Math.min(this.width - 1, entity.x))
        entity.y = Math.max(0, Math.min(this.height - 1, entity.y))
      }

      // Per-type simulation
      if (entity.type === 'cell') {
        const cell = entity as Cell
        if (cell.x >= this.width || cell.x <= 0 || cell.y >= this.height || cell.y <= 0) {
          cell.feelBarrier()
        }
        cell.see(this)
        cell.simulate(this)

      } else if (entity.type === 'plant') {
        (entity as Plant).grow(this)

      } else if (entity.type === 'spore') {
        const spore = entity as Spore
        // Apply wind
        if (spore.x >= this.wind.x && spore.x <= this.wind.x + this.wind.width &&
            spore.y >= this.wind.y && spore.y <= this.wind.y + this.wind.height) {
          spore.applyForce(this.wind.dirX * this.wind.magnitude, this.wind.dirY * this.wind.magnitude, 0)
        }
        spore.lifeTime += this.deltaTime
        spore.simulate(this)

      } else if (entity.type === 'food') {
        const food = entity as Food
        if (food.x >= this.wind.x && food.x <= this.wind.x + this.wind.width &&
            food.y >= this.wind.y && food.y <= this.wind.y + this.wind.height) {
          food.applyForce(this.wind.dirX * this.wind.magnitude, this.wind.dirY * this.wind.magnitude, 0)
        }
      }
    }

    for (const id of toDelete) {
      this.entities.delete(id)
    }
  }

  // ── State export ──────────────────────────────────────────────────────────

  getState(): WorldState {
    const weather: WeatherState = {
      wind: { x: this.wind.x, y: this.wind.y, width: this.wind.width, height: this.wind.height, dirX: this.wind.dirX, dirY: this.wind.dirY, magnitude: this.wind.magnitude },
      rain: { x: this.rain.x, y: this.rain.y, width: this.rain.width, height: this.rain.height, magnitude: this.rain.magnitude },
      sun:  { x: this.sun.x,  y: this.sun.y,  width: this.sun.width,  height: this.sun.height,  magnitude: this.sun.magnitude },
    }

    return {
      time: this.worldTime,
      width: this.width,
      height: this.height,
      entities: Array.from(this.entities.values()).map(e => e.toJSON()),
      lands: this.lands.map(l => l.toJSON()),
      weather,
    }
  }
}
