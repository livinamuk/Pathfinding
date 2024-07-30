#include "Pathfinding.h"
#include "Input.h"
#include "../BackEnd/BackEnd.h"
#include "../Core/Audio.hpp"
#include "../Core/JSON.hpp"
#include "../Renderer/RendererCommon.h"
#include "../Util.hpp"
#include <algorithm>

namespace Pathfinding {

    int g_mapWidth = 0;
    int g_mapHeight = 0;
    ivec2 g_start;
    ivec2 g_target;
    std::vector<std::vector<bool>> g_map;
    AStar g_AStar;
    bool g_slowMode = true;

    void Init() {
        g_mapWidth = PRESENT_WIDTH / CELL_SIZE;
        g_mapHeight = PRESENT_HEIGHT / CELL_SIZE + 1;
        g_map.resize(g_mapWidth, std::vector<bool>(g_mapHeight, false));
        LoadMap();
    }

    void ResetAStar() {
        g_AStar.ClearData();
    }

    void Update(float deltaTime) {

        if (Input::LeftMouseDown()) {
            SetObstacle(GetMouseCellX(), GetMouseCellY(), true);
            ResetAStar();
        }
        if (Input::RightMouseDown()) {
            SetObstacle(GetMouseCellX(), GetMouseCellY(), false);
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_L)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            LoadMap();
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_S)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            SaveMap();
        }
        if (Input::KeyPressed(HELL_KEY_N)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            ClearMap();
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_D)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            g_slowMode = !g_slowMode;
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_1)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            SetStart(GetMouseCellX(), GetMouseCellY());
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_2)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
            SetTarget(GetMouseCellX(), GetMouseCellY());
            ResetAStar();
        }
        if (Input::KeyPressed(HELL_KEY_SPACE)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
        }
        if (Input::KeyDown(HELL_KEY_SPACE) && !g_AStar.GridPathFound()) {
            Audio::PlayAudio("UI_Select.wav", 0.5);
            if (!g_AStar.SearchInitilized()) {
                g_AStar.InitSearch(g_map, g_start.x, g_start.y, g_target.x, g_target.y);
            }
            if (!g_AStar.GridPathFound()) {
                g_AStar.FindPath();
            }
        }
        if (Input::KeyPressed(HELL_KEY_W) || Input::KeyPressed(HELL_KEY_A)) {
            Audio::PlayAudio("SELECT.wav", 1.0);
        }
        if (Input::KeyDown(HELL_KEY_W) && !g_AStar.SmoothPathFound() || Input::KeyPressed(HELL_KEY_A)) {
            Audio::PlayAudio("UI_Select.wav", 0.5);
            if (!g_AStar.SmoothPathFound()) {
                g_AStar.FindSmoothPath();
            }
        }
    }

    void ClearMap() {
        for (int y = 0; y < GetMapWidth(); ++y) {
            for (int x = 0; x < GetMapHeight(); ++x) {
                g_map[y][x] = false;
            }
        }
        g_start = { 0,0 };
        g_target = { 0,1 };
    }

    void LoadMap() {
        ClearMap();
        std::string fullPath = "res/maps/mappp.txt";
        if (Util::FileExists(fullPath)) {
            std::cout << "Loading map '" << fullPath << "'\n";
            std::ifstream file(fullPath);
            std::stringstream buffer;
            buffer << file.rdbuf();
            if (buffer) {
                nlohmann::json data = nlohmann::json::parse(buffer.str());
                for (const auto& jsonObject : data["map"]) {
                    int x = jsonObject["position"]["x"];
                    int y = jsonObject["position"]["y"];
                    SetObstacle(x, y, true);
                }
                g_start.x = data["start"]["x"];
                g_start.y = data["start"]["y"];
                g_target.x = data["target"]["x"];
                g_target.y = data["target"]["y"];
            }
        }
    }

    void SaveMap() {
        JSONObject saveFile;
        nlohmann::json data;
        nlohmann::json jsonMap = nlohmann::json::array();
        for (int x = 0; x < GetMapWidth(); x++) {
            for (int y = 0; y < GetMapHeight(); y++) {
                if (IsObstacle(x, y)) {
                    nlohmann::json jsonObject;
                    jsonObject["position"] = { {"x", x}, {"y", y} };
                    jsonMap.push_back(jsonObject);
                }
            }
        }
        data["map"] = jsonMap;
        data["start"] = { {"x", g_start.x}, {"y", g_start.y} };
        data["target"] = { {"x", g_target.x}, {"y", g_target.y} };
        int indent = 4;
        std::string text = data.dump(indent);
        std::cout << text << "\n\n";
        std::ofstream out("res/maps/mappp.txt");
        out << text;
        out.close();
        std::cout << "Saving map\n";
    }

    void SetStart(int x, int y) {
        if (IsInBounds(x, y)) {
            g_start = { x , y };
        }
    }

    void SetTarget(int x, int y) {
        if (IsInBounds(x, y)) {
            g_target = { x , y };
        }
    }

    bool IsInBounds(int x, int y) {
        return (x >= 0 && y >= 0 && x < g_mapWidth && y < g_mapHeight);
    }

    void SetObstacle(int x, int y, bool value) {
        if (IsInBounds(x, y)) {
            g_map[x][y] = value;
        }
    }

    bool IsObstacle(int x, int y) {
        if (IsInBounds(x, y)) {
            return g_map[x][y];
        }
        else {
            return false;
        }
    }

    int GetMouseX() {
        float scalingRatio = BackEnd::GetCurrentWindowWidth() / PRESENT_WIDTH;
        return Util::MapRange(Input::GetMouseX(), 0, BackEnd::GetCurrentWindowWidth(), 0, PRESENT_WIDTH);
    }

    int GetMouseY() {
        return Util::MapRange(Input::GetMouseY(), 0, BackEnd::GetCurrentWindowHeight(), 0, PRESENT_HEIGHT);
    }

    int GetMouseCellX() {
        return Util::MapRange(Input::GetMouseX(), 0, BackEnd::GetCurrentWindowWidth(), 0, PRESENT_WIDTH) / CELL_SIZE;
    }

    int GetMouseCellY() {
        return Util::MapRange(Input::GetMouseY(), 0, BackEnd::GetCurrentWindowHeight(), 0, PRESENT_HEIGHT) / CELL_SIZE;
    }

    int GetMapWidth() {
        return g_mapWidth;
    }

    int GetMapHeight() {
        return g_mapHeight;
    }

    int GetStartX() {
        return g_start.x;
    }

    int GetStartY() {
        return g_start.y;
    }

    int GetTargetX() {
        return g_target.x;
    }

    int GetTargetY() {
        return g_target.y;
    }

    AStar& GetAStar() {
        return g_AStar;
    }

    bool SlowModeEnabled() {
        return g_slowMode;
    }
}

