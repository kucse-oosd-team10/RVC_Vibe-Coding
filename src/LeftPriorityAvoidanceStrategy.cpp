#include "rvc/IObstacleAvoidanceStrategy.hpp"

namespace rvc {

AvoidanceAction LeftPriorityAvoidanceStrategy::decideOnFrontObstacle(
    const SideObstacleSnapshot &sideSnapshot) const {
    if (sideSnapshot.leftDetected && sideSnapshot.rightDetected) {
        return AvoidanceAction::MoveBackward;
    }
    if (sideSnapshot.leftDetected) {
        return AvoidanceAction::TurnRight;
    }
    return AvoidanceAction::TurnLeft;
}

AvoidanceAction LeftPriorityAvoidanceStrategy::decideWhileBackward(
    const SideObstacleSnapshot &previousSideSnapshot,
    const SideObstacleSnapshot &currentSideSnapshot) const {
    const bool leftCleared = previousSideSnapshot.leftDetected && !currentSideSnapshot.leftDetected;
    const bool rightCleared =
        previousSideSnapshot.rightDetected && !currentSideSnapshot.rightDetected;

    if (leftCleared) {
        return AvoidanceAction::TurnLeft;
    }
    if (rightCleared) {
        return AvoidanceAction::TurnRight;
    }
    return AvoidanceAction::KeepBackward;
}

} // namespace rvc
