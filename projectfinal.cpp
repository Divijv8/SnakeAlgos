#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>
#include <functional>
#include <numeric>
#include <iomanip>
#include <sstream>

// Game Constants
const int GRID_SIZE = 20;
const int CELL_SIZE = 30;
const int WINDOW_WIDTH = GRID_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_SIZE * CELL_SIZE;
const float GAME_SPEED = 0.1f; // Seconds per move
const int TOTAL_TURNS = 10;    // Total number of turns for competition

// Position class to represent grid coordinates
struct Position {
    int x, y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Custom hash function for Position to use in unordered_map
namespace std {
    template <>
    struct hash<Position> {
        size_t operator()(const Position& pos) const {
            return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1);
        }
    };
}

// Direction enum for snake movement
enum Direction {
    UP, RIGHT, DOWN, LEFT
};

// Node structure for pathfinding algorithms
struct Node {
    Position pos;
    float g_cost;    // Cost from start to this node
    float h_cost;    // Heuristic cost to goal
    float f_cost;    // g_cost + h_cost
    Position parent;

    Node(Position pos, float g = 0, float h = 0) :
        pos(pos), g_cost(g), h_cost(h), f_cost(g + h) {
    }

    // Comparison operator for priority queue
    bool operator>(const Node& other) const {
        return f_cost > other.f_cost ||
            (f_cost == other.f_cost && h_cost > other.h_cost);
    }
};

// Performance metrics structure for each turn
struct TurnMetrics {
    int nodesExplored;
    float computationTime;

    TurnMetrics(int nodes = 0, float time = 0) :
        nodesExplored(nodes), computationTime(time) {
    }
};

// Snake class
class Snake {
public:
    std::vector<Position> body;
    Direction direction;
    sf::Color color;
    std::string algorithm;
    int score;
    int nodesExplored;
    float computationTime;
    std::vector<TurnMetrics> turnMetrics;  // Track metrics for each turn

    Snake(Position startPos, Direction dir, sf::Color col, std::string algo) :
        direction(dir), color(col), algorithm(algo), score(0),
        nodesExplored(0), computationTime(0) {
        body.push_back(startPos);
    }

    Position getHead() const {
        return body.front();
    }

    bool checkCollision(const Position& pos) const {
        for (const auto& segment : body) {
            if (segment == pos) return true;
        }
        return false;
    }

    void move(const Position& nextPos) {
        body.insert(body.begin(), nextPos);
        body.pop_back();
    }

    void grow(const Position& nextPos) {
        body.insert(body.begin(), nextPos);
        score++;
    }

    bool selfCollision() const {
        Position head = getHead();
        for (size_t i = 1; i < body.size(); i++) {
            if (head == body[i]) return true;
        }
        return false;
    }

    // Store metrics for current turn
    void recordTurnMetrics() {
        turnMetrics.push_back(TurnMetrics(nodesExplored, computationTime));
    }

    // Calculate average metrics across turns
    TurnMetrics getAverageMetrics() const {
        if (turnMetrics.empty()) return TurnMetrics();

        int totalNodes = 0;
        float totalTime = 0.0f;

        for (const auto& metrics : turnMetrics) {
            totalNodes += metrics.nodesExplored;
            totalTime += metrics.computationTime;
        }

        return TurnMetrics(
            totalNodes / turnMetrics.size(),
            totalTime / turnMetrics.size()
        );
    }
};

// Game class
class Game {
private:
    sf::RenderWindow window;
    Snake snake1;
    Snake snake2;
    Position food;
    sf::Font font;
    sf::Text scoreText;
    sf::Text statsText;
    sf::Text turnText;
    sf::Text avgMetricsText;
    sf::Clock gameClock;
    float elapsedTime;
    bool gameOver;
    std::mt19937 rng;
    int currentTurn;

    // Get valid neighbors for a position
    std::vector<Position> getNeighbors(const Position& pos) {
        std::vector<Position> neighbors;
        std::vector<Position> directions = {
            {0, -1}, {1, 0}, {0, 1}, {-1, 0}  // UP, RIGHT, DOWN, LEFT
        };

        for (const auto& dir : directions) {
            Position newPos(pos.x + dir.x, pos.y + dir.y);

            // Check bounds
            if (newPos.x >= 0 && newPos.x < GRID_SIZE &&
                newPos.y >= 0 && newPos.y < GRID_SIZE) {
                neighbors.push_back(newPos);
            }
        }

        return neighbors;
    }

