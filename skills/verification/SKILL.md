# Verification Skill

## Purpose

Use this skill when validating software quality, behavior, correctness, and requirement coverage.

Verification includes:

- Static Analysis
- Dynamic Analysis
- Unit Testing
- System Testing

---

## Static Analysis Responsibilities

Use tools such as:

- clang-format
- clang-tidy
- cppcheck

Review:

- coding style consistency
- maintainability
- complexity
- dependency quality
- potential defects

---

## Dynamic Analysis Responsibilities

Analyze runtime behavior under various scenarios.

Examples include:

- obstacle detection behavior
- state transitions
- backward movement logic
- dust response behavior
- runtime logging
- simulation execution

Focus on behavioral correctness.

---

## Unit Testing Rules

- Unit tests should validate public behavior.
- Each meaningful public method should have tests whenever practical.
- Include normal cases and boundary cases.
- Include error situations where applicable.
- Avoid overly implementation-dependent tests.

---

## System Testing Rules

- System tests should validate requirement-level behavior.
- Every implemented requirement should be covered whenever possible.
- Requirement IDs should be traceable to tests.

Example:

- R2 -> obstacle avoidance scenario test

---

## Recommended Verification Outputs

Possible outputs include:

- Unit test files
- System test files
- Static analysis reports
- Simulation logs
- Verification summaries
- Requirement coverage reports

---

## Verification Checklist

Before completing verification:

- Did static analysis complete successfully?
- Were verification failures reported honestly?
- Are unit tests meaningful?
- Are system scenarios covered?
- Are requirement IDs traceable to tests?
- Were unexpected behaviors documented?

---

## Communication Rules

When reporting results:

- Clearly distinguish passed and failed checks.
- Report warnings and limitations honestly.
- Do not hide failing tests or analysis results.
- Explain likely causes of failures when possible.

---

## Coverage Verification

Use coverage and quality-analysis tools whenever available.

Examples include:

- SonarQube
- llvm-cov
- gcov
- lcov

Coverage reports should help identify:

- untested behavior
- partially tested logic
- missing requirement verification

Coverage metrics should not replace meaningful test design.