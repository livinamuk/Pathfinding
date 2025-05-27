#include "Cell.h"

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