void AStar::InitSearch(std::vector<std::vector<bool>>& map, int startX, int startY, int destinationX, int destinationY) {
    ClearData();
    m_cells.resize(Pathfinding::GetMapWidth(), std::vector<Cell>(Pathfinding::GetMapHeight()));
    m_openList.AllocateSpace(Pathfinding::GetMapWidth() * Pathfinding::GetMapHeight());
    m_openList.Clear();
    for (int x = 0; x < Pathfinding::GetMapWidth(); x++) {
        for (int y = 0; y < Pathfinding::GetMapHeight(); y++) {
            m_cells[x][y].x = x;
            m_cells[x][y].y = y;
            m_cells[x][y].obstacle = Pathfinding::IsObstacle(x, y);
            m_cells[x][y].g = 99999;
            m_cells[x][y].h = -1;
            m_cells[x][y].f = -1;
            m_cells[x][y].parent = nullptr;
            m_cells[x][y].neighbours.clear();
        }
    }
    m_start = &m_cells[startX][startY];
    m_current = m_start;
    m_destination = &m_cells[destinationX][destinationY];
    m_start->g = 0;
    m_start->GetF(m_destination);
    m_openList.AddItem(m_start);
    m_searchInitilized = true;
}


void AStar::ClearData() {
    m_closedList.clear();
    m_finalPath.clear();
    m_openList.Clear();
    m_intersectionPoints.clear();
    m_gridPathFound = false;
    m_smoothPathFound = false;
    m_searchInitilized = false;
    m_smoothSearchIndex = 0;
}

bool AStar::GridPathFound() {
    return m_gridPathFound;
}

bool AStar::SmoothPathFound() {
    return m_smoothPathFound;
}

bool AStar::SearchInitilized() {
    return m_searchInitilized;
}

