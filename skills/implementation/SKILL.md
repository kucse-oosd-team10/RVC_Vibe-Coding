# Implementation Skill

## Purpose

Use this skill when implementing approved requirements and approved designs.

The objective of this stage is to produce readable, maintainable, traceable implementation code.

---

## Responsibilities

During this stage:

- Implement approved functionality
- Preserve design intent
- Maintain traceability
- Keep code maintainable
- Minimize unnecessary complexity

---

## Implementation Rules

- Implement only approved requirements.
- Avoid feature creep.
- Do not introduce unnecessary dependencies.
- Prefer readability over cleverness.
- Keep sensor input, control logic, and actuator output separated.
- Preserve object-oriented structure.
- Follow the existing project architecture.
- Must follow System Operations defined in the analysis phase. Do not add new operations.

---

## C++ Guidelines

- Prefer modern C++ practices where appropriate.
- Avoid unnecessary global state.
- Keep classes focused on a single responsibility.
- Use meaningful naming.
- Avoid overly large functions.
- Prefer explicit behavior over hidden side effects.

---

## Hardware Abstraction Guidance

Detailed hardware interaction is outside the project scope.

Use:

- mocks
- stubs
- simulations
- abstractions

instead of direct hardware control when appropriate.

---

## Expected Outputs

Possible outputs include:

- Source files
- Header files
- State-machine implementation
- Controller logic
- Supporting utility classes

---

## Verification Checklist

Before completing implementation work:

- Does the implementation match requirements?
- Does the implementation follow the design?
- Is code readable?
- Are responsibilities separated properly?
- Is unnecessary complexity avoided?
- Is traceability preserved?

---

## Communication Rules

When reporting results:

- Explain what was implemented.
- Explain why implementation decisions were made.
- Identify incomplete areas clearly.
- Report technical limitations honestly.