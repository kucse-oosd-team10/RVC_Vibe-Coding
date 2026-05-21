"""World grid: bounds, obstacles, dust, collisions."""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Set, Tuple

from .types import Pose

Cell = Tuple[int, int]


@dataclass
class World:
    width: int
    height: int
    obstacles: Set[Cell] = field(default_factory=set)
    dust: Dict[Cell, float] = field(default_factory=dict)
    dust_threshold: float = 0.5
    collisions: List[Cell] = field(default_factory=list)

    def is_in_bounds(self, pos: Cell) -> bool:
        x, y = pos
        return 0 <= x < self.width and 0 <= y < self.height

    def is_blocked(self, pos: Cell) -> bool:
        return not self.is_in_bounds(pos) or pos in self.obstacles

    def detect_obstacles(self, pose: Pose) -> Tuple[bool, bool, bool]:
        return (
            self.is_blocked(pose.front_pos()),
            self.is_blocked(pose.left_pos()),
            self.is_blocked(pose.right_pos()),
        )

    def detect_dust(self, pose: Pose) -> bool:
        return self.dust.get((pose.x, pose.y), 0.0) >= self.dust_threshold

    def collect_dust(self, pose: Pose, amount: float) -> float:
        if amount <= 0:
            return 0.0
        cell = (pose.x, pose.y)
        current = self.dust.get(cell, 0.0)
        removed = min(current, amount)
        new_amount = current - removed
        if new_amount > 0:
            self.dust[cell] = new_amount
        elif cell in self.dust:
            del self.dust[cell]
        return removed

    def total_dust(self) -> float:
        return sum(self.dust.values())

    def record_collision(self, pos: Cell) -> None:
        self.collisions.append(pos)

    def replenish_dust(self, amount: float = 1.0) -> None:
        """Top up dust on every initially-dusty cell back to `amount`.

        Used by long-run NFR scenarios to ensure the controller keeps finding
        work to do without needing to seed thousands of dust cells upfront.
        Cells with current dust above `amount` are left untouched.
        """
        if not hasattr(self, "_dust_seed_cells"):
            return
        for cell in self._dust_seed_cells:
            if self.dust.get(cell, 0.0) < amount:
                self.dust[cell] = amount

    def seed_replenish(self) -> None:
        self._dust_seed_cells = set(self.dust.keys())
