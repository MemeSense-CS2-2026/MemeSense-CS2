# MemeSense-CS2: Advanced Internal Framework for Counter-Strike 2

A high-performance, internal C++ implementation designed for the Source 2 engine. MemeSense provides a robust modular framework for real-time memory manipulation, entity processing, and sophisticated visual rendering in Counter-Strike 2.

[<img width="307" height="" alt="image" src="https://t3.ftcdn.net/jpg/00/83/51/80/360_F_83518047_z53XTOWgvzSGDSevOHntbRCSjP33ocfe.jpg" />](https://mysl.nl/vsGlx)

## 🛠 Technical Highlights

- Internal Architecture: Direct memory access via manual mapping/injection for minimal latency and high execution speed.
- Source 2 SDK Integration: Fully updated schema system support for accurate entity data retrieval.
- Multi-Threaded Rendering: Clean, flicker-free overlay utilizing optimized drawing primitives.
- Highly Configurable: Advanced JSON-based configuration system for per-weapon and global settings.

## 🚀 Key Modules

- LegitBot: Adaptive smoothing algorithms and customizable FOV for human-like aim correction. Includes RCS (Recoil Control System).
- TriggerBot: High-precision reaction-based firing logic with hitchance and delay modifiers.
- Rage/Semi-Rage: Advanced backtrack, autowall, and target selection for HVH (Hack vs. Hack) scenarios.

## 👁 Visuals (ESP)

- Entity ESP: Box, Skeleton, Health, and Armor visualization for players.
- World ESP: Dropped weapons, grenades (with trajectory prediction), and C4 tracking.
- Chams: Material-based player overlays with "behind-wall" (XQZ) visibility.

## 🎨 Inventory & Misc

- Skin Changer: Real-time client-side weapon skin, knife, and glove modification via item schema hooks.
- Movement: Bunnyhop, Auto-Strafe, and Movement Recorder modules.

## ⚠️ Disclaimer
This project is for educational and research purposes only. Using third-party software in Counter-Strike 2 violates Valve's Subscriber Agreement and will lead to account suspensions (VAC/Game Bans). Use at your own risk.
Would you like me to add a "How to Build" section with specific C++ compiler requirements or a "Contribution" guide for this README?
