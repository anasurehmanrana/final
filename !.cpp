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
/*
    MEMBER 1: PLAYER, SCORE, HIGH SCORE FILE, PLAYER + HUD DRAWING

    What to explain:
     - the player is just two numbers: playerCol and playerRow
     - movePlayer changes them and uses if statements to stay inside the grid
     - score goes up by 1 for every new road row we reach
     - the high score is saved to and read from a text file
     - drawPlayer and drawHUD only draw, they never change the game
*/

// put the player at the bottom middle
void resetPlayer() {
    playerCol = COLS / 2;
    playerRow = ROWS - 1;
}

// move by dx columns and dy rows (for example dy = -1 means one step up)
void movePlayer(int dx, int dy) {
    playerCol = playerCol + dx;
    playerRow = playerRow + dy;

    // don't let the player leave the screen
    if (playerCol < 0) playerCol = 0;
    if (playerCol >= COLS) playerCol = COLS - 1;
    if (playerRow < 0) playerRow = 0;
    if (playerRow >= ROWS) playerRow = ROWS - 1;
}

// true if the player is standing on grass
bool isPlayerInSafeZone() {
    if (playerRow == 0) return true;
    if (playerRow == ROWS - 1) return true;
    if (playerRow == MIDDLE_ROW_1) return true;
    if (playerRow == MIDDLE_ROW_2) return true;
    return false;
}

// +1 point each time we reach a row higher than ever before
void updateScore() {
    if (playerRow < highestRowReached) {
        highestRowReached = playerRow;
        if (isPlayerInSafeZone() == false) {
            score = score + 1;
        }
    }
}

// reaching the top row sends the player back to the start so they can go again
void checkReachedTop() {
    if (playerRow == 0) {
        playerRow = ROWS - 1;
        highestRowReached = ROWS - 1;
    }
}

// read the saved best score (if the file doesn't exist yet, it stays 0)
void loadHighScore() {
    ifstream file;
    file.open(HIGH_SCORE_FILE);
    if (file.is_open() == true) {
        file >> highScore;
        file.close();
    }
    else {
        highScore = 0;
    }
}

// write the best score to the file
void saveHighScore() {
    ofstream file;
    file.open(HIGH_SCORE_FILE);
    if (file.is_open() == true) {
        file << highScore;
        file.close();
    }
}
void drawPlayer(RenderWindow& window) {
    float x = playerCol * GRID_SIZE + (GRID_SIZE - PLAYER_SIZE) / 2.0f;
    float y = playerRow * GRID_SIZE + (GRID_SIZE - PLAYER_SIZE) / 2.0f;
    drawRect(window, x, y, PLAYER_SIZE, PLAYER_SIZE, Color::Green);
}

// score and high score at the top left
void drawHUD(RenderWindow& window) {
    // show the current score as the best one if we are beating the record
    int bestToShow = highScore;
    if (score > highScore) {
        bestToShow = score;
    }

    string line = "Score: " + to_string(score) + "   High Score: " + to_string(bestToShow);
    Text text(font, line, 24);
    text.setFillColor(Color::White);
    text.setPosition(Vector2f(15, 10));
    window.draw(text);
}
//Member 2 part


// work out which rows are roads and store them in laneRows[]
void setupLanes() {
    int count = 0;
    for (int r = 1; r < ROWS - 1; r++) {
        if (r != MIDDLE_ROW_1 && r != MIDDLE_ROW_2) {
            laneRows[count] = r;
            count++;
        }
    }
}
// make every car slot empty
void clearCars() {
    for (int i = 0; i < MAX_CARS; i++) {
        carActive[i] = false;
    }
}
// add one new car on a random lane
void spawnCar() {
    // look for the first empty slot
    for (int i = 0; i < MAX_CARS; i++) {
        if (carActive[i] == false) {

            int row = laneRows[rand() % NUM_LANES];
            carRow[i] = row;

            // cars in the same lane all drive the same way (even rows right, odd rows left)
            if (row % 2 == 0) {
                carDir[i] = 1;
                carX[i] = -CAR_WIDTH; // starts just outside the left edge
            }
            else {
                carDir[i] = -1;
                carX[i] = WINDOW_WIDTH; // starts just outside the right edge
            }

            // random speed between 100 and 250, plus more as difficulty grows
            carSpeed[i] = 100 + rand() % 151 + 50 * difficulty;

            carActive[i] = true;
            return; // we placed one car, stop looking
        }
    }
}

// gets harder over time and spawns cars when the timer runs out
void updateSpawning(float dt) {
    gameTime = gameTime + dt;
    difficulty = 1.0f + gameTime / 10.0f;

    spawnRate = 1.5f - gameTime / 20.0f;
    if (spawnRate < 0.3f) {
        spawnRate = 0.3f; // never faster than one car every 0.3 s
    }

    spawnTimer = spawnTimer + dt;
    if (spawnTimer >= spawnRate) {
        spawnTimer = 0;
        spawnCar();
    }
}

// move every active car and free the ones that left the screen
void updateCars(float dt) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (carActive[i] == true) {
            carX[i] = carX[i] + carSpeed[i] * carDir[i] * dt;

            if (carDir[i] == 1 && carX[i] > WINDOW_WIDTH) {
                carActive[i] = false;
            }
            if (carDir[i] == -1 && carX[i] < -CAR_WIDTH) {
                carActive[i] = false;
            }
        }
    }
}

// true if any car touches the player
bool checkCollision() {
    // the player's rectangle (top-left corner + size)
    float playerLeft = playerCol * GRID_SIZE + (GRID_SIZE - PLAYER_SIZE) / 2.0f;
    float playerTop = playerRow * GRID_SIZE + (GRID_SIZE - PLAYER_SIZE) / 2.0f;

    for (int i = 0; i < MAX_CARS; i++) {
        if (carActive[i] == true) {
            // the car's rectangle
            float carLeft = carX[i];
            float carTop = carRow[i] * GRID_SIZE + (GRID_SIZE - CAR_HEIGHT) / 2.0f;

            // two rectangles overlap when they overlap both horizontally AND vertically
            bool overlapX = playerLeft < carLeft + CAR_WIDTH && playerLeft + PLAYER_SIZE > carLeft;
            bool overlapY = playerTop < carTop + CAR_HEIGHT && playerTop + PLAYER_SIZE > carTop;

            if (overlapX == true && overlapY == true) {
                return true;
            }
        }
    }
    return false;
}

// gray road everywhere, then green grass on the 4 safe rows
void drawBackground(RenderWindow& window) {
    window.clear(Color(50, 50, 50));

    Color grass(100, 200, 100);
    drawRect(window, 0, 0, WINDOW_WIDTH, GRID_SIZE, grass); // top
    drawRect(window, 0, MIDDLE_ROW_1 * GRID_SIZE, WINDOW_WIDTH, GRID_SIZE * 2, grass); // middle (2 rows)
    drawRect(window, 0, WINDOW_HEIGHT - GRID_SIZE, WINDOW_WIDTH, GRID_SIZE, grass); // bottom
}

void drawCars(RenderWindow& window) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (carActive[i] == true) {
            float y = carRow[i] * GRID_SIZE + (GRID_SIZE - CAR_HEIGHT) / 2.0f;
            drawRect(window, carX[i], y, CAR_WIDTH, CAR_HEIGHT, Color::Red);
        }
    }
}
