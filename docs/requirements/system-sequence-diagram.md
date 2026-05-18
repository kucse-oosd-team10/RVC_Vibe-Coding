# RVC System Sequence Diagrams

## 현재 단계

현재 단계는 Requirements Analysis 이후의 분석 모델 정리이다. 본 문서는 외부 actor와 `rvc-controller` 사이의 시스템 수준 이벤트 순서를 표현한다.

## SSD-001 자동 진공 청소 시작

```mermaid
sequenceDiagram
    actor User as 사용자
    participant Front as 전방 센서 하드웨어
    participant Side as 측면 센서 하드웨어
    participant Dust as 먼지 센서 하드웨어
    participant RVC as rvc-controller
    participant Move as Movement Motor
    participant Clean as Cleaning Motor

    User->>RVC: 청소 시작 버튼 입력
    RVC-->>RVC: OFF에서 전원 ON 상태로 전환
    RVC->>Front: 센서 인터페이스 초기화
    RVC->>Side: 센서 인터페이스 초기화
    RVC->>Dust: 센서 인터페이스 초기화
    RVC->>Clean: NORMAL 흡입 명령
    RVC->>Move: 전진 명령
```

## SSD-002 자동 진공 청소 종료

```mermaid
sequenceDiagram
    actor User as 사용자
    participant Front as 전방 센서 하드웨어
    participant Side as 측면 센서 하드웨어
    participant Dust as 먼지 센서 하드웨어
    participant RVC as rvc-controller
    participant Move as Movement Motor
    participant Clean as Cleaning Motor

    User->>RVC: 청소 종료 버튼 입력
    RVC-->>RVC: 청소 동작 종료
    RVC->>Move: 정지 명령
    RVC->>Clean: OFF 흡입 명령
    RVC->>Front: 센서 인터페이스 종료
    RVC->>Side: 센서 인터페이스 종료
    RVC->>Dust: 센서 인터페이스 종료
    RVC-->>RVC: OFF 상태로 전환
```

## SSD-003 전방 장애물 회피

```mermaid
sequenceDiagram
    participant Front as 전방 센서 하드웨어
    participant Side as 측면 센서 하드웨어
    participant RVC as rvc-controller
    participant Move as Movement Motor
    participant Clean as Cleaning Motor

    Front->>RVC: 전방 장애물 interrupt
    RVC->>Move: 정지 명령
    RVC->>Clean: NORMAL 흡입 명령
    RVC->>Side: 좌우 장애물 값 요청
    Side-->>RVC: 좌우 장애물 값 반환
    alt 전방 장애물만 감지
        RVC->>Move: 좌회전 명령
    else 전방 + 좌측 장애물 감지
        RVC->>Move: 우회전 명령
    else 전방 + 우측 장애물 감지
        RVC->>Move: 좌회전 명령
    end
    RVC-->>RVC: 10초 후 회전 완료 간주
    RVC->>Move: 전진 명령
```

## SSD-004 삼방향 장애물 탈출

```mermaid
sequenceDiagram
    participant Front as 전방 센서 하드웨어
    participant Side as 측면 센서 하드웨어
    participant RVC as rvc-controller
    participant Move as Movement Motor
    participant Clean as Cleaning Motor

    Front->>RVC: 전방 장애물 interrupt
    RVC->>Side: 좌우 장애물 값 요청
    Side-->>RVC: 좌측 및 우측 장애물 감지
    RVC->>Move: 정지 명령
    RVC->>Clean: NORMAL 흡입 명령
    RVC->>Move: 후진 명령
    loop 후진 중
        RVC->>Side: 좌우 장애물 값 요청
        Side-->>RVC: 좌우 장애물 값 반환
    end
    alt 좌측 장애물 해제
        RVC->>Move: 좌회전 명령
    else 우측 장애물 해제
        RVC->>Move: 우회전 명령
    else 좌우 장애물 동시 해제
        RVC->>Move: 좌회전 명령
    end
    RVC-->>RVC: 10초 후 회전 완료 간주
    RVC->>Move: 전진 명령
```

## SSD-005 먼지 감지에 따른 흡입 강화

```mermaid
sequenceDiagram
    participant Dust as 먼지 센서 하드웨어
    participant RVC as rvc-controller
    participant Clean as Cleaning Motor

    Dust->>RVC: 먼지 감지
    alt RVC 전진 중
        RVC->>Clean: POWER_UP 흡입 명령
        RVC-->>RVC: 3초 유지
        RVC->>Dust: 먼지 값 재확인
        alt 먼지 재감지
            RVC->>Clean: POWER_UP 흡입 명령 유지
            RVC-->>RVC: 3초 타이머 재시작
        else 먼지 미감지
            RVC->>Clean: NORMAL 흡입 명령
        end
    else RVC 정지/회전/후진 중
        RVC->>Clean: NORMAL 흡입 명령 유지
        RVC-->>RVC: 다음 전진 시작 시 POWER_UP 진입 조건 저장
    end
```
