#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "block_library.hpp"
#include "coordinate.hpp"

struct Block {
    Block(const Coordinate& location, BlockLibrary::Tag blockType)
    : location(location), blockType(blockType) {}

    Coordinate location;
    BlockLibrary::Tag blockType;
};

#endif