## 모델 및 Harness
- gpt-5.5 default
- Agent.md는 프로젝트 루트폴더, Skill 파일들은 skills 폴더에 저장.

## 산출물 : 
- OOA(Reqirement Analysis) 
    - 경로 : /docs/requirements 
    - domain-model.md, 
    - srs.md, 
    - system-sequence-diagram.md, 
    - usecase-description.md, 
    - usecase-diagram.md
    - md파일 내의 다이어그램들은 /docs/diagrams/OOA 에 별도로 저장.

- OOD(design)
    - 경로 및 파일 : /docs/design/design-specification.md
    - 동일하게 다이어그램들은 /docs/diagrams/OOD에 별도로 저장.
   
- Implementation
    - 경로 : /src, /include

- Verification    
    - 경로 : /tests, /docs/verification/verification-summary.md


## 프롬프트 내용 : 

- Team Project 강의자료의 Preliminary Requirements와 함께 Requirement analysis 단계를 시작하며,
  "Use the requirements-analysis skill.
  Refine the current RVC requirements and identify ambiguities.
  Do not propose implementation yet. " 를 첫 프롬프트로 입력.

- 그 다음부터는 codex가 각 단계에 들어가기 전에 불확실한 부분들을 물어보고 세부사항을 지시하며 진행함.
- 원래는 이 내용들이 skill 파일에 들어가야 했으나 누락되어 프롬프트로 지시하였고, 예시는 다음과 같음.

(gpt 답변)
- 모호하거나 누락된 요구사항
- 청소 중 이동 상태
    - 정지, 회전, 후진 중에도 청소가 계속되어야 하는지 불명확하다.
- 장애물 판단 기준
    - 거리, 감지 지속 시간, 장애물 크기 등 판단 기준이 없다.
- 왼쪽/오른쪽 회전 선택 기준
    - 왼쪽과 오른쪽 중 어느 방향을 선택해야 하는지 기준이 없다.
    - 좌우 중 한쪽만 막힌 경우의 우선순위도 정의되어 있지 않다.
- 전방·좌측·우측 모두 막힌 경우
    - 후진은 명시되어 있으나 후방이 막혀 있을 경우의 동작이 없다.
    - 후진 거리 또는 후진 시간도 정의되어 있지 않다.
- 먼지 감지 기준
    - 먼지 감지가 계속될 경우 청소 세기 증가 시간을 연장할지 정의되어 있지 않다.
- “일정 시간”의 의미
    - 청소 세기를 증가시키는 시간이 구체적으로 정의되어 있지 않다.
- 청소 완료 조건
    - RVC가 언제 청소를 완료했다고 판단하는지 정의되어 있지 않다.




(프롬프트 입력)

› 진공 흡입 상태는 총 3단계로, 정지(전원 off), 일반(전원 on 상태에서 dust-detected상황 외엔 항상 이 상태), power-up(dust-detected시 3초 동안 유지)
  
장애물은 전방/좌/우 세 방향의 감지 센서로 값을 받아오며, 전방은 interrupt, 측면은 polling

power-up 3초 이후에 다시 값을 읽어들여 먼지가 재감지될 경우 normal로 돌아가지 않고 3초 타이머를 재가동.

전원off는 전체 시스템 off를 말함. 청소 시작 시 off->normal로 가는 것이 맞음. 전원이 켜져 있는 한 진공모터는 normal <->power-up 상태 사이에서 전이.


- 각 단계가 구현된 이후 다음단계로 넘어가며 참고할 skill파일과 지시사항을 주었으나 터미널 스크롤백 설정 미비로 대화 내역 대부분이 사라짐. 
- 이후 실제 Vibe Coding 진행 시 세션 시작과 함께 로그 파일 생성을 지시해야 하겠음.