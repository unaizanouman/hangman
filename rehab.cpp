#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cmath> // Required for sound generation

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// ----------------------------------------------------------------------------------
// Constants & Colors
// ----------------------------------------------------------------------------------
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

// Professional Color Palette
const Color BG_TOP = { 20, 25, 40, 255 };
const Color BG_BOTTOM = { 40, 45, 70, 255 };
const Color ACCENT_COLOR = { 100, 200, 255, 255 };
const Color HINT_COLOR = { 255, 200, 100, 255 };
const Color BTN_NORMAL = { 60, 70, 90, 255 };
const Color BTN_HOVER = { 80, 90, 120, 255 };
const Color BTN_WRONG = { 200, 60, 60, 255 };
const Color BTN_CORRECT = { 60, 200, 100, 255 };

// ----------------------------------------------------------------------------------
// Audio Generator (The Fix for Missing Sounds)
// ----------------------------------------------------------------------------------
// 0: Sine, 1: Square, 2: Noise, 3: Sawtooth
Wave GenerateSoundWave(int type, float duration, int freq) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *data = (short *)malloc(frameCount * sizeof(short));
    
    for (int i = 0; i < frameCount; i++) {
        float t = (float)i / sampleRate;
        float sample = 0.0f;
        
        if (type == 0) sample = sinf(2.0f * PI * freq * t); // Sine
        else if (type == 1) sample = (sinf(2.0f * PI * freq * t) > 0) ? 0.5f : -0.5f; // Square
        else if (type == 2) sample = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // Noise
        else if (type == 3) { // Sawtooth
            float period = (float)sampleRate / freq;
            sample = 2.0f * (fmod((float)i, period) / period) - 1.0f;
        }
        
        // Fade Out to prevent popping
        if (i > frameCount - 1000) {
            float fade = (float)(frameCount - i) / 1000.0f;
            sample *= fade;
        }

        data[i] = (short)(sample * 10000); // Volume (Max 32767)
    }

    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = data;
    return wave;
}

// ----------------------------------------------------------------------------------
// Structures
// ----------------------------------------------------------------------------------
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    VICTORY,
    GAME_COMPLETE
};

struct VirtualKey {
    char letter;
    Rectangle rect;
    int state; // 0: Unpressed, 1: Correct, 2: Wrong
    bool mouseHover;
};

struct WordData {
    std::string word;
    std::string hint;
};

// ----------------------------------------------------------------------------------
// Global Variables
// ----------------------------------------------------------------------------------
std::vector<std::vector<WordData>> gameLevels;
int currentLevel = 0;
const int TOTAL_LEVELS = 3;

std::string targetWord;
std::string targetHint;
std::string displayWord;
int lives;
const int MAX_LIVES = 6;
GameState currentState = MENU;
std::vector<VirtualKey> keyboard;

// Sound Effects
Sound sndCorrect;
Sound sndWrong;
Sound sndWin;
Sound sndLose;

// ----------------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------------

