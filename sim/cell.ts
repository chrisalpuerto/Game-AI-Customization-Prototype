import { Entity } from './entity'
import { PerceptualObject, PushRecord, Conclusion, EntityState } from './types'
import { Vector } from './vector'
import { Food } from './food'

const SIGHT_HEIGHT = 500
const SIGHT_ANGLE_DEG = 50
const SIGHT_ANGLE_RAD = SIGHT_ANGLE_DEG * Math.PI / 180
const MAX_ENERGY = 15000
const INITIAL_ENERGY = 10000
const GRASP_RANGE = 10    // extra reach beyond radius
const PUSH_FORCE = 1000
const OBSERVATION_TICKS = 30
const TURN_TICKS = 20
const TURN_ANGLE_RAD = 4 * Math.PI / 180  // 4° per tick
const BARRIER_PREDICT_DT = 16.65

interface AnalysisObject {
  changedMoving: boolean
  movedIt: boolean
  stopped: boolean
  gotFurther: boolean
  gotCloser: boolean
  speedUp: boolean
  slowedDown: boolean
}

export class Cell extends Entity {
  energy: number

  sightDir: Vector
  worldWidth: number
  worldHeight: number

  awareness: PerceptualObject[]
  graspSpace: PerceptualObject[]

  // Behavioral flags
  hungry: boolean
  feedMode: boolean
  pursuingFood: boolean
  foodWithinGrasp: boolean
  bored: boolean
  explore: boolean
  confused: boolean
  busy: boolean

  // Push curiosity / learning
  pursuingPush: boolean
  pushWithinGrasp: boolean
  pushGoal: PerceptualObject | null
  goalX: number; goalY: number

  pushDb: PushRecord[]
  conclusions: Conclusion[]

  // Observation
  doObserve: boolean
  analyzeObservation: boolean
  ooi: string | null        // entity id of object of interest
  observations: PerceptualObject[]

  // Barrier / maneuver
  seeBarrier: boolean
  hitBarrier: boolean
  respondingToBarrier: boolean
  turnManeuver: boolean
  keepDistanceToBarrier: boolean
  responseTicks: number
  endTick: number

  constructor(x: number, y: number, worldWidth: number, worldHeight: number) {
    super('cell')
    this.x = x; this.y = y
    this.radius = 20
    this.mass = 50
    this.maxSpeed = 30
    this.charge = 1

    this.energy = INITIAL_ENERGY
    this.worldWidth = worldWidth
    this.worldHeight = worldHeight

    this.sightDir = new Vector(1, 0, 0)

    this.awareness = []
    this.graspSpace = []

    this.hungry = false
    this.feedMode = false
    this.pursuingFood = false
    this.foodWithinGrasp = false
    this.bored = true
    this.explore = true
    this.confused = false
    this.busy = false

    this.pursuingPush = false
    this.pushWithinGrasp = false
    this.pushGoal = null
    this.goalX = 0; this.goalY = 0

    this.pushDb = []
    this.conclusions = []

    this.doObserve = false
    this.analyzeObservation = false
    this.ooi = null
    this.observations = []

    this.seeBarrier = false
    this.hitBarrier = false
    this.respondingToBarrier = false
    this.turnManeuver = false
    this.keepDistanceToBarrier = true
    this.responseTicks = 0
    this.endTick = 0
  }

  // ── Atomic actions ────────────────────────────────────────────────────────

  rotateEyes(angle: number): void {
    this.sightDir.rotateInPlace2D(angle)
  }

  moveForward(force: number): void {
    this.depleteEnergy(-1)
    const f = this.sightDir.scale(force)
    this.applyForceVec(f)
  }

  stop(): void { this.forceStop() }

