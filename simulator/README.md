# RVC Simulator

RVC 컨트롤러(C++) 의 GUI 시뮬레이터 + 시나리오 기반 시스템 테스트 하네스.

## 개요

- **GUI**: Pygame 기반 그리드/로봇 시각화
- **Headless**: CI 환경(`SDL_VIDEODRIVER=dummy`)에서 자동 동작
- **시나리오**: YAML 정의 + assertion 엔진 + pytest 통합

## 백엔드

시뮬레이터는 C++ `RVCController` 본체를 pybind11 바인딩(`rvc`) 으로 직접 구동한다.
`simulator/rvc_sim/rvc.py` 가 binding 모듈을 import 하고 누락 시 명시적 ImportError 로 안내한다.

## 빌드 / 설치

```bash
# 1) C++ 라이브러리 + pybind11 모듈 빌드
cmake --preset dev -DBUILD_PYTHON_BINDINGS=ON
cmake --build build/dev --target rvc -j

# 2) Python 패키지 install (editable)
pip install -e simulator/[dev]
```

## 실행

빌드 산출물(`build/dev/bindings/rvc.<...>.so`) 을 `PYTHONPATH` 에 노출하거나
프로젝트 루트에서 pytest 를 실행하면 `tests/conftest.py` 가 자동으로 경로를 잡는다.

```bash
# Headless
PYTHONPATH=build/dev/bindings rvc-sim run simulator/scenarios/01_basic_cleaning.yaml

# GUI
PYTHONPATH=build/dev/bindings rvc-sim run simulator/scenarios/01_basic_cleaning.yaml --gui

# pytest (모든 시나리오)
PYTHONPATH=build/dev/bindings pytest simulator/tests/
```
