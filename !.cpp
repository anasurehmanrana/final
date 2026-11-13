#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <optional>

using namespace std;
using namespace sf;

// SHARED CONSTANTS AND VARIABLES

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 50;                       // one cell is 50 x 50 pixels
const int COLS = WINDOW_WIDTH / GRID_SIZE;      // 16 columns
const int ROWS = WINDOW_HEIGHT / GRID_SIZE;     // 12 rows

// rows 0, 5, 6 and 11 are grass (safe). All other rows are roads.
const int MIDDLE_ROW_1 = ROWS / 2 - 1;          // row 5
const int MIDDLE_ROW_2 = ROWS / 2;              // row 6

const int PLAYER_SIZE = 40;
const int CAR_WIDTH = 60;
const int CAR_HEIGHT = 35;
const int MAX_CARS = 40;                        // the car arrays have this many slots
const int NUM_LANES = 8;                        // 12 rows - 4 grass rows = 8 road lanes

const int STATE_PLAYING = 0;
const int STATE_GAMEOVER = 1;

const string HIGH_SCORE_FILE = "highscore.txt";

// player (position is stored as a grid cell, not pixels)
int playerCol;
int playerRow;

// cars: every car uses the same index in all five arrays
float carX[MAX_CARS];        // x position in pixels
int carRow[MAX_CARS];        // which road row it drives on
float carSpeed[MAX_CARS];    // pixels per second
int carDir[MAX_CARS];        // 1 = moves right, -1 = moves left
bool carActive[MAX_CARS];    // false = this slot is empty

int laneRows[NUM_LANES];     // the row numbers of the road lanes

// game info
int gameState = STATE_PLAYING;
bool isPaused = false;
float gameTime = 0;          // seconds since the game started
float spawnTimer = 0;        // counts up until it's time for a new car
float difficulty = 1.0f;     // grows with time
float spawnRate = 1.5f;      // seconds between cars, gets smaller with time
int score = 0;
int highScore = 0;
int highestRowReached = ROWS - 1;   // so we only give points for NEW rows

Font font;

// small drawing helpers

void drawRect(RenderWindow& window, float x, float y, float w, float h, Color color) {
    RectangleShape rect(Vector2f(w, h));
    rect.setPosition(Vector2f(x, y));
    rect.setFillColor(color);
    window.draw(rect);
}

// text in the horizontal middle of the window
void drawCenteredText(RenderWindow& window, string words, int size, float y, Color color) {
    Text text(font, words, size);
    text.setFillColor(color);
    FloatRect bounds = text.getGlobalBounds();
    float x = (WINDOW_WIDTH - bounds.size.x) / 2.0f;
    text.setPosition(Vector2f(x, y));
    window.draw(text);
}