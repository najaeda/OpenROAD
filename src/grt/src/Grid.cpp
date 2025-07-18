// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

#include "Grid.h"

#include <cmath>
#include <complex>

namespace grt {

void Grid::init(const odb::Rect& die_area,
                const int tile_size,
                const int x_grids,
                const int y_grids,
                const bool perfect_regular_x,
                const bool perfect_regular_y,
                const int num_layers)
{
  die_area_ = die_area;
  tile_size_ = tile_size;
  x_grids_ = x_grids;
  y_grids_ = y_grids;
  perfect_regular_x_ = perfect_regular_x;
  perfect_regular_y_ = perfect_regular_y;
  num_layers_ = num_layers;
  track_pitches_.resize(num_layers);
  horizontal_edges_capacities_.resize(num_layers);
  vertical_edges_capacities_.resize(num_layers);
}

void Grid::clear()
{
  track_pitches_.clear();
  horizontal_edges_capacities_.clear();
  vertical_edges_capacities_.clear();
}

odb::Point Grid::getPositionOnGrid(const odb::Point& position)
{
  int x = position.x();
  int y = position.y();

  // Computing x and y center:
  int gcell_id_x = (x - die_area_.xMin()) / tile_size_;
  int gcell_id_y = (y - die_area_.yMin()) / tile_size_;

  if (gcell_id_x >= x_grids_)
    gcell_id_x--;

  if (gcell_id_y >= y_grids_)
    gcell_id_y--;

  int center_x
      = (gcell_id_x * tile_size_) + (tile_size_ / 2) + die_area_.xMin();
  int center_y
      = (gcell_id_y * tile_size_) + (tile_size_ / 2) + die_area_.yMin();

  return odb::Point(center_x, center_y);
}

void Grid::getBlockedTiles(const odb::Rect& obstruction,
                           odb::Rect& first_tile_bds,
                           odb::Rect& last_tile_bds,
                           odb::Point& first_tile,
                           odb::Point& last_tile)
{
  odb::Point lower = obstruction.ll();  // lower bound of obstruction
  odb::Point upper = obstruction.ur();  // upper bound of obstruction

  lower
      = getPositionOnGrid(lower);  // translate lower bound of obstruction to
                                   // the center of the tile where it is inside
  upper
      = getPositionOnGrid(upper);  // translate upper bound of obstruction to
                                   // the center of the tile where it is inside

  // Get x and y indices of first blocked tile
  first_tile = {(lower.x() - getXMin()) / getTileSize(),
                (lower.y() - getYMin()) / getTileSize()};

  // Get x and y indices of last blocked tile
  last_tile = {(upper.x() - getXMin()) / getTileSize(),
               (upper.y() - getYMin()) / getTileSize()};

  odb::Point ll_first_tile = odb::Point(lower.x() - (getTileSize() / 2),
                                        lower.y() - (getTileSize() / 2));
  odb::Point ur_first_tile = odb::Point(lower.x() + (getTileSize() / 2),
                                        lower.y() + (getTileSize() / 2));

  odb::Point ll_last_tile = odb::Point(upper.x() - (getTileSize() / 2),
                                       upper.y() - (getTileSize() / 2));
  odb::Point ur_last_tile = odb::Point(upper.x() + (getTileSize() / 2),
                                       upper.y() + (getTileSize() / 2));

  if ((die_area_.xMax() - ur_last_tile.x()) / getTileSize() < 1) {
    ur_last_tile.setX(die_area_.xMax());
  }
  if ((die_area_.yMax() - ur_last_tile.y()) / getTileSize() < 1) {
    ur_last_tile.setY(die_area_.yMax());
  }

  first_tile_bds = odb::Rect(ll_first_tile, ur_first_tile);
  last_tile_bds = odb::Rect(ll_last_tile, ur_last_tile);
}

interval<int>::type Grid::computeTileReduceInterval(
    const odb::Rect& obs,
    const odb::Rect& tile,
    int track_space,
    bool first,
    const odb::dbTechLayerDir& direction,
    const int layer_cap,
    const bool is_macro)
{
  int start_point, end_point;
  if (direction == odb::dbTechLayerDir::VERTICAL) {
    if (obs.xMin() >= tile.xMin() && obs.xMax() <= tile.xMax()) {
      start_point = obs.xMin();
      end_point = obs.xMax();
    } else if (first) {
      start_point = obs.xMin();
      end_point = tile.xMax();
    } else {
      start_point = tile.xMin();
      end_point = obs.xMax();
    }
  } else {
    if (obs.yMin() >= tile.yMin() && obs.yMax() <= tile.yMax()) {
      start_point = obs.yMin();
      end_point = obs.yMax();
    } else if (first) {
      start_point = obs.yMin();
      end_point = tile.yMax();
    } else {
      start_point = tile.yMin();
      end_point = obs.yMax();
    }
  }
  int interval_length = std::abs(end_point - start_point);
  if (is_macro) {
    int blocked_tracks
        = std::ceil(static_cast<float>(interval_length) / track_space);
    const int resources_left = layer_cap - blocked_tracks;
    if (resources_left == 1) {
      end_point += track_space;
    }
  }
  interval<int>::type reduce_interval(start_point, end_point);
  return reduce_interval;
}

int Grid::computeTileReduce(const odb::Rect& obs,
                            const odb::Rect& tile,
                            double track_space,
                            bool first,
                            odb::dbTechLayerDir direction)
{
  int reduce = -1;
  if (direction == odb::dbTechLayerDir::VERTICAL) {
    if (obs.xMin() >= tile.xMin() && obs.xMax() <= tile.xMax()) {
      reduce = ceil(std::abs(obs.xMax() - obs.xMin()) / track_space);
    } else if (first) {
      reduce = ceil(std::abs(tile.xMax() - obs.xMin()) / track_space);
    } else {
      reduce = ceil(std::abs(obs.xMax() - tile.xMin()) / track_space);
    }
  } else {
    if (obs.yMin() >= tile.yMin() && obs.yMax() <= tile.yMax()) {
      reduce = ceil(std::abs(obs.yMax() - obs.yMin()) / track_space);
    } else if (first) {
      reduce = ceil(std::abs(tile.yMax() - obs.yMin()) / track_space);
    } else {
      reduce = ceil(std::abs(obs.yMax() - tile.yMin()) / track_space);
    }
  }

  return reduce;
}

odb::Point Grid::getMiddle()
{
  return odb::Point((die_area_.xMin() + (die_area_.dx() / 2.0)),
                    (die_area_.yMin() + (die_area_.dy() / 2.0)));
}

const odb::Rect& Grid::getGridArea() const
{
  return die_area_;
}

}  // namespace grt