void InitGameData() {
    gameLevels.clear();

    // --- LEVEL 1: EASY ---
    std::vector<WordData> level1;
    level1.push_back({"PIXEL", "The smallest unit of a digital image."});
    level1.push_back({"CODE", "Instructions for a computer."});
    level1.push_back({"BUG", "An error in a program."});
    level1.push_back({"RAM", "Temporary computer memory."});
    level1.push_back({"LOOP", "Repeating a block of code."});
    level1.push_back({"DATA", "Information processed by a computer."});
    level1.push_back({"WIFI", "Wireless networking technology."});
    level1.push_back({"JAVA", "A popular programming language (and coffee)."});
    level1.push_back({"MOUSE", "Handheld pointing device."});
    level1.push_back({"FILE", "A resource for storing information."});
    gameLevels.push_back(level1);

    // --- LEVEL 2: MEDIUM ---
    std::vector<WordData> level2;
    level2.push_back({"POINTER", "A variable that stores a memory address."});
    level2.push_back({"ARRAY", "A collection of items stored at contiguous memory."});
    level2.push_back({"SYNTAX", "The grammar rules of a programming language."});
    level2.push_back({"STRING", "A sequence of characters."});
    level2.push_back({"BINARY", "A system of zeros and ones."});
    level2.push_back({"SERVER", "A computer that provides data to others."});
    level2.push_back({"PYTHON", "A snake, but also a coding language."});
    level2.push_back({"DRIVER", "Software that controls hardware."});
    level2.push_back({"KERNEL", "The core part of an operating system."});
    level2.push_back({"SOCKET", "Endpoint for sending or receiving data."});
    gameLevels.push_back(level2);

    // --- LEVEL 3: HARD ---
    std::vector<WordData> level3;
    level3.push_back({"ALGORITHM", "A step-by-step procedure for solving a problem."});
    level3.push_back({"COMPILER", "Translates code into machine language."});
    level3.push_back({"POLYMORPHISM", "Objects of different types treated as the same."});
    level3.push_back({"RECURSION", "When a function calls itself."});
    level3.push_back({"ENCRYPTION", "Encoding data to prevent unauthorized access."});
    level3.push_back({"ABSTRACTION", "Hiding complex implementation details."});
    level3.push_back({"INHERITANCE", "Deriving a class from another class."});
    level3.push_back({"BANDWIDTH", "Maximum data transfer rate."});
    level3.push_back({"FIREWALL", "Network security system."});
    level3.push_back({"DEBUGGING", "The process of finding and fixing bugs."});
    gameLevels.push_back(level3);
}

void InitKeyboard() {
    keyboard.clear();
    int startX = 100;
    int startY = 480; 
    int keySize = 50;
    int padding = 10;
    int cols = 13; 

    for (int i = 0; i < 26; i++) {
        VirtualKey key;
        key.letter = 'A' + i;
        key.state = 0;
        
        int row = i / cols;
        int col = i % cols;

        key.rect = Rectangle{ 
            (float)(startX + col * (keySize + padding)), 
            (float)(startY + row * (keySize + padding)), 
            (float)keySize, 
            (float)keySize 
        };
        key.mouseHover = false;
        keyboard.push_back(key);
    }
}

void StartLevel() {
    lives = MAX_LIVES;
    int randomIndex = rand() % gameLevels[currentLevel].size();
    targetWord = gameLevels[currentLevel][randomIndex].word;
    targetHint = gameLevels[currentLevel][randomIndex].hint;
    
    displayWord = "";
    for (size_t i = 0; i < targetWord.length(); i++) {
        displayWord += "_";
    }
    InitKeyboard();
    currentState = PLAYING;
}

void DrawGradientBackground() {
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BG_TOP, BG_BOTTOM);
}

void DrawGallowsAndMan(int mistakes) {
    int baseX = 200;
    int baseY = 350;

    DrawLineEx(Vector2{(float)baseX - 50, (float)baseY}, Vector2{(float)baseX + 50, (float)baseY}, 5, ACCENT_COLOR); 
    DrawLineEx(Vector2{(float)baseX, (float)baseY}, Vector2{(float)baseX, (float)baseY - 250}, 5, ACCENT_COLOR);     
    DrawLineEx(Vector2{(float)baseX, (float)baseY - 250}, Vector2{(float)baseX + 100, (float)baseY - 250}, 5, ACCENT_COLOR); 
    DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 250}, Vector2{(float)baseX + 100, (float)baseY - 200}, 3, ACCENT_COLOR); 

    Color manColor = RAYWHITE;
    if (mistakes >= 1) DrawCircleLines(baseX + 100, baseY - 180, 20, manColor);
    if (mistakes >= 2) DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 160}, Vector2{(float)baseX + 100, (float)baseY - 100}, 3, manColor);
    if (mistakes >= 3) DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 140}, Vector2{(float)baseX + 70, (float)baseY - 120}, 3, manColor);
    if (mistakes >= 4) DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 140}, Vector2{(float)baseX + 130, (float)baseY - 120}, 3, manColor);
    if (mistakes >= 5) DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 100}, Vector2{(float)baseX + 80, (float)baseY - 50}, 3, manColor);
    if (mistakes >= 6) DrawLineEx(Vector2{(float)baseX + 100, (float)baseY - 100}, Vector2{(float)baseX + 120, (float)baseY - 50}, 3, manColor);
}

