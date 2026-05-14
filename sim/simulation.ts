import { World, WorldConfig } from './world'
import { WorldState } from './types'

export type { WorldState, WorldConfig }
export type { EntityState, LandState, WeatherState, PerceptualObject, PushRecord, Conclusion } from './types'
export { World } from './world'
export { Cell } from './cell'
export { Food } from './food'
export { Plant } from './plant'
export { Spore } from './spore'
export { Barrier } from './barrier'
export { Land } from './land'
export { Vector } from './vector'

export interface SimulationConfig {
  width: number
  height: number
  columns: number
  rows: number
  deltaTime?: number
}

export interface Simulation {
  world: World
  step(dt?: number): void
  getState(): WorldState
  reset(): void
}

export function createSimulation(config: SimulationConfig): Simulation {
  const worldConfig: WorldConfig = {
    width: config.width,
    height: config.height,
    numColumns: config.columns,
    numRows: config.rows,
    deltaTime: config.deltaTime,
  }

  let world = new World(worldConfig)

  return {
    get world() { return world },

    step(dt?: number): void {
      world.step(dt)
    },

    getState(): WorldState {
      return world.getState()
    },

    reset(): void {
      world = new World(worldConfig)
    },
  }
}

// ── Usage example (run with: npx tsx sim/simulation.ts) ─────────────────────
// Uncomment to run as a standalone script:
//
// const sim = createSimulation({ width: 1200, height: 800, columns: 6, rows: 4 })
//
// sim.world.spawnCell(300, 400)
// sim.world.spawnFood(500, 300)
// sim.world.spawnFood(700, 200)
// sim.world.spawnPlant(100, 600, 0, -1, 50)
//
// for (let i = 0; i < 300; i++) sim.step(1 / 30)
//
// const state = sim.getState()
// console.log('World time:', state.time.toFixed(2), 's')
// console.log('Entities:', state.entities.map(e => `${e.type}@(${Math.round(e.x)},${Math.round(e.y)})`).join('  '))
// const cell = state.entities.find(e => e.type === 'cell')
// if (cell) console.log('Cell energy:', cell.energy?.toFixed(0), '  hungry:', cell.hungry)
