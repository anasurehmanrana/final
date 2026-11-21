// Lane Dasher - Group 5

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <optional>

using namespace std;
using namespace sf;


// SHARED CONSTANTS AND VARIABLES

const int GRID_SIZE = 65;                       // was 50 (makes each lane taller)
const int WINDOW_WIDTH = 16 * GRID_SIZE;        // 1040 pixels wide
const int WINDOW_HEIGHT = 12 * GRID_SIZE;       // 780 pixels tall
const int COLS = WINDOW_WIDTH / GRID_SIZE;      // still 16 columns
const int ROWS = WINDOW_HEIGHT / GRID_SIZE;     // still 12 rows

// rows 0, 5, 6 and 11 are grass (safe). All other rows are roads.
const int MIDDLE_ROW_1 = ROWS / 2 - 1;          // row 5
const int MIDDLE_ROW_2 = ROWS / 2;              // row 6

const int PLAYER_SIZE = 40;
const int CAR_WIDTH = 105;
const int CAR_HEIGHT = 46;
const int MAX_CARS = 40;                        // the car arrays have this many slots
const int NUM_LANES = 8;                        // 12 rows - 4 grass rows = 8 road lanes

const int STATE_PLAYING = 0;
const int STATE_GAMEOVER = 1;
const int STATE_MENU = 2;       // start screen
const int STATE_HIGHSCORE = 3;  // high score screen
int gameState = STATE_MENU;     // game starts on the menu
int selectedOption = 0;         // 0 = Start Game, 1 = High Score, 2 = Quit
int gameOverOption = 0;         // 0 = Play Again, 1 = Main Menu, 2 = Exit
const string HIGH_SCORE_FILE = "highscore.txt";

// player (position is stored as a grid cell, not pixels)
int playerCol;
int playerRow;
int turboCharges = 0;
// cars: every car uses the same index in all five arrays
float carX[MAX_CARS];        // x position in pixels
int carRow[MAX_CARS];        // which road row it drives on
float carSpeed[MAX_CARS];    // pixels per second
int carDir[MAX_CARS];        // 1 = moves right, -1 = moves left
bool carActive[MAX_CARS];    // false = this slot is empty

int laneRows[NUM_LANES];     // the row numbers of the road lanes

// game info
bool isPaused = false;
float gameTime = 0;          // seconds since the game started
float spawnTimer = 0;        // counts up until it's time for a new car
float difficulty = 1.0f;     // grows with time
float spawnRate = 1.5f;      // seconds between cars, gets smaller with time
int score = 0;
int highScore = 0;
int highestRowReached = ROWS - 1;   // so we only give points for NEW rows

Font font;

Texture carTexture;
Color carColors[MAX_CARS]; // Gives each car a random paint job
Texture grassTexture;
Texture frogTexture;
Texture laneTexture;
// small drawing helpers

Music bgMusic;
bool musicLoaded = false;

SoundBuffer jumpBuffer;
Sound jumpSound(jumpBuffer);

SoundBuffer crashBuffer;
Sound crashSound(crashBuffer);

SoundBuffer pointBuffer;
Sound pointSound(pointBuffer);

SoundBuffer selectBuffer;
Sound selectSound(selectBuffer);



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
    if (jumpBuffer.getSampleCount() > 0) jumpSound.play();
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
            int oldScore = score;
            score = score + 1;

            if ((score / 50) > (oldScore / 50)) {
                turboCharges = turboCharges + 1;
            }
        }
    }
}

