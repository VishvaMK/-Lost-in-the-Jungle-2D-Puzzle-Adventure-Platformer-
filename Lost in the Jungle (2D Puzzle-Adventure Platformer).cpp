#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <cmath>
using namespace std;

enum CellType { EMPTY, PLAYER, EXIT, OBSTACLE, TRAP, PUZZLE, ITEM, ENEMY };
enum ItemType { NONE, FOOD, TOOL, TREASURE };

struct Cell {
    CellType type = EMPTY;
    bool solved = false;
    ItemType item = NONE;
};

struct Player {
    int x = 0, y = 0;
    int stamina = 30;
    int puzzlesSolved = 0, treasures = 0;
    int food = 0, tool = 0;
    bool hintUsed = false, immobilized = false;
    bool trapReady = false;
};

class Jungle {
private:
    vector<vector<Cell>> map;
    Player player;
    int mapSize = 0;
    int level = 1;
    int totalTreasures = 0;
    pair<int, int> exitPos;

    void generate(CellType type, int count) {
        while (count--) {
            int x = rand() % mapSize, y = rand() % mapSize;
            if (map[x][y].type != EMPTY) continue;
            map[x][y].type = type;
            if (type == ITEM) {
                int r = rand() % 3;
                map[x][y].item = (r == 0) ? FOOD : (r == 1 ? TOOL : TREASURE);
                if (map[x][y].item == TREASURE) totalTreasures++;
            }
            if (type == EXIT) exitPos = {x, y};
        }
    }

    bool solvePuzzle() {
        int a = rand() % 10 + 1, b = rand() % 10 + 1;
        cout << "🧠 Puzzle: What is " << a << " + " << b << "? ";
        int ans; cin >> ans;
        return ans == a + b;
    }

    bool showHint() {
        if (player.hintUsed) {
            cout << "💡 You've already used your hint this level.\n";
            return false;
        }
        player.hintUsed = true;
        cout << "🧭 Hint: Exit is ";
        if (exitPos.first < player.x) cout << "north ";
        else if (exitPos.first > player.x) cout << "south ";
        if (exitPos.second < player.y) cout << "west";
        else if (exitPos.second > player.y) cout << "east";
        cout << ".\n";
        return false;
    }

public:
    void initLevel() {
        mapSize = 5 + level * 2;
        map.assign(mapSize, vector<Cell>(mapSize));
        player.stamina = 25 + level * 5;
        player.puzzlesSolved = 0;
        player.treasures = 0;
        player.food = player.tool = 0;
        player.hintUsed = player.immobilized = player.trapReady = false;
        totalTreasures = 2 + level;

        generate(EXIT, 1);
        generate(OBSTACLE, mapSize + 5);
        generate(TRAP, 3 + level);
        generate(PUZZLE, 3 + level);
        generate(ITEM, 3 + level);
        generate(ENEMY, level);

        while (true) {
            int x = rand() % mapSize, y = rand() % mapSize;
            if (map[x][y].type == EMPTY) {
                player.x = x; player.y = y;
                map[x][y].type = PLAYER; break;
            }
        }

        cout << "\n🗺️ Entering LEVEL " << level << " | Map " << mapSize << "×" << mapSize << "\n";
    }

    void drawMap() {
        int vision = 2 + level / 2;
        for (int i = 0; i < mapSize; ++i) {
            for (int j = 0; j < mapSize; ++j) {
                if (abs(player.x - i) + abs(player.y - j) <= vision) {
                    if (i == player.x && j == player.y) cout << "P ";
                    else switch (map[i][j].type) {
                        case OBSTACLE: cout << "# "; break;
                        case TRAP: cout << "^ "; break;
                        case PUZZLE: cout << "? "; break;
                        case ITEM: cout << "$ "; break;
                        case ENEMY: cout << "A "; break;
                        case EXIT: cout << "E "; break;
                        default: cout << ". ";
                    }
                } else cout << "* ";
            }
            cout << '\n';
        }
        cout << "⚡ " << player.stamina << " | 🧠 " << player.puzzlesSolved
             << " | 💰 " << player.treasures << "/" << totalTreasures
             << " | 🍎 " << player.food << " | 🔧 " << player.tool
             << " | Hint: " << (player.hintUsed ? "Used" : "Available") << "\n";
        if (player.immobilized) cout << "🚩 You're immobilized this turn.\n";
    }

