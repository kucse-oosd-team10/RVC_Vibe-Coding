"""Shared geometry types: Heading + Pose.

Pygame convention: ``+y`` is downward, ``N`` heading points up (``-y``).
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Tuple


class Heading(Enum):
    N = (0, -1)
    E = (1, 0)
    S = (0, 1)
    W = (-1, 0)

    def __init__(self, dx: int, dy: int) -> None:
        self.dx = dx
        self.dy = dy

    def rotate_left(self) -> "Heading":
        order = (Heading.N, Heading.W, Heading.S, Heading.E)
        return order[(order.index(self) + 1) % 4]

    def rotate_right(self) -> "Heading":
        order = (Heading.N, Heading.E, Heading.S, Heading.W)
        return order[(order.index(self) + 1) % 4]

    @classmethod
    def from_label(cls, label: str) -> "Heading":
        return cls[label.upper()]


@dataclass(frozen=True)
class Pose:
    x: int
    y: int
    heading: Heading

    def front_pos(self) -> Tuple[int, int]:
        return (self.x + self.heading.dx, self.y + self.heading.dy)

    def back_pos(self) -> Tuple[int, int]:
        return (self.x - self.heading.dx, self.y - self.heading.dy)

    def left_pos(self) -> Tuple[int, int]:
        h = self.heading.rotate_left()
        return (self.x + h.dx, self.y + h.dy)

    def right_pos(self) -> Tuple[int, int]:
        h = self.heading.rotate_right()
        return (self.x + h.dx, self.y + h.dy)
