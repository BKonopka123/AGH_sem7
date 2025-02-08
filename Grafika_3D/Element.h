#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


class Element {
private:
	std::vector<Element*> _neighbours;
	bool _isBomb;
	int _bombsNeighbours;
	std::string _boardPlacement;
	int _xPosition;
	int _yPosition;
	bool _isShowed = false;

public:
	Element(bool isBomb, std::string boardPlacement, int xPosition, int yPosition);
	~Element();
	
	bool getIsBomb() const {return _isBomb;}
	void setIsBomb(bool isBomb) { _isBomb = isBomb; }
	bool isBomb() const { return _isBomb; }
	const std::string& boardPlacement() const { return _boardPlacement; }
	int xPosition() const { return _xPosition; }
	int yPosition() const { return _yPosition; }
	int bombsNeighbours() const { return _bombsNeighbours; }
	std::vector<Element*> getNeighbours() const { return _neighbours; }
	bool getIsShowed() const { return _isShowed; }

	void addNeighbour(Element* neighbour) {_neighbours.push_back(neighbour);}
	void updateBombsNeighbours();
	void setShowed(bool b) { _isShowed = b; }
};

