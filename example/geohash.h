#pragma once
#include <vector>
#include <cstdint>

uint64_t GeoHash(int latE5, int lngE5, int level);
void GeoHashRange(int latE5, int lngE5, int level, int distance, std::vector<uint64_t>* cells);
