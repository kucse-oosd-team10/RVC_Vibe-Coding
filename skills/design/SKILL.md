# Design Skill

## Purpose

Use this skill when designing the software structure of the RVC controller before implementation.

The objective of this stage is to transform requirements into maintainable object-oriented software design artifacts.

---

## Responsibilities

During this stage:

- Define class responsibilities
- Design interfaces
- Design object relationships
- Define module boundaries
- Design controller flow
- Design state-machine behavior
- Define dependencies
- Refine software architecture

Implementation should not begin before sufficient design artifacts exist.

---

## Design Principles

Apply object-oriented and maintainable design principles:

- Single Responsibility Principle
- Separation of concerns
- Low coupling
- High cohesion
- Explicit state transitions
- Readability and maintainability

---

## State Machine Guidance

The RVC controller behavior should be modeled as a state machine whenever practical.

Possible states may include:

- Idle
- Cleaning
- ObstacleAvoidance
- BackwardMovement
- PowerBoostCleaning

State transitions should be clearly documented.

---

## Recommended Design Artifacts

Possible outputs include:

- Class diagrams
- State diagrams
- Sequence diagrams
- Module descriptions
- Interface definitions
- Dependency diagrams
- Design rationale

---

## Verification Checklist

Before completing this stage:

- Are responsibilities clearly separated?
- Are dependencies reasonable?
- Are interfaces understandable?
- Are states and transitions defined?
- Is the design traceable to requirements?
- Is the design implementable?

---

## Communication Rules

When reporting results:

- Explain design decisions and rationale.
- Distinguish confirmed design from optional ideas.
- Avoid unnecessary implementation detail.