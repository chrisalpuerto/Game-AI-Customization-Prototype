export interface Vec2 { x: number; y: number }
export interface Vec3 { x: number; y: number; z: number }
export interface Rect { left: number; top: number; right: number; bottom: number }

export type EntityType = 'food' | 'cell' | 'plant' | 'spore' | 'barrier'
export type SporeMode = 'dormant' | 'growing'

export interface Attachment {
  px: number; py: number; pz: number
  strength: number
  length: number
}

export interface Root {
  x: number; y: number
  landId: number
  waterSuction: number
  nutrientSuction: number
}

export interface FruitNode {
  x: number; y: number
  landId: number | null
  energyRequired: number
  energyInvested: number
  foodId: string | null
}

export interface PerceptualObject {
  entityId: string
  type: EntityType
  distance: number
  angle: number
  moving: boolean
  speed: number
  dirX: number
  dirY: number
}

export interface PushRecord {
  objectType: string
  pushability: number
  confidence: number
  trials: number
}

export interface Conclusion {
  actionType: number
  actionName: string
  objectType: string
  subject: string
  results: string[]
}

export interface LandState {
  id: number; col: number; row: number
  x: number; y: number; width: number; height: number
  sunlight: number; nutrients: number; moisture: number
  temperature: number; fertility: number
}

export interface EntityState {
  id: string; type: EntityType; alive: boolean
  x: number; y: number
  vx: number; vy: number
  radius: number; mass: number
  energy?: number
  hungry?: boolean; bored?: boolean; confused?: boolean
  sightDirX?: number; sightDirY?: number
  pushDb?: PushRecord[]
  conclusions?: Conclusion[]
  vineLength?: number; mature?: boolean
  mode?: SporeMode
}

export interface WeatherState {
  wind: { x: number; y: number; width: number; height: number; dirX: number; dirY: number; magnitude: number }
  rain: { x: number; y: number; width: number; height: number; magnitude: number }
  sun:  { x: number; y: number; width: number; height: number; magnitude: number }
}

export interface WorldState {
  time: number
  width: number; height: number
  entities: EntityState[]
  lands: LandState[]
  weather: WeatherState
}
