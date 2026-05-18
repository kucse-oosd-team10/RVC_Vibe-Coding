# AGENTS.md

## Project Overview

This project implements an RVC Controller (Robot Vacuum Cleaner Controller) using vibe-coding while satisfying the given software requirements.

The project emphasizes not only implementation, but also the active application of software engineering processes and object-oriented development methodology.

The controller focuses only on automatic cleaning behavior. Detailed hardware control implementation is outside the scope of this project unless explicitly required.

---

## Development Process

The project must proceed through the following stages:

1. Requirements Analysis
2. Design
3. Implementation
4. Verification

Each stage has a corresponding skill file. Refer to the appropriate skill file before performing work related to that stage.

Do not skip stages.

The user determines whether the current stage is complete. Do not proceed to the next stage unless the user explicitly approves it.

---

## Stage Descriptions

### 1. Requirements Analysis

Focus on understanding, refining, organizing, and documenting requirements.

Activities may include:

- Functional requirement analysis
- Non-functional requirement analysis
- Constraint identification
- Assumption clarification
- Scope definition
- Requirement traceability preparation

Every functional requirement should have a unique requirement ID whenever practical.

Implementation should not begin during this stage unless explicitly requested by the user.

---

### 2. Design

Focus on object-oriented software design before implementation.

Activities may include:

- Class responsibility definition
- Interface design
- State-machine modeling
- Dependency analysis
- Architecture definition
- Sequence or interaction modeling
- Design refinement

Behavior control logic should be modeled explicitly as a state machine whenever appropriate.

Implementation should not begin before sufficient design artifacts are produced.

---

### 3. Implementation

Implement only approved requirements and approved designs.

Implementation guidelines:

- Prefer simple, maintainable, readable C++ code.
- Keep responsibilities clearly separated.
- Separate sensor input, decision logic, and actuator commands.
- Avoid unnecessary dependencies.
- Avoid feature creep.
- Do not implement out-of-scope functionality.
- Preserve traceability between requirements, design, implementation, and tests.

Use mocks, stubs, or simulations instead of real hardware interaction when necessary.

---

### 4. Verification

Verification includes the following activities:

- Static Analysis
- Dynamic Analysis
- Unit Testing
- System Testing

#### Static Analysis

Use tools and techniques such as:

- clang-format
- clang-tidy
- cppcheck

Review:

- Naming
- Maintainability
- Complexity
- Dependency structure
- Potential bugs
- Coding style consistency

#### Dynamic Analysis

Analyze runtime behavior and controller logic under various situations.

Examples include:

- State transition validation
- Obstacle handling behavior
- Runtime logging
- Simulation-based behavior checks
- Scenario validation

#### Unit Testing

Unit tests must verify the public behavior of each implemented class.

For each meaningful public method, corresponding tests should exist whenever practical.

Boundary conditions and error situations should also be tested where appropriate.

#### System Testing

System tests must validate requirement-level scenarios.

Every implemented requirement should be covered by at least one test whenever possible.

---

## Development Environment

Use the following development environment and tools:

- Git
- CMake
- Ninja
- Clang/LLVM
- clang-format
- clang-tidy
- cppcheck
- VSCode

---

## General Rules

- Follow software engineering and object-oriented design principles.
- Keep requirements, design, implementation, and tests traceable.
- Understand the current project structure before modifying code.
- Explain changes clearly after modifications.
- Explain work according to the current development stage.
- Clearly state assumptions and limitations.
- Do not hide failures or errors.
- If verification tools or tests fail, report the failure clearly.

---

## Skill Usage

Each development stage has a corresponding skill file located under the skills directory.

Examples:

- skills/requirements-analysis/SKILL.md
- skills/design/SKILL.md
- skills/implementation/SKILL.md
- skills/verification/SKILL.md

When performing work for a stage, follow the instructions defined in the corresponding skill file.

---

## Verification Commands

Use the following commands when appropriate:

- ./commands/configure.sh
- ./commands/build.sh
- ./commands/verify-all.sh
- ./commands/run-tests.sh

---

## Expected Workflow

Typical workflow:

1. Analyze requirements
2. Produce design artifacts
3. Implement approved design
4. Perform verification activities
5. Report results
6. Request user approval before proceeding

---

## Communication Rules

When responding:

- State the current development stage.
- Explain reasoning clearly.
- Distinguish assumptions from confirmed facts.
- Describe what changed and why.
- Describe what was not changed and why.
- Request user approval before advancing stages.

---

## Simulator Reuse Rule

This project reuses the existing simulator and verification infrastructure from:

https://github.com/kucse-oosd-team10/rvc-controller

The simulator, scenario files, Python integration layer, and test harness should be treated as external verification infrastructure.

Do not rewrite or redesign the simulator unless explicitly requested.

The controller implementation must adapt to the simulator interface instead of modifying the simulator to fit the implementation.

---

## Code Quality and Coverage

Use SonarQube and coverage tooling as part of verification activities whenever available.

Verification should include:

- static analysis
- test execution
- code coverage analysis
- maintainability inspection

Unit test coverage should be measurable and reportable.

Low coverage areas should be identified and documented.

---

## Coverage and SonarQube Commands

When verification includes coverage analysis, use:

- ./commands/coverage.sh

When SonarQube analysis is available, use:

- ./commands/sonarqube.sh

Coverage reports should be generated before running SonarQube analysis.