// reaching the top row sends the player back to the start so they can go again
void checkReachedTop() {
    if (playerRow == 0) {
        if (pointBuffer.getSampleCount() > 0) pointSound.play();
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
void drawFrog(RenderWindow& window) {
    // Center of the player's grid cell in pixels
    float centerX = playerCol * GRID_SIZE + GRID_SIZE / 2.0f;
    float centerY = playerRow * GRID_SIZE + GRID_SIZE / 2.0f;

    Sprite frogSprite(frogTexture);

    // Set origin to the middle of the frog image
    Vector2u texSize = frogTexture.getSize();
    frogSprite.setOrigin(Vector2f(texSize.x / 2.0f, texSize.y / 2.0f));

    // Scale to fit nicely inside PLAYER_SIZE (40x40)
    float scaleX = (float)PLAYER_SIZE / texSize.x;
    float scaleY = (float)PLAYER_SIZE / texSize.y;
    float scale = (scaleX < scaleY) ? scaleX : scaleY; // keeps proportions
    frogSprite.setScale(Vector2f(scale, scale));

    frogSprite.setPosition(Vector2f(centerX, centerY));
    window.draw(frogSprite);
}

void drawPlayer(RenderWindow& window) {
    drawFrog(window);
}
// score and high score at the top left
void drawHUD(RenderWindow& window) {
    // show the current score as the best one if we are beating the record
    int bestToShow = highScore;
    if (score > highScore) {
        bestToShow = score;
    }

    string line = "Score: " + to_string(score) + "   High Score: " + to_string(bestToShow) + "   Turbo [Space]: " + to_string(turboCharges);
    Text text(font, line, 24);
    text.setFillColor(Color::White);
    text.setPosition(Vector2f(15, 10));
    window.draw(text);
}

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
            Color palette[5] = {
    Color::White,               // Original texture color
    Color(220, 50, 50),         // Sports Red
    Color(50, 130, 230),        // Electric Blue
    Color(240, 190, 40),        // Taxi / Muscle Yellow
    Color(180, 180, 190)        // Metallic Silver
            };
            carColors[i] = palette[rand() % 5];

            carActive[i] = true;

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

void drawGrassArea(RenderWindow& window, float y, int width, int height) {
    float scale = 0.25f;

    Sprite grassSprite(grassTexture);
    grassSprite.setTextureRect(IntRect({ 0, 0 }, { (int)(width / scale), (int)(height / scale) }));
    grassSprite.setScale(Vector2f(scale, scale));
    grassSprite.setPosition(Vector2f(0, y));
    grassSprite.setColor(Color(210, 230, 210));

    window.draw(grassSprite);
}

void drawLaneArea(RenderWindow& window, float y, int width, int height) {
    // Fit 1 image height to 1 grid row (50 pixels = 1 lane)
    float scale = (float)GRID_SIZE / laneTexture.getSize().y;

    Sprite laneSprite(laneTexture);
    laneSprite.setTextureRect(IntRect({ 0, 0 }, { (int)(width / scale), (int)(height / scale) }));
    laneSprite.setScale(Vector2f(scale, scale));
    laneSprite.setPosition(Vector2f(0, y));

    window.draw(laneSprite);
}

void drawBackground(RenderWindow& window) {
    window.clear(Color(50, 50, 50));

    // Single-lane road strips (rows 1-4 and rows 7-10)
    drawLaneArea(window, 1 * GRID_SIZE, WINDOW_WIDTH, GRID_SIZE * 4);
    drawLaneArea(window, 7 * GRID_SIZE, WINDOW_WIDTH, GRID_SIZE * 4);

    drawGrassArea(window, 0, WINDOW_WIDTH, GRID_SIZE);                            // top
    drawGrassArea(window, MIDDLE_ROW_1 * GRID_SIZE, WINDOW_WIDTH, GRID_SIZE * 2); // middle (2 rows)
    drawGrassArea(window, WINDOW_HEIGHT - GRID_SIZE, WINDOW_WIDTH, GRID_SIZE);    // bottom
}

// Draw all active high-resolution car sprites with shadows
void drawCars(RenderWindow& window) {
    Vector2u texSize = carTexture.getSize();

    for (int i = 0; i < MAX_CARS; i++) {
        if (carActive[i] == true) {
            float x = carX[i];
            float y = carRow[i] * GRID_SIZE + (GRID_SIZE - CAR_HEIGHT) / 2.0f;

            Sprite carSprite(carTexture);
            carSprite.setOrigin(Vector2f(texSize.x / 2.0f, texSize.y / 2.0f));
            // 2. Rotate 90 degrees clockwise when moving right (1), or -90 degrees when moving left (-1)
            carSprite.setRotation(degrees(90.f * carDir[i]));

            float baseScale = (float)CAR_HEIGHT / texSize.x;
            float widthScale = baseScale * 1.6f;   // Controls car width (top-to-bottom in lane)
            float lengthScale = baseScale * 2.6f;  // Controls car length (bumper-to-bumper, ~2x width!)
            carSprite.setScale(Vector2f(widthScale,lengthScale));

            float centerX = x + CAR_WIDTH / 2.0f;
            float centerY = y + CAR_HEIGHT / 2.0f;

            // 1. Draw Drop Shadow underneath
            Sprite shadowSprite = carSprite;
            shadowSprite.setPosition(Vector2f(centerX + 4.0f, centerY + 6.0f));
            shadowSprite.setColor(Color(0, 0, 0, 110));
            window.draw(shadowSprite);

            // 2. Draw High-Detail Car on top
            carSprite.setPosition(Vector2f(centerX, centerY));
            carSprite.setColor(carColors[i]);
            window.draw(carSprite);
        }
    }
}

// drawing the screens 
// draws the main menu screen
void drawMenu(RenderWindow& window) {
    // dark background and title
    drawRect(window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(20, 20, 30));
    drawRect(window, 0, 0, WINDOW_WIDTH, 30, Color(40, 40, 60));
    drawRect(window, 0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30, Color(40, 40, 60));
    // 2. decorative car sprites on the left and right sides
    Vector2u carSize = carTexture.getSize();

    Sprite leftCar(carTexture);
    leftCar.setOrigin(Vector2f(carSize.x / 2.0f, carSize.y / 2.0f));
    leftCar.setRotation(degrees(90.f));
    leftCar.setScale(Vector2f(0.15f, 0.24f));
    leftCar.setColor(Color(220, 50, 50));
    leftCar.setPosition(Vector2f(120, 360));
    window.draw(leftCar);

    Sprite rightCar(carTexture);
    rightCar.setOrigin(Vector2f(carSize.x / 2.0f, carSize.y / 2.0f));
    rightCar.setRotation(degrees(-90.f));
    rightCar.setScale(Vector2f(0.15f, 0.24f));
    rightCar.setColor(Color(50, 130, 230));
    rightCar.setPosition(Vector2f(WINDOW_WIDTH - 120, 360));
    window.draw(rightCar);

    drawCenteredText(window, "LANE DASHER", 60, 120, Color::Cyan);

    // option 0: Start Game
    if (selectedOption == 0) {
        drawCenteredText(window, "> Start Game", 32, 280, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  Start Game", 32, 280, Color::White);
    }

    // option 1: High Score
    if (selectedOption == 1) {
        drawCenteredText(window, "> High Score", 32, 340, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  High Score", 32, 340, Color::White);
    }

    // option 2: Quit
    if (selectedOption == 2) {
        drawCenteredText(window, "> Quit", 32, 400, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  Quit", 32, 400, Color::White);
    }
    // 5. simple key hint at the bottom
    drawCenteredText(window, "Up/Down or W/S to Move   |   Enter to Select", 22, 545, Color(180, 180, 180));
}

// draws the high score screen
void drawHighScoreScreen(RenderWindow& window) {
    drawRect(window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(20, 20, 30));
    drawRect(window, 0, 0, WINDOW_WIDTH, 30, Color(40, 40, 60));
    drawRect(window, 0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30, Color(40, 40, 60));
    // 2. clean accent bar behind the high score value
    drawRect(window, WINDOW_WIDTH / 2.0f - 180, 240, 360, 95, Color(35, 35, 52));
    drawCenteredText(window, "HIGH SCORE", 50, 150, Color::Yellow);
    drawCenteredText(window, to_string(highScore), 60, 260, Color::White);
    drawCenteredText(window, "Press ESC to Return to Menu", 24, 450, Color(180, 180, 180));
}
void drawGameOver(RenderWindow& window) {
    drawRect(window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(20, 20, 30));
    drawRect(window, 0, 0, WINDOW_WIDTH, 30, Color(40, 40, 60));
    drawRect(window, 0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30, Color(40, 40, 60));

    drawCenteredText(window, "GAME OVER", 60, 110, Color::Red);
    drawCenteredText(window, "Score: " + to_string(score) + "   |   High Score: " + to_string(highScore), 28, 195, Color::White);

    if (gameOverOption == 0) {
        drawCenteredText(window, "> Play Again", 32, 280, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  Play Again", 32, 280, Color::White);
    }

    if (gameOverOption == 1) {
        drawCenteredText(window, "> Main Menu", 32, 340, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  Main Menu", 32, 340, Color::White);
    }

    if (gameOverOption == 2) {
        drawCenteredText(window, "> Exit", 32, 400, Color::Yellow);
    }
    else {
        drawCenteredText(window, "  Exit", 32, 400, Color::White);
    }

    drawCenteredText(window, "Up/Down or W/S to Move   |   Enter to Select", 22, 545, Color(180, 180, 180));
}

void drawPause(RenderWindow& window) {
    drawRect(window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color(0, 0, 0, 150));
    drawCenteredText(window, "PAUSED", 60, 250, Color::White);
}

// the car hit us: save the high score and show the game over screen
void triggerGameOver() {
    if (musicLoaded) {
        bgMusic.stop();
    }
    if (crashBuffer.getSampleCount() > 0) crashSound.play();
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
    gameOverOption = 0;
    gameState = STATE_GAMEOVER;
}

// reset everything for a new round
void restartGame() {
    turboCharges = 0;
    if (musicLoaded) {
        bgMusic.play();
    }
    resetPlayer();
    clearCars();
    gameTime = 0;
    spawnTimer = 0;
    difficulty = 1.0f;
    spawnRate = 1.5f;
    score = 0;
    highestRowReached = ROWS - 1;
    isPaused = false;
    gameState = STATE_PLAYING;
}

// everything that changes in one frame while the game is running
void updateGame(float dt) {
    updateSpawning(dt);          
    updateCars(dt);              
    updateScore();              
    if (checkCollision() == true) {
        triggerGameOver();
    }
    checkReachedTop();          
}

// read the keyboard
void handleEvents(RenderWindow& window) {

    while (const optional event = window.pollEvent()) {

        if (event->is<Event::Closed>()) {
            window.close();
        }

        const Event::KeyPressed* keyPress = event->getIf<Event::KeyPressed>();
        if (keyPress != nullptr) {
            Keyboard::Key key = keyPress->code;
            if (gameState == STATE_MENU) {
                // move selection up and wrap around
                if (key == Keyboard::Key::Up || key == Keyboard::Key::W) {
                    selectedOption = selectedOption - 1;
                    if (selectedOption < 0) {
                        selectedOption = 2;
                    }
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                }
                // move selection down and wrap around
                else if (key == Keyboard::Key::Down || key == Keyboard::Key::S) {
                    selectedOption = selectedOption + 1;
                    if (selectedOption > 2) {
                        selectedOption = 0;
                    }
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                }
                // run selected option on Enter
                else if (key == Keyboard::Key::Enter) {
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                    if (selectedOption == 0) {
                        restartGame();
                    }
                    else if (selectedOption == 1) {
                        loadHighScore();
                        gameState = STATE_HIGHSCORE;
                    }
                    else if (selectedOption == 2) {
                        window.close();
                    }
                }
            }
            else if (gameState == STATE_HIGHSCORE) {
                // Esc returns to menu
                if (key == Keyboard::Key::Escape) {
                    gameState = STATE_MENU;
                }
            }
            else if (gameState == STATE_GAMEOVER) {
                if (key == Keyboard::Key::Up || key == Keyboard::Key::W) {
                    gameOverOption = gameOverOption - 1;
                    if (gameOverOption < 0) {
                        gameOverOption = 2;
                    }
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                }
                else if (key == Keyboard::Key::Down || key == Keyboard::Key::S) {
                    gameOverOption = gameOverOption + 1;
                    if (gameOverOption > 2) {
                        gameOverOption = 0;
                    }
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                }
                else if (key == Keyboard::Key::Enter) {
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                    if (gameOverOption == 0) {
                        restartGame();
                    }
                    else if (gameOverOption == 1) {
                        selectedOption = 0;
                        gameState = STATE_MENU;
                    }
                    else if (gameOverOption == 2) {
                        window.close();
                    }
                }
                else if (key == Keyboard::Key::Space) {
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                    restartGame();
                }
                else if (key == Keyboard::Key::Escape) {
                    if (selectBuffer.getSampleCount() > 0) selectSound.play();
                    selectedOption = 0;
                    gameState = STATE_MENU;
                }
            }
            else {
                // P toggles pause (we can only pause while standing on grass)
                if (key == Keyboard::Key::P) {
                    if (isPaused == true) {
                        isPaused = false;
                    }
                    else if (isPlayerInSafeZone() == true) {
                        isPaused = true;
                    }
                }

                // movement (ignored while paused)
                if (isPaused == false) {
                    if (key == Keyboard::Key::Up || key == Keyboard::Key::W) {
                        movePlayer(0, -1);
                    }
                    else if (key == Keyboard::Key::Down || key == Keyboard::Key::S) {
                        movePlayer(0, 1);
                    }
                    else if (key == Keyboard::Key::Left || key == Keyboard::Key::A) {
                        movePlayer(-1, 0);
                    }
                    else if (key == Keyboard::Key::Right || key == Keyboard::Key::D) {
                        movePlayer(1, 0);
                    }
                    else if (key == Keyboard::Key::Space) {
                        if (turboCharges > 0) {
                            turboCharges = turboCharges - 1;
                            if (playerRow > MIDDLE_ROW_2) {
                                movePlayer(0, -5);
                            }
                            else {
                                movePlayer(0, -6);
                            }
                            if (pointBuffer.getSampleCount() > 0) pointSound.play();
                        }
                    }
                }
            }
        }
    }
}

int main() {
    srand(time(NULL));    // so the cars are different every time we play

    RenderWindow window(VideoMode(Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Lane Dasher");
    window.setFramerateLimit(60);

    grassTexture.setSmooth(true);

    if (!jumpBuffer.loadFromFile("jump.wav")) {
        cout << "Error: Could not load jump.wav!" << endl;
    }
    if (!crashBuffer.loadFromFile("crash.wav")) {
        cout << "Error: Could not load crash.wav!" << endl;
    }
    if (!pointBuffer.loadFromFile("point.wav")) {
        cout << "Error: Could not load point.wav!" << endl;
    }
    if (!selectBuffer.loadFromFile("select.wav")) {
        cout << "Error: Could not load select.wav!" << endl;
    }

    if (!bgMusic.openFromFile("music.wav")) {
        cout << "Error: Could not load music.wav!" << endl;
    }
    else {
        bgMusic.setLooping(true);
        musicLoaded = true;
    }
    // set everything up once
    // text needs a font file
    if (font.openFromFile("arial.ttf") == false) {
        if (font.openFromFile("C:/Windows/Fonts/arial.ttf") == false) {
            cout << "Could not find a font. Put arial.ttf next to the program." << endl;
            return 1;
        }
    }
    if (!carTexture.loadFromFile("car.png")) {
        cout << "Error: Could not load car.png!" << endl;
        return -1;
    }
    carTexture.setSmooth(true);
    if (!grassTexture.loadFromFile("grass.png")) {
        cout << "Could not find grass.png" << endl;
        return 1;
    }
    grassTexture.setRepeated(true);

    if (!frogTexture.loadFromFile("frog.png")) {
        cout << "Error: Could not load frog.png!" << endl;
        return -1;
    }
    frogTexture.setSmooth(true);

    laneTexture.setSmooth(true);
    if (!laneTexture.loadFromFile("lane.png")) {
        cout << "Could not find lane.png" << endl;
        return 1;
    }
    laneTexture.setRepeated(true);
   
   
    setupLanes();
    clearCars();
    resetPlayer();
    loadHighScore();

    Clock gameClock;

    // the main game loop: runs until the window is closed
    while (window.isOpen() == true) {
        float dt = gameClock.restart().asSeconds();   // seconds since the last frame

        handleEvents(window);

        if (gameState == STATE_PLAYING && isPaused == false) {
            updateGame(dt);
        }

        // draw (always in this order: back to front)
        if (gameState == STATE_MENU) {
            drawMenu(window);
        }
        else if (gameState == STATE_HIGHSCORE) {
            drawHighScoreScreen(window);
        }
        else if (gameState == STATE_GAMEOVER) {
            drawGameOver(window);
        }
        else {
            drawBackground(window);     
            drawCars(window);           
            drawPlayer(window);         
            drawHUD(window);            

            if (isPaused == true) {
                drawPause(window);
            }
        }
        window.display();
     
    }

    saveHighScore();
    return 0;
}
