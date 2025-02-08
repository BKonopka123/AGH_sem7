#include "Element.h"
#include "Board.h"
#include "pch.h"

Element::Element(bool isBomb, std::string boardPlacement, int xPosition, int yPosition) : _isBomb(isBomb), _boardPlacement(boardPlacement), _yPosition(yPosition), _xPosition(xPosition){
	_bombsNeighbours = 0;
	_neighbours = {};
}

Element::~Element() {}

void Element::updateBombsNeighbours() {
    int count = 0;
    for (auto* neighbour : _neighbours) {
        if (neighbour->isBomb()) {
            count++;
        }
    }
    _bombsNeighbours = count;
}