void AStar::FindPath() {
    if (m_destination->obstacle) {
        return;
    }
    if (m_openList.IsEmpty()) {
        return;
    }
    if (m_gridPathFound) {
        return;
    }
    //while (!m_openList.IsEmpty()
    {
        m_current = m_openList.RemoveFirst();
        if (IsDestination(m_current)) {
            m_gridPathFound = true;
            BuildFinalPath();
            return;
        }
        AddIfUnique(&m_closedList, m_current);
        FindNeighbours(m_current);
        for (Cell* neighbour : m_current->neighbours) {
            // Calculate G cost. Equal to parent G cost + 10 if orthogonal and + 14 if diagonal
            int new_g = IsOrthogonal(m_current, neighbour) ? m_current->g + ORTHOGONAL_COST : m_current->g + DIAGONAL_COST;

            if (IsInClosedList(neighbour) || m_openList.Contains(neighbour)) {
                // If new G is lower than currently stored value, update it and change parent to the current cell
                if (new_g < neighbour->g) {
                    neighbour->g = new_g;
                    neighbour->f = new_g + neighbour->GetH(m_destination);
                    neighbour->parent = m_current;
                }
            }
            else {
                neighbour->g = new_g;
                neighbour->f = new_g + neighbour->GetH(m_destination);
                neighbour->parent = m_current;

                if (!m_openList.Contains(neighbour)) {
                    m_openList.AddItem(neighbour);
                }
            }
        }
    }
    if (!Pathfinding::SlowModeEnabled()) {
        FindPath();
    }
}

MinHeap& AStar::GetOpenList() {
    return m_openList;
}

std::list<Cell*>& AStar::GetClosedList() {
    return m_closedList;
}

std::vector<Cell*>& AStar::GetPath() {
    return m_finalPath;
}

bool AStar::IsDestination(Cell* cell) {
    return cell == m_destination;
}

void AStar::BuildFinalPath() {
    Cell* cell = m_destination;
    while (cell != m_start) {
        m_finalPath.push_back(cell);
        cell = cell->parent;
    }
    std::reverse(m_finalPath.begin(), m_finalPath.end());

    // Init smooth search
    m_smoothPathFound = false;
    m_smoothSearchIndex = 2;
    m_intersectionPoints.clear();
    glm::vec2 startPoint = glm::vec2(m_start->x + 0.5f, m_start->y + 0.5f);
    glm::vec2 endPoint = glm::vec2(m_destination->x + 0.5f, m_destination->y + 0.5f);
    m_intersectionPoints.push_back(startPoint);
    for (int j = 0; j < m_finalPath.size(); j++) {
        Cell* cell = m_finalPath[j];
        m_intersectionPoints.push_back(glm::vec2(cell->x, cell->y));
    }
    m_intersectionPoints.push_back(endPoint);
}

bool HasLineOfSight(glm::vec2 startPosition, glm::vec2 endPosition) {
    float stepSize = 0.5f;
    glm::vec2 direction = glm::normalize(endPosition - startPosition);
    glm::vec2 testPosition = startPosition;
    float distanceToEnd = glm::distance(testPosition, endPosition);
    float currentDistance = distanceToEnd;
    while (currentDistance > 1) {
        testPosition += direction * stepSize;
        if (Pathfinding::IsObstacle((int)testPosition.x, (int)testPosition.y)) {
            return false;
        }
        currentDistance = glm::distance(testPosition, endPosition);
        // You made it past your target! There is line of sight
        if (currentDistance > distanceToEnd) {
            return true;
        }
    }
    return true;
}

void AStar::FindSmoothPath() {
    if (!m_gridPathFound) {
        return;
    }
    if (m_smoothSearchIndex >= m_intersectionPoints.size()) {
        m_smoothPathFound = true;
    }
    if (m_smoothPathFound) {
        return;
    }
    // Remove points with line of sight
    glm::vec2 currentPosition = m_intersectionPoints[m_smoothSearchIndex];
    glm::vec2 queryPosition = m_intersectionPoints[m_smoothSearchIndex -2];
    glm::vec2 intersectionPointOut;
    if (HasLineOfSight(currentPosition, queryPosition)) {
        m_intersectionPoints.erase(m_intersectionPoints.begin() + m_smoothSearchIndex - 1);
    }
    else {
        m_smoothSearchIndex++;
    }
    if (!Pathfinding::SlowModeEnabled()) {
        FindSmoothPath();
    }
}

void AStar::AddIfUnique(std::list<Cell*>* list, Cell* cell) {
    std::list<Cell*>::iterator it;
    it = std::find(list->begin(), list->end(), cell);
    if (it == list->end()) {
        list->push_front(cell);
    }
}

bool AStar::IsOrthogonal(Cell* cellA, Cell* cellB) {
    return (cellA->x == cellB->x || cellA->y == cellB->y);
}

bool AStar::IsInClosedList(Cell* cell) {
    std::list<Cell*>::iterator it;
    it = std::find(m_closedList.begin(), m_closedList.end(), cell);
    return (it != m_closedList.end());
}

