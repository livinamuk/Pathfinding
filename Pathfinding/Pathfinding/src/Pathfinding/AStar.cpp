#include "AStar.h"
#include "Pathfinding.h"

void AStar::InitSearch(int startX, int startY, int destinationX, int destinationY) {
    ClearData();

    m_cellsFlat.resize(Pathfinding::GetCellCount());

    int mapWidth = Pathfinding::GetMapWidth();
    int mapHeight = Pathfinding::GetMapHeight();

    m_openList.AllocateSpace(mapWidth * mapHeight);
    m_openList.Clear();

    for (int x = 0; x < mapWidth; x++) {
        for (int y = 0; y < mapHeight; y++) {
            int idx = Pathfinding::Index1D(x, y);
            m_cellsFlat[idx].x = x;
            m_cellsFlat[idx].y = y;
            m_cellsFlat[idx].obstacle = Pathfinding::IsObstacle(x, y);
            m_cellsFlat[idx].g = 99999;
            m_cellsFlat[idx].h = -1;
            m_cellsFlat[idx].f = -1;
            m_cellsFlat[idx].parent = nullptr;
            m_cellsFlat[idx].neighbours.clear();
        }
    }

    int idxStart = Pathfinding::Index1D(startX, startY);
    int idxDestination = Pathfinding::Index1D(destinationX, destinationY);

    m_start = &m_cellsFlat[idxStart];
    m_destination = &m_cellsFlat[idxDestination];
    m_current = m_start;
    m_start->g = 0;
    m_start->GetF(m_destination);
    m_openList.AddItem(m_start);
    m_searchInitilized = true;

    // initialize closed list to false
    m_closedFlags.assign(mapWidth * mapHeight, false);

    // Cache all neighbours (previously you were doing it for every cell on the open list
    for (int x = 0; x < Pathfinding::GetMapWidth(); x++) {
        for (int y = 0; y < Pathfinding::GetMapHeight(); y++) {
            Cell tempCell;
            tempCell.x = x;
            tempCell.y = y;
            FindNeighbours(&tempCell);
        }
    }
}

void AStar::ClearData() {
    m_closedFlags.clear();
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

    //while (!m_openList.IsEmpty())
    {
        m_current = m_openList.RemoveFirst();
        if (IsDestination(m_current)) {
            m_gridPathFound = true;
            BuildFinalPath();
            //PrintPath();
            return;
        }

        // Add current cell to closed list
        int idxCurrent = Pathfinding::Index1D(m_current->x, m_current->y);
        m_closedFlags[idxCurrent] = true;

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

void AStar::PrintPath() {

    for (int y = 0; y < Pathfinding::GetMapHeight(); ++y) {
        for (int x = 0; x < Pathfinding::GetMapWidth(); ++x) {

            bool inPath = false;
            for (Cell* cell : m_finalPath) {
                if (cell->x == x && cell->y == y) {
                    inPath = true;
                    break;
                }
            }

            int idx = Pathfinding::Index1D(x, y);

            if (x == m_start->x && y == m_start->y) {
                std::cout << "A";
            }
            else if (x == m_destination->x && y == m_destination->y) {
                std::cout << "B";
            }
            else if (inPath) {
                std::cout << ".";
            }
            else if (Pathfinding::IsObstacle(x, y)) {
                std::cout << char(219);
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }
}

MinHeap& AStar::GetOpenList() {
    return m_openList;
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
    glm::vec2 queryPosition = m_intersectionPoints[m_smoothSearchIndex - 2];
    glm::vec2 intersectionPointOut;
    if (Pathfinding::HasLineOfSight(currentPosition, queryPosition)) {
        m_intersectionPoints.erase(m_intersectionPoints.begin() + m_smoothSearchIndex - 1);
    }
    else {
        m_smoothSearchIndex++;
    }
    if (!Pathfinding::SlowModeEnabled()) {
        FindSmoothPath();
    }
}

bool AStar::IsOrthogonal(Cell* cellA, Cell* cellB) {
    return (cellA->x == cellB->x || cellA->y == cellB->y);
}

bool AStar::IsInClosedList(Cell* cell) {
    int idx = Pathfinding::Index1D(cell->x, cell->y);
    return m_closedFlags[idx];
}

void AStar::FindNeighbours(Cell* cell) {
    int x = cell->x;
    int y = cell->y;

    int idx = Pathfinding::Index1D(x, y);
    int idxN = Pathfinding::Index1D(x, y - 1);
    int idxS = Pathfinding::Index1D(x, y + 1);
    int idxE = Pathfinding::Index1D(x - 1, y);
    int idxW = Pathfinding::Index1D(x + 1, y);

    // Don't search for neighbours if they already exist
    if (m_cellsFlat[idx].neighbours.size() != 0) {
        return;
    }

    // North
    if (Pathfinding::IsInBounds(x, y - 1) && !Pathfinding::IsObstacle(x, y - 1)) {
        m_cellsFlat[idx].neighbours.push_back(&m_cellsFlat[idxN]);
    }
    // South
    if (Pathfinding::IsInBounds(x, y + 1) && !Pathfinding::IsObstacle(x, y + 1)) {
        m_cellsFlat[idx].neighbours.push_back(&m_cellsFlat[idxS]);
    }
    // West
    if (Pathfinding::IsInBounds(x - 1, y) && !Pathfinding::IsObstacle(x - 1, y)) {
        m_cellsFlat[idx].neighbours.push_back(&m_cellsFlat[idxE]);
    }
    // East
    if (Pathfinding::IsInBounds(x + 1, y) && !Pathfinding::IsObstacle(x + 1, y)) {
        m_cellsFlat[idx].neighbours.push_back(&m_cellsFlat[idxW]);
    }

    //bool allowDiagonal = false;
    //if (allowDiagonal) {
    //// North West
    //    if (Pathfinding::IsInBounds(x - 1, y - 1) && !Pathfinding::IsObstacle(x - 1, y - 1)) {// && !Pathfinding::IsObstacle(x - 1, y) && !Pathfinding::IsObstacle(x, y - 1)) {
    //        //m_cellsFlat[idx].neighbours.push_back(&m_cells[x - 1][y - 1]);
    //    }
    //    // North East
    //    if (Pathfinding::IsInBounds(x + 1, y - 1) && !Pathfinding::IsObstacle(x + 1, y - 1)) {// && !Pathfinding::IsObstacle(x + 1, y) && !Pathfinding::IsObstacle(x, y - 1)) {
    //        //m_cellsFlat[idx].neighbours.push_back(&m_cells[x + 1][y - 1]);
    //    }
    //    // South West
    //    if (Pathfinding::IsInBounds(x - 1, y + 1) && !Pathfinding::IsObstacle(x - 1, y + 1)) {// && !Pathfinding::IsObstacle(x - 1, y) && !Pathfinding::IsObstacle(x, y + 1)) {
    //       // m_cellsFlat[idx].neighbours.push_back(&m_cells[x - 1][y + 1]);
    //    }
    //    // South East
    //    if (Pathfinding::IsInBounds(x + 1, y + 1) && !Pathfinding::IsObstacle(x + 1, y + 1)) {// && !Pathfinding::IsObstacle(x + 1, y) && !Pathfinding::IsObstacle(x, y + 1)) {
    //       // m_cellsFlat[idx].neighbours.push_back(&m_cells[x + 1][y + 1]);
    //    }
    //}
}