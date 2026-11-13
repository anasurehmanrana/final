Team Contributions

**Member 1: Player Mechanics, Scoring & Persistence**
* Implemented the discrete grid-based movement system (`playerCol`, `playerRow`) with strict screen boundary checks.
* Developed the dynamic progression and scoring system (`updateScore`) that rewards forward movement through road lanes.
* Built persistent high-score file handling using C++ `<fstream>` to read and write records (`highscore.txt`).
* Handled the rendering logic for the player character and real-time Heads-Up Display (HUD).

**Member 2: Entities, Spawning & Collision Detection**
* Designed the parallel array architecture (`carX`, `carRow`, `carSpeed`, `carDir`, `carActive`) for managing vehicles.
* Implemented procedural car spawning (`spawnCar`) and dynamic difficulty scaling that increases speed and frequency over time.
* Coded frame-rate independent movement (`updateCars`) using delta time and cleanup for off-screen cars.
* Implemented precise axis-aligned bounding box collision detection between the player and vehicles.

**Member 3: Game Flow, Controls & Main Architecture**
* Orchestrated the main application loop, window initialization, frame-rate limiting (60 FPS), and resource loading.
* Developed real-time event handling (`handleEvents`) for keyboard inputs (Arrow keys, WASD, pause toggle, restart).
* Managed overall game states (`STATE_PLAYING`, `STATE_GAMEOVER`) and pause restrictions tied to safe grass zones.
* Designed the layered rendering pipeline and UI overlay screens for Game Over and Pause menus.
