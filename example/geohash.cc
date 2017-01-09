
#include "geometry/s2/s2cellid.h"
#include "geometry/s2/s2latlng.h"

uint64_t GeoHash(int latE5, int lngE5, int level) {
  auto ll = S2LatLng::FromE5(latE5, lngE5);
  auto cid = S2CellId::FromLatLng(ll);
  return cid.parent(level).id();
}

void GeoHashRange(int latE5, int lngE5, int level, int distance, std::vector<uint64_t>* cells) {
  auto ll = S2LatLng::FromE5(latE5, lngE5);
  auto cid = S2CellId::FromLatLng(ll).parent(level);
  auto pid = cid.advance_wrap(-distance);
  for (int i = 0; i < 2 * distance; ++ i) {
    cells->emplace_back(pid.id());
    pid = pid.next_wrap();
  }
}
