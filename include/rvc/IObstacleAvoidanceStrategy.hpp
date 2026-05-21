#pragma once

#include "rvc/Types.hpp"

namespace rvc {

class IObstacleAvoidanceStrategy {
public:
    virtual ~IObstacleAvoidanceStrategy() = default;

    virtual AvoidanceAction
    decideOnFrontObstacle(const SideObstacleSnapshot& sideSnapshot) const = 0;
    virtual AvoidanceAction
    decideWhileBackward(const SideObstacleSnapshot& previousSideSnapshot,
                        const SideObstacleSnapshot& currentSideSnapshot) const = 0;
};

class LeftPriorityAvoidanceStrategy final : public IObstacleAvoidanceStrategy {
public:
    AvoidanceAction decideOnFrontObstacle(const SideObstacleSnapshot& sideSnapshot) const override;
    AvoidanceAction
    decideWhileBackward(const SideObstacleSnapshot& previousSideSnapshot,
                        const SideObstacleSnapshot& currentSideSnapshot) const override;
};

} // namespace rvc
