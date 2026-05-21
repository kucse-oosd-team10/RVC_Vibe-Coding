"""rvc module shim — exposes the C++ controller API via pybind11."""
try:
    from rvc import (  # type: ignore
        CleaningManager,
        DefaultAvoidStrategy,
        Direction,
        DustSensorSubject,
        IAvoidStrategy,
        ICleaner,
        IDustSensor,
        IMotor,
        IObstacleSensor,
        MovementManager,
        ObstacleSensorSubject,
        PowerLevel,
        RVCController,
        current_state_name,
    )
except ImportError as exc:  # pragma: no cover - import-time guard
    raise ImportError(
        "rvc binding not available. Build with -DBUILD_PYTHON_BINDINGS=ON "
        "and ensure the resulting rvc.<...>.so directory is on PYTHONPATH "
        "(e.g. PYTHONPATH=build/dev/bindings)."
    ) from exc


__all__ = [
    "CleaningManager",
    "DefaultAvoidStrategy",
    "Direction",
    "DustSensorSubject",
    "IAvoidStrategy",
    "ICleaner",
    "IDustSensor",
    "IMotor",
    "IObstacleSensor",
    "MovementManager",
    "ObstacleSensorSubject",
    "PowerLevel",
    "RVCController",
    "current_state_name",
]