    bool handleMove(char cmd) {
        // Weather loss
        if (rand() % 8 == 0) {
            player.stamina--;
            cout << "⛈️  It's raining! You slipped and lost 1 stamina.\n";
        }

        if (player.immobilized) {
            player.immobilized = false;
            cout << "🪢 You're immobilized! Skipping turn...\n";
            return false;
        }

        if (cmd == 'f') {
            if (player.food > 0) {
                player.food--; player.stamina += 5;
                cout << "🍎 Restored 5 stamina.\n";
            } else cout << "🚫 No food left.\n";
            return false;
        }
        if (cmd == 't') {
            if (player.tool > 0) {
                player.tool--; player.trapReady = true;
                cout << "🔧 Tool ready! Next trap won't hurt.\n";
            } else cout << "🚫 No tool available.\n";
            return false;
        }
        if (cmd == 'h') return showHint();

        int dx = 0, dy = 0;
        if (cmd == 'w') dx = -1;
        else if (cmd == 's') dx = 1;
        else if (cmd == 'a') dy = -1;
        else if (cmd == 'd') dy = 1;
        else {
            cout << "❓ Unknown command.\n";
            return false;
        }

        int nx = player.x + dx, ny = player.y + dy;
        if (nx < 0 || ny < 0 || nx >= mapSize || ny >= mapSize) {
            cout << "🧱 Can't move outside the map.\n";
            return false;
        }
        if (map[nx][ny].type == OBSTACLE) {
            cout << "🌳 A tree blocks your path.\n";
            return false;
        }

        map[player.x][player.y].type = EMPTY;
        player.x = nx; player.y = ny;
        player.stamina--;

        // Cell logic
        Cell& cell = map[nx][ny];

        // Exit
        if (cell.type == EXIT) {
            cout << "\n🎉 You escaped Level " << level << "!\n";
            level++;
            return true;
        }

        // Trap
        if (cell.type == TRAP) {
            if (player.trapReady) {
                cout << "🔧 Trap avoided thanks to your tool.\n";
                player.trapReady = false;
            } else {
                if (rand() % 2 == 0) {
                    cout << "💢 Trap immobilized you!\n";
                    player.immobilized = true;
                } else {
                    cout << "💥 Trap hit! -3 stamina.\n";
                    player.stamina -= 3;
                }
            }
        }

        // Puzzle
        if (cell.type == PUZZLE && !cell.solved) {
            if (solvePuzzle()) {
                cout << "✅ Puzzle solved!\n";
                player.puzzlesSolved++; cell.solved = true;
            } else {
                cout << "❌ Wrong! -2 stamina.\n";
                player.stamina -= 2;
            }
        }

        // Item
        if (cell.type == ITEM) {
            if (cell.item == FOOD) { player.food++; cout << "🍎 Found food!\n"; }
            else if (cell.item == TOOL) { player.tool++; cout << "🔧 Found a tool!\n"; }
            else if (cell.item == TREASURE) { player.treasures++; cout << "💰 Found treasure!\n"; }
            cell.item = NONE;
        }

        // Enemy
        if (cell.type == ENEMY) {
            char choice;
            cout << "🐯 Animal encounter! Run (r) or solve puzzle (p)? ";
            cin >> choice;
            if (choice == 'r') {
                cout << "🏃 You ran! -3 stamina.\n";
                player.stamina -= 3;
            } else {
                if (solvePuzzle()) {
                    cout << "👻 You scared it off!\n";
                } else {
                    cout << "🐾 It attacked! -4 stamina.\n";
                    player.stamina -= 4;
                }
            }
        }

        if (player.stamina <= 0) {
            cout << "\n☠️  You collapsed from exhaustion...\n";
            return true;
        }

        map[player.x][player.y].type = PLAYER;
        return false;
    }

    bool play() {
        initLevel();
        while (true) {
            drawMap();
            cout << "(w/a/s/d: move | f: food | t: tool | h: hint): ";
            char input;
            cin >> input;
            if (handleMove(input)) {
                return player.stamina > 0 && level <= 5;
            }
        }
    }

    int score() const {
        return player.stamina * 10 + player.puzzlesSolved * 20 + player.treasures * 25;
    }

    void achievements() const {
        cout << "\n🏅 Achievements:\n";
        if (player.puzzlesSolved >= 4)     cout << "🧠 Puzzle Master\n";
        if (player.treasures == totalTreasures) cout << "💎 Treasure Hunter\n";
        if (!player.hintUsed)             cout << "🔍 No-Hint Explorer\n";
        if (player.stamina >= 40)         cout << "💪 Endurance Hero\n";
        cout << "-----------------------\n";
    }

    int currentLevel() const { return level; }
};

// Leaderboard
void saveScore(const string& name, int score) {
    ofstream file("leaderboard.txt", ios::app);
    file << name << " " << score << "\n";
}

void printLeaderboard() {
    ifstream file("leaderboard.txt");
    vector<pair<int, string>> scores;
    string name; int score;
    while (file >> name >> score) {
        scores.emplace_back(score, name);
    }
    sort(scores.rbegin(), scores.rend());
    cout << "\n🏆 Leaderboard (Top 5):\n";
    for (int i = 0; i < min(5, (int)scores.size()); ++i) {
        cout << i + 1 << ". " << scores[i].second << " - " << scores[i].first << "\n";
    }
}

int main() {
    srand(time(0));
    string name;
    cout << "🌍 LOST IN THE JUNGLE 🛤️\n";
    cout << "Enter your name, explorer: ";
    cin >> name;

    Jungle jungle;
    while (jungle.play()) {
        cout << "\n🌟 Advancing to next level...\n";
    }

    int finalScore = jungle.score();
    cout << "\n🎯 Final Score: " << finalScore << "\n";
    jungle.achievements();

    saveScore(name, finalScore);
    printLeaderboard();

    cout << "\n👋 Thanks for playing, " << name << "!\n";
    return 0;
}