  facePoint(tx: number, ty: number): void {
    const stri_h = this.sightDir.scale(SIGHT_HEIGHT)
    // flip Y to convert from window coords (Y-down) to standard (Y-up)
    const vobj = new Vector(tx - this.x, -(ty - this.y), 0)
    const sth_flipped = new Vector(stri_h.x, -stri_h.y, 0)

    const distToObj = vobj.magnitude()
    if (distToObj === 0) return

    const dot = vobj.dot(sth_flipped)
    let cosArg = dot / SIGHT_HEIGHT / distToObj
    if (Math.abs(1 - cosArg) < 0.0001) cosArg = 1
    cosArg = Math.max(-1, Math.min(1, cosArg))
    let phi = Math.acos(cosArg) * 180 / Math.PI

    const alphaObj = vobj.angle()
    const alphaSight = sth_flipped.angle()
    let diff = (alphaSight - alphaObj) * 180 / Math.PI

    if (Math.round(Math.abs(diff) * 10) <= Math.round(phi * 10)) {
      if (diff > 0) phi *= -1
    } else {
      if (diff < 0) phi *= -1
    }

    // C++ matrix rotates opposite to standard, so negate
    this.rotateEyes(-phi * Math.PI / 180)
  }

  turnShift(ticks: number): void {
    this.stop()
    this.turnManeuver = true
    this.busy = true
    this.responseTicks = 0
    this.endTick = ticks
  }

  depleteEnergy(delta: number): void {
    this.energy += delta
    if (this.energy <= 0) {
      this.alive = false
      return
    }
    const prop = this.energy / MAX_ENERGY
    if (prop <= 0.7) {
      this.hungry = true
      this.bored = false
    }
  }

  // ── Actions on environment ────────────────────────────────────────────────

  tryEat(entityId: string, world: import('./world').World): void {
    const obj = world.getEntityById(entityId)
    if (!obj || !obj.alive || obj.type !== 'food') return
    const food = obj as Food
    this.energy += food.energy
    food.energy = 0
    food.alive = false

    if (this.energy >= MAX_ENERGY) {
      this.hungry = false
      this.feedMode = false
      this.explore = false
      this.bored = true
    }
  }

  tryPush(entityId: string, world: import('./world').World): void {
    const po = this.graspSpace.find(p => p.entityId === entityId)
    if (!po) return
    const obj = world.getEntityById(entityId)
    if (!obj) return
    const f = this.sightDir.scale(PUSH_FORCE)
    obj.applyForceVec(f)
    this.pushWithinGrasp = false
  }

  observeObject(entityId: string): void {
    const po = this.awareness.find(p => p.entityId === entityId)
    if (po) this.observations.push({ ...po })
  }

  feelBarrier(): void { this.clearThinking() }

  // ── Perception ────────────────────────────────────────────────────────────

  see(world: import('./world').World): void {
    this.awareness = []
    this.graspSpace = []

    const stri_h = this.sightDir.scale(SIGHT_HEIGHT)

    for (const [, entity] of world.entities) {
      if (entity === this || !entity.alive) continue

      const dx = entity.x - this.x
      // flip Y for standard-coordinate angle math (mirrors C++ v_obj.tuple[1] *= -1)
      const dy = -(entity.y - this.y)
      const vobj = new Vector(dx, dy, 0)

      const distToObj = vobj.magnitude()
      if (distToObj === 0) continue

      const stri_h_flipped = new Vector(stri_h.x, -stri_h.y, 0)
      const dot = vobj.dot(stri_h_flipped)
      let cosArg = dot / SIGHT_HEIGHT / distToObj
      if (Math.abs(1 - cosArg) < 0.0001) cosArg = 1
      cosArg = Math.max(-1, Math.min(1, cosArg))
      const phi = Math.acos(cosArg) * 180 / Math.PI

      const angleWithin = phi <= SIGHT_ANGLE_DEG
      const withinAwareness = distToObj <= SIGHT_HEIGHT
      const withinGrasp = distToObj <= this.radius + GRASP_RANGE

      if (angleWithin && withinAwareness) {
        const spd = entity.speed
        const po: PerceptualObject = {
          entityId: entity.id,
          type: entity.type,
          distance: distToObj,
          angle: phi,
          moving: Math.round(spd * 1000) > 0,
          speed: spd,
          dirX: entity.vx !== 0 || entity.vy !== 0 ? entity.vx / (spd || 1) : 0,
          dirY: entity.vy !== 0 || entity.vy !== 0 ? entity.vy / (spd || 1) : 0,
        }
        this.awareness.push(po)
        if (withinGrasp) this.graspSpace.push(po)
      }
    }

    // Sort both lists nearest-first (bubble sort matches C++)
    this.awareness.sort((a, b) => a.distance - b.distance)
    this.graspSpace.sort((a, b) => a.distance - b.distance)

    // Check barrier: project velocity 16.65s forward and see if we exit world
    const predX = this.x + this.vx * BARRIER_PREDICT_DT
    const predY = this.y + this.vy * BARRIER_PREDICT_DT
    const predSightX = predX + stri_h.x
    const predSightY = predY + stri_h.y

    const sightEnd = { x: this.x + stri_h.x, y: this.y + stri_h.y }
    this.seeBarrier = this.outsideWorld(sightEnd.x, sightEnd.y) ||
      this.outsideWorld(predSightX, predSightY)

    this.depleteEnergy(-2)
  }

