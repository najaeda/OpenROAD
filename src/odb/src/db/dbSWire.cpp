// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

#include "dbSWire.h"

#include "dbBlock.h"
#include "dbNet.h"
#include "dbSBox.h"
#include "dbSBoxItr.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "odb/db.h"
#include "odb/dbBlockCallBackObj.h"
#include "odb/dbSet.h"

namespace odb {

template class dbTable<_dbSWire>;

bool _dbSWire::operator==(const _dbSWire& rhs) const
{
  if (_flags._wire_type != rhs._flags._wire_type) {
    return false;
  }

  if (_net != rhs._net) {
    return false;
  }

  if (_shield != rhs._shield) {
    return false;
  }

  if (_wires != rhs._wires) {
    return false;
  }

  if (_next_swire != rhs._next_swire) {
    return false;
  }

  return true;
}

bool _dbSWire::operator<(const _dbSWire& rhs) const
{
  if (_flags._wire_type < rhs._flags._wire_type) {
    return true;
  }

  if (_flags._wire_type > rhs._flags._wire_type) {
    return false;
  }

  if ((_shield != 0) && (rhs._shield != 0)) {
    _dbBlock* lhs_blk = (_dbBlock*) getOwner();
    _dbBlock* rhs_blk = (_dbBlock*) rhs.getOwner();
    _dbNet* lhs_net = lhs_blk->_net_tbl->getPtr(_net);
    _dbNet* rhs_net = rhs_blk->_net_tbl->getPtr(rhs._net);
    int r = strcmp(lhs_net->_name, rhs_net->_name);

    if (r < 0) {
      return true;
    }

    if (r > 0) {
      return false;
    }
  } else if (_shield != 0) {
    return false;
  } else if (rhs._shield != 0) {
    return true;
  }

  return false;
}

void _dbSWire::addSBox(_dbSBox* box)
{
  box->_owner = getOID();
  box->_next_box = (uint) _wires;
  _wires = box->getOID();
  _dbBlock* block = (_dbBlock*) getOwner();
  for (auto callback : block->_callbacks) {
    callback->inDbSWireAddSBox((dbSBox*) box);
  }
}

void _dbSWire::removeSBox(_dbSBox* box)
{
  _dbBlock* block = (_dbBlock*) getOwner();
  uint boxid = box->getOID();
  if (boxid == _wires) {
    // at head of list, need to move head
    _wires = (uint) box->_next_box;
  } else {
    // in the middle of the list, need to iterate and relink
    dbId<_dbSBox> id = _wires;
    if (id == 0) {
      return;
    }
    while (id != 0) {
      _dbSBox* nbox = block->_sbox_tbl->getPtr(id);
      uint nid = nbox->_next_box;

      if (nid == boxid) {
        nbox->_next_box = box->_next_box;
        break;
      }

      id = nid;
    }
  }

  for (auto callback : block->_callbacks) {
    callback->inDbSWireRemoveSBox((dbSBox*) box);
  }
}

dbBlock* dbSWire::getBlock()
{
  return (dbBlock*) getImpl()->getOwner();
}

dbNet* dbSWire::getNet()
{
  _dbSWire* wire = (_dbSWire*) this;
  _dbBlock* block = (_dbBlock*) wire->getOwner();
  return (dbNet*) block->_net_tbl->getPtr(wire->_net);
}

dbWireType dbSWire::getWireType()
{
  _dbSWire* wire = (_dbSWire*) this;
  return wire->_flags._wire_type;
}

dbNet* dbSWire::getShield()
{
  _dbSWire* wire = (_dbSWire*) this;

  if (wire->_shield == 0) {
    return nullptr;
  }

  _dbBlock* block = (_dbBlock*) wire->getOwner();
  return (dbNet*) block->_net_tbl->getPtr(wire->_shield);
}

dbSet<dbSBox> dbSWire::getWires()
{
  _dbSWire* wire = (_dbSWire*) this;
  _dbBlock* block = (_dbBlock*) wire->getOwner();
  return dbSet<dbSBox>(wire, block->_sbox_itr);
}

dbSWire* dbSWire::create(dbNet* net_, dbWireType type, dbNet* shield_)
{
  _dbNet* net = (_dbNet*) net_;
  _dbNet* shield = (_dbNet*) shield_;
  _dbBlock* block = (_dbBlock*) net->getOwner();

  _dbSWire* wire = block->_swire_tbl->create();
  wire->_flags._wire_type = type.getValue();
  wire->_net = net->getOID();
  wire->_next_swire = net->_swires;
  net->_swires = wire->getOID();

  if (shield) {
    wire->_shield = shield->getOID();
  }
  for (auto callback : block->_callbacks) {
    callback->inDbSWireCreate((dbSWire*) wire);
  }
  return (dbSWire*) wire;
}

static void destroySBoxes(_dbSWire* wire)
{
  _dbBlock* block = (_dbBlock*) wire->getOwner();
  dbId<_dbSBox> id = wire->_wires;
  if (id == 0) {
    return;
  }
  for (auto callback : block->_callbacks) {
    callback->inDbSWirePreDestroySBoxes((dbSWire*) wire);
  }
  while (id != 0) {
    _dbSBox* box = block->_sbox_tbl->getPtr(id);
    uint nid = box->_next_box;
    dbProperty::destroyProperties(box);
    block->_sbox_tbl->destroy(box);
    id = nid;
  }
  for (auto callback : block->_callbacks) {
    callback->inDbSWirePostDestroySBoxes((dbSWire*) wire);
  }
}

void dbSWire::destroy(dbSWire* wire_)
{
  _dbSWire* wire = (_dbSWire*) wire_;
  _dbBlock* block = (_dbBlock*) wire->getOwner();
  _dbNet* net = block->_net_tbl->getPtr(wire->_net);
  _dbSWire* prev = nullptr;
  dbId<_dbSWire> id;
  // destroy the sboxes
  destroySBoxes(wire);
  for (auto callback : block->_callbacks) {
    callback->inDbSWireDestroy(wire_);
  }
  // unlink the swire
  for (id = net->_swires; id != 0; id = prev->_next_swire) {
    _dbSWire* w = block->_swire_tbl->getPtr(id);
    if (w == wire) {
      if (prev == nullptr) {
        net->_swires = w->_next_swire;
      } else {
        prev->_next_swire = w->_next_swire;
      }
      break;
    }
    prev = w;
  }
  ZASSERT(id != 0);
  // destroy the wire
  dbProperty::destroyProperties(wire);
  block->_swire_tbl->destroy(wire);
}

dbSet<dbSWire>::iterator dbSWire::destroy(dbSet<dbSWire>::iterator& itr)
{
  dbSWire* w = *itr;
  dbSet<dbSWire>::iterator next = ++itr;
  destroy(w);
  return next;
}

dbSWire* dbSWire::getSWire(dbBlock* block_, uint dbid_)
{
  _dbBlock* block = (_dbBlock*) block_;
  return (dbSWire*) block->_swire_tbl->getPtr(dbid_);
}

void _dbSWire::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);
}

}  // namespace odb
