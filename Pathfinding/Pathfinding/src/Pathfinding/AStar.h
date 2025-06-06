#pragma once
#include <vector>
#include "Cell.h"
#include "MinHeap.h"
#include "glm/glm.hpp"

struct AStar {
    void InitSearch(int startX, int startY, int destinationX, int destinationY);
    void FindPath();
    void FindSmoothPath();
    void ClearData();
    bool GridPathFound();
    bool SmoothPathFound();
    bool SearchInitilized();
    void PrintPath();
    std::vector<Cell*>& GetPath();

    Cell* m_start;
    Cell* m_destination;
    Cell* m_current;
    MinHeap m_openList;

    std::vector<Cell> m_cells;
    std::vector<Cell*> m_finalPath;
    std::vector<glm::vec2> m_intersectionPoints;
    std::vector<bool> m_closedFlags;

    MinHeap& GetOpenList()                      { return m_openList; }
    const std::vector<bool>& GetClosedFlags()   { return m_closedFlags; }
    inline int Index1D(int x, int y)            { return y * m_mapWidth + x; }

private:
    void BuildFinalPath();
    void FindNeighbours(int idx);
    bool IsDestination(Cell* cell);
    bool IsOrthogonal(Cell* cellA, Cell* cellB);
    bool IsInClosedList(Cell* cell);

    int m_smoothSearchIndex = 0;
    bool m_gridPathFound = false;
    bool m_smoothPathFound = false;
    bool m_searchInitilized = false;
    int m_mapWidth = 0;
    int m_mapHeight = 0;
};