  // ── Mind ──────────────────────────────────────────────────────────────────

  private outsideWorld(px: number, py: number): boolean {
    return px < 0 || px >= this.worldWidth || py < 0 || py >= this.worldHeight
  }

  private clearThinking(): void {
    this.pursuingFood = false
    this.pursuingPush = false
    this.foodWithinGrasp = false
    this.pushWithinGrasp = false
    this.explore = true
    this.confused = false
    this.bored = false
    this.doObserve = false
    this.analyzeObservation = false
    this.observations = []
  }

  private clearThinkingB(): void {
    this.pursuingFood = false
    this.pursuingPush = false
    this.foodWithinGrasp = false
    this.pushWithinGrasp = false
    this.explore = false
    this.confused = false
    this.bored = false
    this.doObserve = false
    this.analyzeObservation = false
    this.observations = []
  }

  private setPushTarget(po: PerceptualObject): void {
    this.stop()
    this.facePoint(po.dirX, po.dirY)  // face direction approximation — refined in think()
    const obj = { x: 0, y: 0 }; // refined below via world in think()
    this.goalX = 0; this.goalY = 0
    this.pushGoal = po
    this.pursuingPush = true
    this.turnManeuver = false
    this.explore = false
  }

  private setPushTargetWithCoords(po: PerceptualObject, tx: number, ty: number): void {
    this.stop()
    this.facePoint(tx, ty)
    this.goalX = tx; this.goalY = ty
    this.pushGoal = po
    this.pursuingPush = true
    this.turnManeuver = false
    this.explore = false
  }

  private framePushCurious(world: import('./world').World): void {
    for (const po of this.awareness) {
      if (po.type === 'cell' || po.type === 'barrier') continue
      if (!this.inPushDb(po.type)) {
        const obj = world.getEntityById(po.entityId)
        if (obj) { this.setPushTargetWithCoords(po, obj.x, obj.y); break }
      } else {
        const record = this.pushDb.find(r => r.objectType === po.type)
        if (record && record.confidence < 0.7) {
          const obj = world.getEntityById(po.entityId)
          if (obj) { this.setPushTargetWithCoords(po, obj.x, obj.y); break }
        }
      }
      if (this.pursuingPush) break
    }
  }

  private framePushOptimize(world: import('./world').World): void {
    let bestIdx = -1
    let bestRank = 0
    for (let i = 0; i < this.pushDb.length; i++) {
      if (this.pushDb[i].pushability > bestRank) {
        bestRank = this.pushDb[i].pushability
        bestIdx = i
      }
    }

    for (const po of this.awareness) {
      if (po.type === 'cell' || po.type === 'barrier') continue
      if (bestIdx === -1) {
        const obj = world.getEntityById(po.entityId)
        if (obj) { this.setPushTargetWithCoords(po, obj.x, obj.y); break }
      } else if (po.type === this.pushDb[bestIdx].objectType) {
        const obj = world.getEntityById(po.entityId)
        if (obj) { this.setPushTargetWithCoords(po, obj.x, obj.y); break }
      }
    }

    if (!this.pursuingPush) {
      for (const po of this.awareness) {
        if (po.type === 'cell' || po.type === 'barrier') continue
        const obj = world.getEntityById(po.entityId)
        if (obj) { this.setPushTargetWithCoords(po, obj.x, obj.y); break }
      }
    }
  }

