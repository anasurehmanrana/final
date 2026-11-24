# Game Description:

  Lane Dasher is a 2D grid-based arcade game in C++ and SFML 3 where you guide a frog across eight                              bidirectional traffic lanes on a 16 × 12 grid to reach the top row and complete laps. Advancing up road                       lanes earns points, every 50 points grants a Turbo jump (Space) to the next safe grass zone, traffic                          speeds up and spawns faster over time, and high scores persist in highscore.txt.

# Lane Dasher — Implemented Features:

~~ Grid & Traffic:
                   16 × 12 grid with WASD/Arrow movement, 4 grass safe rows, 8 road lanes (even rows drive                                       right, odd rows drive left), lap tracking on top-row reach, and centered collision hitboxes.

~~ Scoring & Turbo:
                    +1 point per new higher road row (no grass farming), +1 Turbo charge every 50 points, and                                     Space Turbo jumps to the next safe zone (row 6 or row 0) for +10 points when advancing upward.

~~ Difficulty Scaling:
                       Car speed increases over time and spawn intervals tighten from 1.5s down to 0.3s.

~~ Visuals & Audio: 
                    Custom frog, grass, and repeating lane textures, rotated tinted cars with drop shadows, live top HUD, SFX                     (jump, crash, point, select), and looping background music.

~~ Menus, Pause & Save:
                        Main Menu, High Score screen, Game Over screen, grass-only pause (P), and persistent high scores                              saved to highscore.txt.

# Lane Dasher — Quick Player Guide:

~~ Goal:
         Cross 8 lanes of traffic (rows 1–4 and 7–10) from the bottom grass (row 11) to the top grass (row 0). Reaching the            top awards +1 Lap and resets you to the bottom. Rows 5–6 act as a safe middle refuge.

~~ Controls: 
            Move: W/A/S/D or Arrow Keys
            Turbo Jump: Space (takes the player to next safe zone)
            Pause: P (only works while standing on grass)
            Menus: Up/Down or W/S + Enter (Esc to go back; Space on Game Over to quick-restart)

~~ Scoring & Turbo:
                    +1 pt for each new road row reached.
                    +10 pts when jumping to a higher row using Turbo.
                    Every 50 points earns +1 Turbo charge.

# Compile & Run Checklist:

~~ Link Libraries: 
                   Link SFML Graphics and SFML Audio (built for SFML 3), and place matching SFML runtime DLLs next to the                        executable if dynamically linked.

~~ Required Assets (in working directory):
                   car.png, grass.png, frog.png, lane.png, and arial.ttf (falls back to C:/Windows/Fonts/arial.ttf)—the app                      exits if any of these fail to load.

~~ Optional Audio & Save File:
                   jump.wav, crash.wav, point.wav, select.wav, and music.wav log a console error if missing without exiting;                     highscore.txt is created/updated automatically.
