# RVC Verification Summary

## 1. 현재 단계

현재 단계는 Verification이다. 검증 기준은 다음과 같이 정정되었다.

- Unit Test: GoogleTest 기반으로 수행한다.
- System Test: 직접 개발되어 GitHub에 존재하는 simulator만 사용한다.
- Mock/fake 기반 controller scenario test는 system test로 간주하지 않는다.

## 2. 현재 검증 상태

| 항목 | 상태 | 비고 |
|---|---|---|
| GoogleTest unit test 코드 | 작성됨 | `tests/rvc_unit_tests.cpp` |
| GoogleTest CMake 연동 | 작성됨 | `find_package(GTest REQUIRED)` 사용 |
| GoogleTest 로컬 의존성 | 미충족 | 현재 환경에서 `GTest`를 찾지 못해 configure 실패 |
| Simulator 기반 system test | 미구현 | simulator GitHub 위치와 실행 방식 필요 |
| Mock/fake 기반 system test | 제거됨 | system test로 오해되지 않도록 제거 |

## 3. GoogleTest Unit Test

Unit test 파일:

- `tests/rvc_unit_tests.cpp`

포함된 테스트:

| 테스트 그룹 | 검증 내용 | 관련 요구사항 |
|---|---|---|
| `LeftPriorityAvoidanceStrategyTest` | 좌회전 우선 회피 전략, 후진 필요 여부, 후진 중 해제 방향 판단 | RVC-FR-008 ~ RVC-FR-014 |
| `CleaningManagerTest` | POWER_UP 3초 유지, 먼지 재감지 시 타이머 재시작, 먼지 미감지 시 NORMAL 복귀 | RVC-FR-023 ~ RVC-FR-029 |
| `CleaningManagerTest` | 정지/회전/후진 중 먼지 감지 시 POWER_UP 지연 | RVC-FR-017 ~ RVC-FR-019 |
| `MovementManagerTest` | 회전 명령 후 timer 기반 회전 완료 판단 | RVC-FR-015 |
| `RvcControllerTest` | 청소 시작/종료 시 센서 초기화/종료와 motor 명령 | RVC-FR-001, RVC-FR-002, RVC-FR-020 ~ RVC-FR-022, RVC-FR-030 ~ RVC-FR-033 |
| `RvcControllerTest` | 전방 장애물만 감지된 경우 좌회전 후 전진 복귀 | RVC-FR-008, RVC-FR-016 |
| `RvcControllerTest` | mock 기반 장애물 탈출 흐름 | unit-level integration check only |

## 4. System Test

System test는 현재 구현하지 않았다. 기준은 기존 GitHub simulator만 사용하는 것이다.

`commands/run-system-tests.sh`는 simulator 명령이 명시되지 않으면 실패하도록 변경했다.

필요한 설정:

```bash
RVC_SIMULATOR_TEST_COMMAND="<simulator 실행 명령>" ./commands/run-system-tests.sh
```

또는 simulator repository와 실행 명령이 확정되면 `commands/run-system-tests.sh`에 고정 명령으로 반영할 수 있다.

## 5. 현재 실행 결과

현재 환경에서 다음 명령은 실패한다.

```bash
./commands/configure.sh
```

실패 원인:

```text
Could NOT find GTest
```

즉, GoogleTest 설치 또는 CMake에서 찾을 수 있는 경로 설정이 필요하다.

## 6. 남은 작업

1. GoogleTest 설치 또는 CMake 경로 설정
2. `./commands/configure.sh`
3. `./commands/build.sh`
4. `./commands/run-unit-tests.sh`
5. simulator GitHub repository 확인
6. simulator 기반 system test command 확정
7. `./commands/run-system-tests.sh`
8. `./commands/verify-all.sh`

