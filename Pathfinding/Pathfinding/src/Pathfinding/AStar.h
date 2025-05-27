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
    MinHeap& GetOpenList();

    Cell* m_start;
    Cell* m_destination;
    Cell* m_current;
    MinHeap m_openList;
    std::vector<Cell> m_cellsFlat;
    std::vector<Cell*> m_finalPath;
    std::vector<glm::vec2> m_intersectionPoints;
    std::vector<bool> m_closedFlags;

    const std::vector<bool>& GetClosedFlags() { return m_closedFlags; }

private:
    void BuildFinalPath();
    void FindNeighbours(Cell* cell);
    bool IsDestination(Cell* cell);
    bool IsOrthogonal(Cell* cellA, Cell* cellB);
    bool IsInClosedList(Cell* cell);

    int m_smoothSearchIndex = 0;
    bool m_gridPathFound = false;
    bool m_smoothPathFound = false;
    bool m_searchInitilized = false;
};