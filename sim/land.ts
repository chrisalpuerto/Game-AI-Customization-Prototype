import { LandState } from './types'

const MAX_NUTRIENTS = 500
const MAX_MOISTURE = 100
const MAX_TEMPERATURE = 100

export class Land {
  id: number
  col: number; row: number
  x: number; y: number
  width: number; height: number

  sunlight: number       // 0–100
  nutrients: number      // 0–500
  moisture: number       // 0–100
  temperature: number    // 0–100
  fertility: number      // 0–1

  nutRegen: number       // nutrients regenerated per second
  midTemp: number        // temperature midpoint for sine oscillation
  tempRadius: number     // amplitude of temperature sine wave

  constructor(id: number, col: number, row: number, x: number, y: number, width: number, height: number) {
    this.id = id
    this.col = col; this.row = row
    this.x = x; this.y = y
    this.width = width; this.height = height

    this.sunlight = 0
    this.nutrients = 0
    this.moisture = 0
    this.temperature = 0
    this.fertility = 0

    this.nutRegen = 5.5
    this.midTemp = 50
    this.tempRadius = 10
  }

  setSunlight(value: number): void {
    this.sunlight = value
  }

  changeNutrients(delta: number): void {
    this.nutrients = Math.max(0, Math.min(MAX_NUTRIENTS, this.nutrients + delta))
  }

  changeMoisture(delta: number): void {
    this.moisture = Math.max(0, Math.min(MAX_MOISTURE, this.moisture + delta))
  }

  updateTemperature(time: number): void {
    const c = 1
    this.temperature = Math.max(0, Math.min(MAX_TEMPERATURE, this.tempRadius * Math.sin(time * c) + this.midTemp))
  }

  updateFertility(): void {
    this.fertility = Math.max(0, Math.min(1, (this.nutrients / MAX_NUTRIENTS + this.moisture / MAX_MOISTURE) / 2))
  }

  pointInLand(px: number, py: number): boolean {
    return px >= this.x && px <= this.x + this.width && py >= this.y && py <= this.y + this.height
  }

  toJSON(): LandState {
    return {
      id: this.id,
      col: this.col, row: this.row,
      x: this.x, y: this.y,
      width: this.width, height: this.height,
      sunlight: this.sunlight,
      nutrients: this.nutrients,
      moisture: this.moisture,
      temperature: this.temperature,
      fertility: this.fertility,
    }
  }
}
