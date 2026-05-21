#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "rvc/SimulatorApi.hpp"

namespace py = pybind11;
using namespace rvc;

namespace {

class PyMotor : public IMotor {
public:
    bool initialize() override { PYBIND11_OVERRIDE_PURE(bool, IMotor, initialize, ); }
    void move(Direction direction) override { PYBIND11_OVERRIDE_PURE(void, IMotor, move, direction); }
};

class PyCleaner : public ICleaner {
public:
    bool initialize() override { PYBIND11_OVERRIDE_PURE(bool, ICleaner, initialize, ); }
    void setPower(PowerLevel level) override {
        PYBIND11_OVERRIDE_PURE(void, ICleaner, setPower, level);
    }
};

class PyObstacleSensor : public IObstacleSensor {
public:
    bool initialize() override { PYBIND11_OVERRIDE_PURE(bool, IObstacleSensor, initialize, ); }
    bool isFrontDetected() override {
        PYBIND11_OVERRIDE_PURE(bool, IObstacleSensor, isFrontDetected, );
    }
    bool isLeftDetected() override {
        PYBIND11_OVERRIDE_PURE(bool, IObstacleSensor, isLeftDetected, );
    }
    bool isRightDetected() override {
        PYBIND11_OVERRIDE_PURE(bool, IObstacleSensor, isRightDetected, );
    }
};

class PyDustSensor : public IDustSensor {
public:
    bool initialize() override { PYBIND11_OVERRIDE_PURE(bool, IDustSensor, initialize, ); }
    void shutdown() override {}
    bool isDustDetected() override {
        PYBIND11_OVERRIDE_PURE(bool, IDustSensor, isDustDetected, );
    }
};

} // namespace

PYBIND11_MODULE(rvc, m) {
    py::enum_<Direction>(m, "Direction")
        .value("FORWARD", Direction::FORWARD)
        .value("BACKWARD", Direction::BACKWARD)
        .value("LEFT", Direction::LEFT)
        .value("RIGHT", Direction::RIGHT)
        .value("STOP", Direction::STOP);

    py::enum_<PowerLevel>(m, "PowerLevel")
        .value("OFF", PowerLevel::OFF)
        .value("NORMAL", PowerLevel::NORMAL)
        .value("POWER_UP", PowerLevel::POWER_UP);

    py::class_<IMotor, PyMotor>(m, "IMotor")
        .def(py::init<>())
        .def("initialize", &IMotor::initialize)
        .def("move", &IMotor::move);

    py::class_<ICleaner, PyCleaner>(m, "ICleaner")
        .def(py::init<>())
        .def("initialize", &ICleaner::initialize)
        .def("setPower", &ICleaner::setPower);

    py::class_<IObstacleSensor, PyObstacleSensor>(m, "IObstacleSensor")
        .def(py::init<>())
        .def("initialize", &IObstacleSensor::initialize)
        .def("isFrontDetected", &IObstacleSensor::isFrontDetected)
        .def("isLeftDetected", &IObstacleSensor::isLeftDetected)
        .def("isRightDetected", &IObstacleSensor::isRightDetected);

    py::class_<IDustSensor, PyDustSensor>(m, "IDustSensor")
        .def(py::init<>())
        .def("initialize", &IDustSensor::initialize)
        .def("isDustDetected", &IDustSensor::isDustDetected);

    py::class_<IAvoidStrategy>(m, "IAvoidStrategy");
    py::class_<DefaultAvoidStrategy, IAvoidStrategy>(m, "DefaultAvoidStrategy").def(py::init<>());

    py::class_<MovementManager>(m, "MovementManager")
        .def(py::init<IMotor&, IAvoidStrategy&>(), py::arg("motor"), py::arg("strategy"),
             py::keep_alive<1, 3>());

    py::class_<CleaningManager>(m, "CleaningManager")
        .def(py::init<ICleaner&, Timer::ClockFn>(), py::arg("cleaner"), py::arg("clock"),
             py::keep_alive<1, 2>());

    py::class_<ObstacleSensorSubject>(m, "ObstacleSensorSubject")
        .def(py::init<IObstacleSensor&>(), py::arg("sensor"), py::keep_alive<1, 2>())
        .def("onInterrupt", &ObstacleSensorSubject::onInterrupt)
        .def("poll", &ObstacleSensorSubject::poll);

    py::class_<DustSensorSubject>(m, "DustSensorSubject")
        .def(py::init<IDustSensor&>(), py::arg("sensor"), py::keep_alive<1, 2>())
        .def("poll", &DustSensorSubject::poll);

    py::class_<RVCController>(m, "RVCController")
        .def(py::init<IObstacleSensor&, IDustSensor&, IMotor&, ICleaner&, MovementManager&,
                      CleaningManager&, ObstacleSensorSubject&, DustSensorSubject&>(),
             py::arg("obstacle_sensor"), py::arg("dust_sensor"), py::arg("motor"),
             py::arg("cleaner"), py::arg("movement_manager"), py::arg("cleaning_manager"),
             py::arg("obstacle_subject"), py::arg("dust_subject"), py::keep_alive<1, 2>(),
             py::keep_alive<1, 3>(), py::keep_alive<1, 4>(), py::keep_alive<1, 5>(),
             py::keep_alive<1, 6>(), py::keep_alive<1, 7>(), py::keep_alive<1, 8>(),
             py::keep_alive<1, 9>())
        .def("powerOn", &RVCController::powerOn)
        .def("powerOff", &RVCController::powerOff)
        .def("tick", &RVCController::tick);

    m.def("current_state_name", &current_state_name, py::arg("controller"));
}