void AStar::FindNeighbours(Cell* cell) {
    if (m_current->neighbours.size() != 0) {
        return;
    }
    int x = cell->x;
    int y = cell->y;
    // North
    if (Pathfinding::IsInBounds(x, y - 1) && !Pathfinding::IsObstacle(x, y - 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x][y - 1]);
    }
    // South
    if (Pathfinding::IsInBounds(x, y + 1) && !Pathfinding::IsObstacle(x, y + 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x][y + 1]);
    }
    // West
    if (Pathfinding::IsInBounds(x - 1, y) && !Pathfinding::IsObstacle(x - 1, y)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x - 1][y]);
    }
    // East
    if (Pathfinding::IsInBounds(x + 1, y) && !Pathfinding::IsObstacle(x + 1, y)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x + 1][y]);
    }
    /*
    // North West
    if (Pathfinding::IsInBounds(x - 1, y - 1) && !Pathfinding::IsObstacle(x - 1, y - 1)) {// && !Pathfinding::IsObstacle(x - 1, y) && !Pathfinding::IsObstacle(x, y - 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x - 1][y - 1]);
    }
    // North East
    if (Pathfinding::IsInBounds(x + 1, y - 1) && !Pathfinding::IsObstacle(x + 1, y - 1)) {// && !Pathfinding::IsObstacle(x + 1, y) && !Pathfinding::IsObstacle(x, y - 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x + 1][y - 1]);
    }
    // South West
    if (Pathfinding::IsInBounds(x - 1, y + 1) && !Pathfinding::IsObstacle(x - 1, y + 1)) {// && !Pathfinding::IsObstacle(x - 1, y) && !Pathfinding::IsObstacle(x, y + 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x - 1][y + 1]);
    }
    // South East
    if (Pathfinding::IsInBounds(x + 1, y + 1) && !Pathfinding::IsObstacle(x + 1, y + 1)) {// && !Pathfinding::IsObstacle(x + 1, y) && !Pathfinding::IsObstacle(x, y + 1)) {
        m_cells[x][y].neighbours.push_back(&m_cells[x + 1][y + 1]);
    }*/
}

float Cell::GetF(Cell* destination) {
    if (f == -1) {
        f = g + GetH(destination);
    }
    return f;
}

float Cell::GetH(Cell* destination) {
    if (h == -1) {
        int dstX = std::abs(x - destination->x);
        int dstY = std::abs(y - destination->y);
        if (dstX > dstY) {
            h = DIAGONAL_COST * dstY + ORTHOGONAL_COST * (dstX - dstY);
        }
        else {
            h = DIAGONAL_COST * dstX + ORTHOGONAL_COST * (dstY - dstX);
        }
    }
    return h;
}

void MinHeap::AllocateSpace(int size) {
    items = std::vector<Cell*>(size);
}

void MinHeap::AddItem(Cell* item) {
    item->heapIndex = currentItemCount;
    items[currentItemCount] = item;
    SortUp(item);
    currentItemCount++;
}

void MinHeap::Update(Cell* cell) {
    SortUp(cell);
}

bool MinHeap::Contains(Cell* cell) {
    return (items[cell->heapIndex] == cell);
}

bool MinHeap::IsEmpty() {
    return (currentItemCount == 0);
}

int MinHeap::Size() {
    return currentItemCount;
}

void MinHeap::Clear() {
    currentItemCount = 0;
}

void MinHeap::SortUp(Cell* item) {
    int parentIndex = (item->heapIndex - 1) / 2;
    while (true) {
        Cell* parent = items[parentIndex];
        if (parent->f > item->f) {
            Swap(item, parent);
        }
        else {
            break;
        }
        parentIndex = (item->heapIndex - 1) / 2;
    }
}

void MinHeap::Swap(Cell* cellA, Cell* cellB) {
    items[cellA->heapIndex] = cellB;
    items[cellB->heapIndex] = cellA;
    int itemAIndex = cellA->heapIndex;
    cellA->heapIndex = cellB->heapIndex;
    cellB->heapIndex = itemAIndex;
}

Cell* MinHeap::RemoveFirst() {
    Cell* firstItem = items[0];
    currentItemCount--;
    items[0] = items[currentItemCount];
    items[0]->heapIndex = 0;
    SortDown(items[0]);
    return firstItem;
}

void MinHeap::SortDown(Cell* cell) {
    while (true) {
        int childIndexLeft = cell->heapIndex * 2 + 1;
        int childIndexRight = cell->heapIndex * 2 + 2;
        int swapIndex = 0;
        if (childIndexLeft < currentItemCount) {
            swapIndex = childIndexLeft;
            if (childIndexRight < currentItemCount) {
                if (items[childIndexLeft]->f > items[childIndexRight]->f) {
                    swapIndex = childIndexRight;
                }
            }
            if (cell->f > items[swapIndex]->f)
                Swap(cell, items[swapIndex]);
            else {
                return;
            }
        }
        else {
            return;
        }
    }
}