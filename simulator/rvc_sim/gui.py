"""Pygame GUI with smooth motion interpolation.

The GUI is opt-in: pass an instance to ``SimulationRunner(gui=PygameRenderer())``.
``render(runner)`` is invoked once per tick and is responsible for advancing
real wall-clock time by ``tick_duration_ms`` while interpolating the robot
between its previous pose and its current pose. Returns ``False`` if the user
closed the window — the runner will then stop early.
"""
from __future__ import annotations

import math
from typing import TYPE_CHECKING, Optional, Tuple

import pygame

from .types import Heading

if TYPE_CHECKING:
    from .runner import SimulationRunner


CELL_PX = 32
HUD_HEIGHT_PX = 60
GRID_LINE = (60, 60, 60)
BG = (24, 24, 28)
HUD_BG = (16, 16, 20)
HUD_FG = (220, 220, 220)
OBSTACLE = (90, 90, 100)
ROBOT_BODY = (90, 200, 255)
ROBOT_HEAD = (255, 220, 80)


def _heading_angle_deg(h: Heading) -> float:
    return {Heading.N: 270.0, Heading.E: 0.0, Heading.S: 90.0, Heading.W: 180.0}[h]


def _shortest_angle(a: float, b: float) -> float:
    d = (b - a + 540.0) % 360.0 - 180.0
    return d


class PygameRenderer:
    def __init__(self, *, fps: int = 60, cell_px: int = CELL_PX) -> None:
        self.fps = fps
        self.cell_px = cell_px
        self._prev_pose: Optional[Tuple[float, float, float]] = None
        self._initialized = False
        self._screen: Optional[pygame.Surface] = None
        self._font: Optional[pygame.font.Font] = None
        self._clock: Optional[pygame.time.Clock] = None

    def _ensure_init(self, runner: "SimulationRunner") -> None:
        if self._initialized:
            return
        pygame.init()
        w = runner.world.width * self.cell_px
        h = runner.world.height * self.cell_px + HUD_HEIGHT_PX
        self._screen = pygame.display.set_mode((w, h))
        pygame.display.set_caption(f"rvc-sim — {runner.scenario.name}")
        self._font = pygame.font.Font(None, 16)
        self._clock = pygame.time.Clock()
        self._initialized = True

    def _draw_world(self, runner: "SimulationRunner") -> None:
        assert self._screen is not None
        self._screen.fill(BG)
        world = runner.world
        for x in range(world.width):
            for y in range(world.height):
                rect = pygame.Rect(
                    x * self.cell_px, y * self.cell_px, self.cell_px, self.cell_px
                )
                pygame.draw.rect(self._screen, GRID_LINE, rect, 1)
        for ox, oy in world.obstacles:
            rect = pygame.Rect(
                ox * self.cell_px, oy * self.cell_px, self.cell_px, self.cell_px
            )
            pygame.draw.rect(self._screen, OBSTACLE, rect)
        for (dx, dy), amount in world.dust.items():
            if amount <= 0:
                continue
            shade = max(40, min(220, int(60 + amount * 40)))
            color = (shade, shade // 2, 30)
            cx = dx * self.cell_px + self.cell_px // 2
            cy = dy * self.cell_px + self.cell_px // 2
            radius = max(2, min(self.cell_px // 3, int(2 + amount * 2)))
            pygame.draw.circle(self._screen, color, (cx, cy), radius)

    def _draw_robot(self, x: float, y: float, angle_deg: float) -> None:
        assert self._screen is not None
        cx = x * self.cell_px + self.cell_px / 2.0
        cy = y * self.cell_px + self.cell_px / 2.0
        r = self.cell_px * 0.4
        pygame.draw.circle(self._screen, ROBOT_BODY, (int(cx), int(cy)), int(r))
        rad = math.radians(angle_deg)
        hx = cx + math.cos(rad) * r
        hy = cy + math.sin(rad) * r
        pygame.draw.line(self._screen, ROBOT_HEAD, (cx, cy), (hx, hy), 3)
        pygame.draw.circle(self._screen, ROBOT_HEAD, (int(hx), int(hy)), 4)

    def _draw_hud(self, runner: "SimulationRunner") -> None:
        assert self._screen is not None and self._font is not None
        world = runner.world
        hud_y = world.height * self.cell_px
        hud_rect = pygame.Rect(0, hud_y, world.width * self.cell_px, HUD_HEIGHT_PX)
        pygame.draw.rect(self._screen, HUD_BG, hud_rect)
        snap = runner.snapshots[-1]
        lines = [
            f"tick {snap.tick:>4d}   state {snap.state_name:<12}   "
            f"clock {snap.clock_ms:>5d}ms",
            f"robot ({snap.robot_x},{snap.robot_y}) {snap.robot_heading}   "
            f"cleaner {snap.cleaner_power.name:<8}   "
            f"dust {snap.dust_total:6.2f}   collisions {snap.collision_count}",
        ]
        for i, text in enumerate(lines):
            surf = self._font.render(text, True, HUD_FG)
            self._screen.blit(surf, (8, hud_y + 8 + i * 20))

    def render(self, runner: "SimulationRunner") -> bool:
        self._ensure_init(runner)
        assert self._clock is not None

        cur_pose = (
            float(runner.robot.x),
            float(runner.robot.y),
            _heading_angle_deg(runner.robot.heading),
        )

        if self._prev_pose is None:
            self._prev_pose = cur_pose

        waypoints: list[Tuple[float, float, float]] = [self._prev_pose]
        for p in runner.motor.tick_pose_log:
            waypoints.append(
                (float(p.x), float(p.y), _heading_angle_deg(p.heading))
            )
        if len(waypoints) == 1:
            waypoints.append(cur_pose)

        n_segments = len(waypoints) - 1
        seg_ms = max(1, runner.tick_duration_ms // n_segments)

        for i in range(n_segments):
            a = waypoints[i]
            b = waypoints[i + 1]
            delta_angle = _shortest_angle(a[2], b[2])
            elapsed = 0
            while elapsed < seg_ms:
                for ev in pygame.event.get():
                    if ev.type == pygame.QUIT:
                        return False
                    if ev.type == pygame.KEYDOWN and ev.key == pygame.K_ESCAPE:
                        return False
                t = min(1.0, elapsed / max(1, seg_ms))
                ix = a[0] + (b[0] - a[0]) * t
                iy = a[1] + (b[1] - a[1]) * t
                iang = a[2] + delta_angle * t
                self._draw_world(runner)
                self._draw_robot(ix, iy, iang)
                self._draw_hud(runner)
                pygame.display.flip()
                elapsed += self._clock.tick(self.fps)

        self._prev_pose = cur_pose
        return True