  think(world: import('./world').World): void {
    if (this.confused) {
      this.stop()
      this.clearThinking()
    }

    // Barrier avoidance — highest priority
    if (this.keepDistanceToBarrier && this.seeBarrier) {
      this.clearThinkingB()
      const predX = this.x + this.vx * BARRIER_PREDICT_DT
      const predY = this.y + this.vy * BARRIER_PREDICT_DT
      if (this.outsideWorld(predX, predY)) {
        this.turnShift(TURN_TICKS)
      }
    }

    // Hunger → feed mode
    if (this.hungry) {
      this.feedMode = true
      this.explore = true
    } else if (!this.busy) {
      this.bored = true
      this.explore = true
    }

    // Execute turn maneuver
    if (this.turnManeuver) {
      this.rotateEyes(TURN_ANGLE_RAD)
      this.responseTicks++
      if (this.responseTicks >= this.endTick) {
        this.responseTicks = 0
        this.turnManeuver = false
        this.busy = false
      }
    }

    // Barrier hit recovery
    if (!this.pursuingFood && !this.foodWithinGrasp && !this.pursuingPush && !this.pushWithinGrasp) {
      if (this.hitBarrier && !this.respondingToBarrier) {
        this.rotateEyes(110 * Math.PI / 180)
        this.hitBarrier = false
        this.respondingToBarrier = true
      } else if (this.respondingToBarrier) {
        this.hitBarrier = false
        this.responseTicks++
        if (this.responseTicks >= 50) {
          this.responseTicks = 0
          this.respondingToBarrier = false
        }
      } else if (this.seeBarrier) {
        this.turnShift(TURN_TICKS)
      }
    }

    if (this.pursuingFood) {
      const dx = this.goalX - this.x, dy = this.goalY - this.y
      const distToFood = Math.sqrt(dx * dx + dy * dy)

      // Switch to closer food if found
      for (const po of this.awareness) {
        if (po.type === 'food' && po.distance < distToFood) {
          const obj = world.getEntityById(po.entityId)
          if (obj) {
            this.stop()
            this.facePoint(obj.x, obj.y)
            this.goalX = obj.x; this.goalY = obj.y
            this.pursuingFood = true
            break
          }
        }
      }

      this.moveForward(100)
      if (distToFood <= this.radius + GRASP_RANGE) {
        this.stop()
        this.foodWithinGrasp = true
        this.pursuingFood = false
      }

    } else if (this.foodWithinGrasp) {
      this.stop()
      for (const po of this.graspSpace) {
        if (po.type === 'food') {
          const obj = world.getEntityById(po.entityId)
          if (obj) this.facePoint(obj.x, obj.y)
          this.tryEat(po.entityId, world)
        }
      }
      this.foodWithinGrasp = false

    } else if (this.pursuingPush) {
      // Re-acquire goal in current awareness
      let matchFound = false
      if (this.pushGoal) {
        for (const po of this.awareness) {
          if (po.type !== this.pushGoal.type) continue
          const obj = world.getEntityById(po.entityId)
          if (!obj) continue
          const dx = obj.x - this.goalX, dy = obj.y - this.goalY
          if (Math.sqrt(dx * dx + dy * dy) <= 10) {
            this.goalX = obj.x; this.goalY = obj.y
            this.pushGoal = po
            matchFound = true
            break
          }
        }
      }

      if (!matchFound) {
        this.confused = true
      }

      this.facePoint(this.goalX, this.goalY)
      if (this.pushGoal?.moving) {
        this.moveForward(300)
      } else {
        this.moveForward(100)
      }

      if (this.pushGoal && this.pushGoal.distance <= this.radius + 20) {
        this.pushWithinGrasp = true
        this.pursuingPush = false
      }

    } else if (this.pushWithinGrasp) {
      if (this.pushGoal) {
        for (const po of this.graspSpace) {
          if (po.type === this.pushGoal.type) {
            const obj = world.getEntityById(po.entityId)
            if (obj) {
              this.facePoint(obj.x, obj.y)
              this.observeObject(po.entityId)
              this.doObserve = true
              this.bored = false
              this.ooi = po.entityId
              this.responseTicks = 0
              this.endTick = OBSERVATION_TICKS
              this.tryPush(po.entityId, world)
            }
            break
          }
        }
      }
      this.pushWithinGrasp = false

    } else {
      if (this.hungry) { this.feedMode = true; this.explore = true }

      // Frame perception
      if (this.bored) this.framePushCurious(world)

      if (this.feedMode) {
        for (const po of this.awareness) {
          if (po.type === 'food') {
            const obj = world.getEntityById(po.entityId)
            if (obj) {
              this.stop()
              this.facePoint(obj.x, obj.y)
              this.goalX = obj.x; this.goalY = obj.y
              this.pursuingFood = true
              this.turnManeuver = false
            }
            break
          }
        }
      }

      // Observation loop
      if (this.doObserve && !this.seeBarrier) {
        this.stop()
        if (this.ooi) {
          const obj = world.getEntityById(this.ooi)
          if (obj) {
            this.facePoint(obj.x, obj.y)
            this.observeObject(this.ooi)
          } else {
            this.confused = true
          }
        } else {
          this.confused = true
        }
        this.responseTicks++
        if (this.responseTicks >= this.endTick) {
          this.doObserve = false
          this.analyzeObservation = true
        }
      } else if (this.analyzeObservation) {
        this.analyze()
        this.analyzeObservation = false
      } else if (this.explore) {
        if (this.turnManeuver) {
          this.rotateEyes(TURN_ANGLE_RAD)
          this.moveForward(120)
          this.responseTicks++
          if (this.responseTicks >= 20) {
            this.responseTicks = 0
            this.turnManeuver = false
          }
        } else {
          this.moveForward(100)
          this.bored = true
        }
      } else if (this.bored) {
        this.explore = true
      } else {
        this.bored = true
      }
    }

    this.depleteEnergy(-2)
  }