    // Check if position is valid (in bounds and not colliding with snakes)
    // Modified to ignore snake body collisions for pathfinding, we only check bounds
    bool isValid(const Position& pos, const Snake& currentSnake, const Snake& otherSnake) {
        // Check bounds
        if (pos.x < 0 || pos.x >= GRID_SIZE || pos.y < 0 || pos.y >= GRID_SIZE) {
            return false;
        }

        // No longer checking for collision with snakes
        return true;
    }

    // Calculate Manhattan distance between two positions
    float manhattanDistance(const Position& a, const Position& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    // A* pathfinding algorithm
    std::vector<Position> findPathAStar(Snake& snake, const Position& goal, Snake& otherSnake) {
        auto startTime = std::chrono::high_resolution_clock::now();

        Position start = snake.getHead();
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_map<Position, float> gScore;
        std::unordered_map<Position, Position> cameFrom;
        std::unordered_set<Position> closedSet;

        openSet.push(Node(start, 0, manhattanDistance(start, goal)));
        gScore[start] = 0;

        int nodesExplored = 0;

        while (!openSet.empty()) {
            Node current = openSet.top();
            openSet.pop();
            nodesExplored++;

            if (current.pos == goal) {
                // Reconstruct path
                std::vector<Position> path;
                Position currentPos = goal;

                while (currentPos != start) {
                    path.push_back(currentPos);
                    currentPos = cameFrom[currentPos];
                }

                std::reverse(path.begin(), path.end());

                auto endTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = endTime - startTime;
                snake.computationTime = duration.count() * 1000; // Convert to milliseconds
                snake.nodesExplored = nodesExplored;

                return path;
            }

            closedSet.insert(current.pos);

            for (const auto& neighbor : getNeighbors(current.pos)) {
                if (closedSet.find(neighbor) != closedSet.end()) {
                    continue;
                }

                if (!isValid(neighbor, snake, otherSnake)) {
                    continue;
                }

                float tentative_gScore = gScore[current.pos] + 1;

                if (gScore.find(neighbor) == gScore.end() || tentative_gScore < gScore[neighbor]) {
                    cameFrom[neighbor] = current.pos;
                    gScore[neighbor] = tentative_gScore;
                    float hScore = manhattanDistance(neighbor, goal);
                    openSet.push(Node(neighbor, tentative_gScore, hScore));
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = endTime - startTime;
        snake.computationTime = duration.count() * 1000; // Convert to milliseconds
        snake.nodesExplored = nodesExplored;

        // No path found
        return {};
    }

    // Dijkstra's algorithm
    std::vector<Position> findPathDijkstra(Snake& snake, const Position& goal, Snake& otherSnake) {
        auto startTime = std::chrono::high_resolution_clock::now();

        Position start = snake.getHead();
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_map<Position, float> gScore;
        std::unordered_map<Position, Position> cameFrom;
        std::unordered_set<Position> closedSet;

        openSet.push(Node(start, 0, 0)); // No heuristic for Dijkstra
        gScore[start] = 0;

        int nodesExplored = 0;

        while (!openSet.empty()) {
            Node current = openSet.top();
            openSet.pop();
            nodesExplored++;

            if (current.pos == goal) {
                // Reconstruct path
                std::vector<Position> path;
                Position currentPos = goal;

                while (currentPos != start) {
                    path.push_back(currentPos);
                    currentPos = cameFrom[currentPos];
                }

                std::reverse(path.begin(), path.end());

                auto endTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = endTime - startTime;
                snake.computationTime = duration.count() * 1000; // Convert to milliseconds
                snake.nodesExplored = nodesExplored;

                return path;
            }

            closedSet.insert(current.pos);

            for (const auto& neighbor : getNeighbors(current.pos)) {
                if (closedSet.find(neighbor) != closedSet.end()) {
                    continue;
                }

                if (!isValid(neighbor, snake, otherSnake)) {
                    continue;
                }

                float tentative_gScore = gScore[current.pos] + 1;

                if (gScore.find(neighbor) == gScore.end() || tentative_gScore < gScore[neighbor]) {
                    cameFrom[neighbor] = current.pos;
                    gScore[neighbor] = tentative_gScore;
                    openSet.push(Node(neighbor, tentative_gScore, 0)); // h_cost is 0 for Dijkstra
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = endTime - startTime;
        snake.computationTime = duration.count() * 1000; // Convert to milliseconds
        snake.nodesExplored = nodesExplored;

        // No path found
        return {};
    }

    // Get next position based on direction
    Position getNextPosition(const Position& pos, Direction dir) {
        switch (dir) {
        case UP:    return Position(pos.x, pos.y - 1);
        case RIGHT: return Position(pos.x + 1, pos.y);
        case DOWN:  return Position(pos.x, pos.y + 1);
        case LEFT:  return Position(pos.x - 1, pos.y);
        default:    return pos;
        }
    }

    // Calculate direction from current position to next position
    Direction calculateDirection(const Position& current, const Position& next) {
        if (next.x > current.x) return RIGHT;
        if (next.x < current.x) return LEFT;
        if (next.y > current.y) return DOWN;
        if (next.y < current.y) return UP;
        return RIGHT; // Default
    }

    // Generate a random position that is not occupied by either snake
    Position generateRandomPosition() {
        std::vector<Position> candidates;
        Position head1 = snake1.getHead();
        Position head2 = snake2.getHead();

        // Check all grid positions for potential food locations
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int y = 0; y < GRID_SIZE; y++) {
                Position pos(x, y);

                // Skip if position is occupied by either snake
                if (snake1.checkCollision(pos) || snake2.checkCollision(pos))
                    continue;

                // Calculate Manhattan distances from both snake heads
                int dist1 = std::abs(pos.x - head1.x) + std::abs(pos.y - head1.y);
                int dist2 = std::abs(pos.x - head2.x) + std::abs(pos.y - head2.y);

                // Check if distances are equal (or very close) and at least 10 units away
                if (std::abs(dist1 - dist2) <= 1 && dist1 >= 10 && dist2 >= 10) {
                    candidates.push_back(pos);
                }
            }
        }

        // If no suitable positions found with equal distances and minimum distance,
        // try with just equal distances
        if (candidates.empty()) {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    Position pos(x, y);

                    if (snake1.checkCollision(pos) || snake2.checkCollision(pos))
                        continue;

                    int dist1 = std::abs(pos.x - head1.x) + std::abs(pos.y - head1.y);
                    int dist2 = std::abs(pos.x - head2.x) + std::abs(pos.y - head2.y);

                    if (std::abs(dist1 - dist2) <= 1) {
                        candidates.push_back(pos);
                    }
                }
            }
        }

        // If still no suitable positions, fall back to any unoccupied position
        if (candidates.empty()) {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    Position pos(x, y);

                    if (!snake1.checkCollision(pos) && !snake2.checkCollision(pos)) {
                        candidates.push_back(pos);
                    }
                }
            }
        }

        // Select a random position from the candidates
        if (!candidates.empty()) {
            std::uniform_int_distribution<int> dist(0, candidates.size() - 1);
            return candidates[dist(rng)];
        }

        // Last resort fallback (shouldn't happen in normal gameplay)
        std::uniform_int_distribution<int> dist(0, GRID_SIZE - 1);
        Position pos;
        do {
            pos.x = dist(rng);
            pos.y = dist(rng);
        } while (snake1.checkCollision(pos) || snake2.checkCollision(pos));

        return pos;
    }

    // Start a new turn
    void startNewTurn() {
        // Generate new food position
        food = generateRandomPosition();

        // Increment turn counter
        currentTurn++;

        // Display turn information
        updateText();
    }

    // Check if both snakes have reached the food
    bool isTurnComplete() {
        return (snake1.body.front() == food && snake2.body.front() == food);
    }

    // Format floating point number as string with specified precision
    std::string formatFloat(float value, int precision = 2) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    }