// ----------------------------------------------------------------------------------
// Main Entry Point
// ----------------------------------------------------------------------------------
int main() {
    srand(time(0)); 

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ultimate Hangman");
    SetTargetFPS(60); 

    // --- AUDIO GENERATION (No external files needed) ---
    InitAudioDevice(); 
    
    // Correct (Ding - High Sine)
    Wave wCorrect = GenerateSoundWave(0, 0.3f, 880); 
    sndCorrect = LoadSoundFromWave(wCorrect);
    UnloadWave(wCorrect);

    // Wrong (Buzz - Low Sawtooth)
    Wave wWrong = GenerateSoundWave(3, 0.4f, 150); 
    sndWrong = LoadSoundFromWave(wWrong);
    UnloadWave(wWrong);

    // Win (Fanfare - Long Sine)
    Wave wWin = GenerateSoundWave(0, 0.8f, 660); 
    sndWin = LoadSoundFromWave(wWin);
    UnloadWave(wWin);

    // Lose (Crash - Noise)
    Wave wLose = GenerateSoundWave(2, 0.8f, 0); 
    sndLose = LoadSoundFromWave(wLose);
    UnloadWave(wLose);

    InitGameData();
    currentLevel = 0;
    currentState = MENU; 

    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();

        if (currentState == MENU || currentState == GAME_COMPLETE) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentLevel = 0; 
                StartLevel();
            }
        }
        else if (currentState == PLAYING) {
            int keyPressed = GetKeyPressed();
            bool inputProcessed = false;
            char charInput = 0;

            if (keyPressed >= 65 && keyPressed <= 90) {
                charInput = (char)keyPressed;
                inputProcessed = true;
            }

            for (auto &key : keyboard) {
                if (CheckCollisionPointRec(mousePoint, key.rect)) {
                    key.mouseHover = true;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && key.state == 0) {
                        charInput = key.letter;
                        inputProcessed = true;
                    }
                } else {
                    key.mouseHover = false;
                }
            }

            if (inputProcessed && charInput != 0) {
                bool alreadyPressed = false;
                for(const auto &key : keyboard) {
                    if (key.letter == charInput && key.state != 0) {
                        alreadyPressed = true;
                        break;
                    }
                }
                
                if (!alreadyPressed) {
                    bool found = false;
                    for(auto &key : keyboard) {
                        if(key.letter == charInput && key.state == 0) {
                            for (size_t i = 0; i < targetWord.length(); i++) {
                                if (targetWord[i] == charInput) {
                                    displayWord[i] = charInput;
                                    found = true;
                                }
                            }
                            if (found) {
                                key.state = 1; // Correct
                                PlaySound(sndCorrect); 
                            } else {
                                key.state = 2; // Wrong
                                lives--;
                                PlaySound(sndWrong); 
                            }
                        }
                    }

                    bool complete = true;
                    for(char c : displayWord) {
                        if(c == '_') complete = false;
                    }
                    
                    if(complete) {
                        currentState = VICTORY;
                        PlaySound(sndWin); 
                    }
                    if(lives <= 0) {
                        currentState = GAME_OVER;
                        PlaySound(sndLose); 
                    }
                }
            }
        }
        else if (currentState == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) StartLevel(); 
        }
        else if (currentState == VICTORY) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentLevel++;
                if (currentLevel >= TOTAL_LEVELS) currentState = GAME_COMPLETE;
                else StartLevel();
            }
        }

        BeginDrawing();
            DrawGradientBackground();

            if (currentState == MENU) {
                const char* title = "ULTIMATE HANGMAN";
                DrawText(title, SCREEN_WIDTH/2 - MeasureText(title, 60)/2, 200, 60, ACCENT_COLOR);
                DrawText("Press ENTER to Start Career", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Start Career", 20)/2, 400, 20, RAYWHITE);
            }
            else if (currentState == PLAYING) {
                DrawText(TextFormat("LEVEL %i / %i", currentLevel + 1, TOTAL_LEVELS), 20, 20, 20, ACCENT_COLOR);
                const char* diffText = (currentLevel == 0) ? "Easy" : (currentLevel == 1) ? "Medium" : "Hard";
                DrawText(diffText, 20, 50, 20, LIGHTGRAY);

                int fontSize = 50;
                std::string spacedWord = "";
                for(char c : displayWord) { spacedWord += c; spacedWord += " "; }
                int textWidth = MeasureText(spacedWord.c_str(), fontSize);
                DrawText(spacedWord.c_str(), SCREEN_WIDTH/2 - textWidth/2, 150, fontSize, RAYWHITE);

                const char* hintPrefix = "HINT: ";
                std::string fullHint = hintPrefix + targetHint;
                int hintWidth = MeasureText(fullHint.c_str(), 20);
                DrawText(fullHint.c_str(), SCREEN_WIDTH/2 - hintWidth/2, 220, 20, HINT_COLOR);

                DrawText(TextFormat("LIVES: %i", lives), SCREEN_WIDTH - 120, 20, 20, (lives < 3) ? RED : GREEN);
                DrawGallowsAndMan(MAX_LIVES - lives);

                for (const auto &key : keyboard) {
                    Color btnColor = BTN_NORMAL;
                    if (key.state == 1) btnColor = BTN_CORRECT;
                    else if (key.state == 2) btnColor = BTN_WRONG;
                    else if (key.mouseHover) btnColor = BTN_HOVER;

                    DrawRectangleRounded(key.rect, 0.3f, 6, btnColor);
                    DrawRectangleRoundedLines(key.rect, 0.3f, 6, BLACK);
                    DrawText(TextFormat("%c", key.letter), (int)key.rect.x + 15, (int)key.rect.y + 10, 30, RAYWHITE);
                }
            }
            else if (currentState == GAME_OVER) {
                DrawText("LEVEL FAILED", SCREEN_WIDTH/2 - MeasureText("LEVEL FAILED", 60)/2, 200, 60, RED);
                const char* wordText = TextFormat("Word was: %s", targetWord.c_str());
                int wordWidth = MeasureText(wordText, 30);
                DrawText(wordText, SCREEN_WIDTH/2 - wordWidth/2, 300, 30, RAYWHITE);
                DrawText("Press ENTER to Retry Level", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Retry Level", 20)/2, 500, 20, GRAY);
                DrawGallowsAndMan(6); 
            }
            else if (currentState == VICTORY) {
                DrawText("LEVEL COMPLETE!", SCREEN_WIDTH/2 - MeasureText("LEVEL COMPLETE!", 60)/2, 200, 60, GREEN);
                const char* wordText = TextFormat("Word: %s", targetWord.c_str());
                int wordWidth = MeasureText(wordText, 30);
                DrawText(wordText, SCREEN_WIDTH/2 - wordWidth/2, 300, 30, RAYWHITE);
                DrawText("Press ENTER for Next Level", SCREEN_WIDTH/2 - MeasureText("Press ENTER for Next Level", 20)/2, 500, 20, RAYWHITE);
                DrawGallowsAndMan(0); 
            }
            else if (currentState == GAME_COMPLETE) {
                DrawText("CHAMPION!", SCREEN_WIDTH/2 - MeasureText("CHAMPION!", 80)/2, 200, 80, GOLD);
                DrawText("You beat all levels!", SCREEN_WIDTH/2 - MeasureText("You beat all levels!", 30)/2, 350, 30, RAYWHITE);
                DrawText("Press ENTER to Play Again", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Play Again", 20)/2, 500, 20, GRAY);
            }

        EndDrawing();
    }

    UnloadSound(sndCorrect);
    UnloadSound(sndWrong);
    UnloadSound(sndWin);
    UnloadSound(sndLose);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}