  private analyze(): void {
    if (this.observations.length < 2) { this.observations = []; return }

    const analysis: AnalysisObject[] = []
    for (let i = 0; i < this.observations.length - 1; i++) {
      const before = this.observations[i]
      const after = this.observations[i + 1]

      const a: AnalysisObject = {
        changedMoving: before.moving !== after.moving,
        movedIt: !before.moving && after.moving,
        stopped: before.moving && !after.moving,
        gotFurther: Math.round((after.distance - before.distance) * 1000) > 0,
        gotCloser: Math.round((after.distance - before.distance) * 1000) < 0,
        speedUp: Math.round((after.speed - before.speed) * 1000) > 0,
        slowedDown: Math.round((after.speed - before.speed) * 1000) < 0,
      }
      analysis.push(a)
    }

    let countFurther = 0, countNoChange = 0
    let eventCausedMovement = false, objectReachedStop = false

    for (const a of analysis) {
      if (a.movedIt) eventCausedMovement = true
      if (eventCausedMovement && a.gotFurther) countFurther++
      if (a.stopped) objectReachedStop = true
      if (!a.changedMoving && !a.gotCloser && !a.gotFurther && !a.slowedDown && !a.speedUp) {
        countNoChange++
      }
    }

    const numObs = analysis.length
    const propFurther = countFurther / numObs

    if (this.ooi) {
      const ooiType = this.observations[0]?.type
      if (ooiType) {
        const existing = this.pushDb.find(r => r.objectType === ooiType)
        if (existing) {
          existing.pushability = (existing.trials * existing.pushability + propFurther) / (existing.trials + 1)
          existing.trials++
          existing.confidence = existing.trials / 100
        } else {
          this.pushDb.push({ objectType: ooiType, pushability: propFurther, confidence: 1 / 100, trials: 1 })
        }

        const results: string[] = []
        if (eventCausedMovement) results.push('to move')
        if (objectReachedStop) results.push('to stop')
        if (!eventCausedMovement && !objectReachedStop) results.push('no result')
        this.conclusions.push({ actionType: 0, actionName: 'push', objectType: ooiType, subject: 'I', results })
      }
    }

    this.observations = []
  }

  private inPushDb(type: string): boolean {
    return this.pushDb.some(r => r.objectType === type)
  }

  simulate(world: import('./world').World): void {
    this.think(world)
  }

  override toJSON(): EntityState {
    return {
      ...super.toJSON(),
      type: 'cell',
      energy: this.energy,
      hungry: this.hungry,
      bored: this.bored,
      confused: this.confused,
      sightDirX: this.sightDir.x,
      sightDirY: this.sightDir.y,
      pushDb: this.pushDb,
      conclusions: this.conclusions,
    }
  }
}