    // Update text display
    void updateText() {
        scoreText.setString("A* (Red): " + std::to_string(snake1.score) +
            " | Dijkstra (Blue): " + std::to_string(snake2.score));

        statsText.setString(
            "A*: " + std::to_string(snake1.nodesExplored) + " nodes, " +
            formatFloat(snake1.computationTime) + " ms\n" +
            "Dijkstra: " + std::to_string(snake2.nodesExplored) + " nodes, " +
            formatFloat(snake2.computationTime) + " ms"
        );

        turnText.setString("Turn: " + std::to_string(currentTurn) + "/" + std::to_string(TOTAL_TURNS));

        // Update average metrics text
        TurnMetrics avg1 = snake1.getAverageMetrics();
        TurnMetrics avg2 = snake2.getAverageMetrics();

        avgMetricsText.setString(
            "Avg A*: " + std::to_string(avg1.nodesExplored) + " nodes, " +
            formatFloat(avg1.computationTime) + " ms\n" +
            "Avg Dijkstra: " + std::to_string(avg2.nodesExplored) + " nodes, " +
            formatFloat(avg2.computationTime) + " ms"
        );
    }

public:
    Game() :
        window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + 150), "Snake AI Competition"),
        snake1(Position(5, 10), RIGHT, sf::Color::Red, "A*"),
        snake2(Position(15, 10), LEFT, sf::Color::Blue, "Dijkstra"),
        elapsedTime(0),
        gameOver(false),
        rng(std::random_device{}()),
        currentTurn(0)
    {
        window.setFramerateLimit(60);

        // Initialize first turn
        startNewTurn();

        // Load font
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
            font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"); // Try Windows path
        }

        // Set up text
        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, WINDOW_HEIGHT + 10);

        statsText.setFont(font);
        statsText.setCharacterSize(14);
        statsText.setFillColor(sf::Color::White);
        statsText.setPosition(10, WINDOW_HEIGHT + 50);

        turnText.setFont(font);
        turnText.setCharacterSize(16);
        turnText.setFillColor(sf::Color::Yellow);
        turnText.setPosition(10, WINDOW_HEIGHT + 90);

        avgMetricsText.setFont(font);
        avgMetricsText.setCharacterSize(14);
        avgMetricsText.setFillColor(sf::Color::Green);
        avgMetricsText.setPosition(10, WINDOW_HEIGHT + 120);
    }

    void run() {
        std::vector<Position> path1, path2;
        bool snake1ReachedFood = false;
        bool snake2ReachedFood = false;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        // Reset game
                        snake1 = Snake(Position(5, 10), RIGHT, sf::Color::Red, "A*");
                        snake2 = Snake(Position(15, 10), LEFT, sf::Color::Blue, "Dijkstra");
                        currentTurn = 0;
                        startNewTurn();
                        gameOver = false;
                        snake1ReachedFood = false;
                        snake2ReachedFood = false;
                    }
                    else if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                }
            }

            if (!gameOver) {
                // Update game state based on elapsed time
                float deltaTime = gameClock.restart().asSeconds();
                elapsedTime += deltaTime;

                if (elapsedTime >= GAME_SPEED) {
                    elapsedTime = 0;

                    // Find paths for both snakes if needed
                    if (path1.empty() && !snake1ReachedFood) {
                        path1 = findPathAStar(snake1, food, snake2);
                    }

                    if (path2.empty() && !snake2ReachedFood) {
                        path2 = findPathDijkstra(snake2, food, snake1);
                    }

                    // Move snake1 along its path if not already at food
                    if (!path1.empty() && !snake1ReachedFood) {
                        Position nextPos = path1[0];
                        path1.erase(path1.begin());
                        snake1.direction = calculateDirection(snake1.getHead(), nextPos);

                        // Check if snake reached food
                        if (nextPos == food) {
                            snake1.grow(nextPos);
                            snake1ReachedFood = true;
                            snake1.recordTurnMetrics(); // Record metrics for this turn
                        }
                        else {
                            snake1.move(nextPos);
                        }
                    }

                    // Move snake2 along its path if not already at food
                    if (!path2.empty() && !snake2ReachedFood) {
                        Position nextPos = path2[0];
                        path2.erase(path2.begin());
                        snake2.direction = calculateDirection(snake2.getHead(), nextPos);

                        // Check if snake reached food
                        if (nextPos == food) {
                            snake2.grow(nextPos);
                            snake2ReachedFood = true;
                            snake2.recordTurnMetrics(); // Record metrics for this turn
                        }
                        else {
                            snake2.move(nextPos);
                        }
                    }

                    // If both snakes have reached food, start a new turn
                    if (snake1ReachedFood && snake2ReachedFood) {
                        // Check if we've completed all turns
                        if (currentTurn >= TOTAL_TURNS) {
                            gameOver = true;
                        }
                        else {
                            startNewTurn();
                            path1.clear();
                            path2.clear();
                            snake1ReachedFood = false;
                            snake2ReachedFood = false;
                        }
                    }

                    updateText();
                }
            }

            // Draw everything
            window.clear(sf::Color(50, 50, 50));

            // Draw grid
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = 0; y < GRID_SIZE; y++) {
                    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                    cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                    cell.setOutlineThickness(1);
                    cell.setOutlineColor(sf::Color(70, 70, 70));
                    cell.setFillColor(sf::Color(30, 30, 30));
                    window.draw(cell);
                }
            }

            // Draw paths (if debug mode)
            sf::Color path1Color(255, 100, 100, 80);
            for (const auto& pos : path1) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(pos.x * CELL_SIZE, pos.y * CELL_SIZE);
                cell.setFillColor(path1Color);
                window.draw(cell);
            }

            sf::Color path2Color(100, 100, 255, 80);
            for (const auto& pos : path2) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(pos.x * CELL_SIZE, pos.y * CELL_SIZE);
                cell.setFillColor(path2Color);
                window.draw(cell);
            }

            // Draw food
            sf::CircleShape foodShape(CELL_SIZE / 2);
            foodShape.setPosition(food.x * CELL_SIZE + CELL_SIZE / 4, food.y * CELL_SIZE + CELL_SIZE / 4);
            foodShape.setFillColor(sf::Color::Green);
            window.draw(foodShape);

            // Draw snake1
            for (const auto& segment : snake1.body) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(segment.x * CELL_SIZE, segment.y * CELL_SIZE);
                cell.setFillColor(snake1.color);
                window.draw(cell);
            }

            // Draw snake2
            for (const auto& segment : snake2.body) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(segment.x * CELL_SIZE, segment.y * CELL_SIZE);
                cell.setFillColor(snake2.color);
                window.draw(cell);
            }

            // Draw text
            window.draw(scoreText);
            window.draw(statsText);
            window.draw(turnText);
            window.draw(avgMetricsText);

            // Draw game over text if applicable
            if (gameOver) {
                sf::Text gameOverText;
                gameOverText.setFont(font);
                gameOverText.setCharacterSize(40);
                gameOverText.setFillColor(sf::Color::White);

                // Determine winner based on score
                std::string resultText;
                if (snake1.score > snake2.score) {
                    resultText = "A* wins!";
                }
                else if (snake2.score > snake1.score) {
                    resultText = "Dijkstra wins!";
                }
                else {
                    resultText = "It's a tie!";
                }

                gameOverText.setString("Game Complete!\n" + resultText + "\nPress R to restart");

                // Center the text
                sf::FloatRect textRect = gameOverText.getLocalBounds();
                gameOverText.setOrigin(textRect.left + textRect.width / 2.0f,
                    textRect.top + textRect.height / 2.0f);
                gameOverText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

                window.draw(gameOverText);

                // Display final statistics
                sf::Text finalStatsText;
                finalStatsText.setFont(font);
                finalStatsText.setCharacterSize(18);
                finalStatsText.setFillColor(sf::Color::Yellow);

                TurnMetrics avg1 = snake1.getAverageMetrics();
                TurnMetrics avg2 = snake2.getAverageMetrics();

                finalStatsText.setString(
                    "Final Stats:/n"
                    "A*: Score=" + std::to_string(snake1.score) + 
                    ", Avg Nodes=" + std::to_string(avg1.nodesExplored) +
                    ", Avg Time=" + formatFloat(avg1.computationTime) + "ms\n" +
                    "Dijkstra: Score=" + std::to_string(snake2.score) +
                    ", Avg Nodes=" + std::to_string(avg2.nodesExplored) +
                    ", Avg Time=" + formatFloat(avg2.computationTime) + "ms"
                );
                
                finalStatsText.setPosition(20, 100);
                window.draw(finalStatsText);
            }

            window.display();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}