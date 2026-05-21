"""Robot: discrete grid-based pose with heading rotation."""
from __future__ import annotations

from .rvc import Direction
from .types import Heading, Pose
from .world import World


class Robot:
    def __init__(self, x: int, y: int, heading: Heading) -> None:
        self.x = x
        self.y = y
        self.heading = heading

    @property
    def pose(self) -> Pose:
        return Pose(self.x, self.y, self.heading)

    def move(self, direction: Direction, world: World) -> None:
        if direction == Direction.STOP:
            return
        if direction == Direction.LEFT:
            self.heading = self.heading.rotate_left()
            return
        if direction == Direction.RIGHT:
            self.heading = self.heading.rotate_right()
            return

        if direction == Direction.FORWARD:
            target = self.pose.front_pos()
        elif direction == Direction.BACKWARD:
            target = self.pose.back_pos()
        else:
            return

        if world.is_blocked(target):
            world.record_collision(target)
            return
        self.x, self.y = target
