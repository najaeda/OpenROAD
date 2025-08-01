// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2025, The OpenROAD Authors

#include "dbTechNonDefaultRule.h"

#include <string>
#include <vector>

#include "dbBlock.h"
#include "dbDatabase.h"
#include "dbTable.h"
#include "dbTable.hpp"
#include "dbTech.h"
#include "dbTechLayer.h"
#include "dbTechLayerRule.h"
#include "dbTechSameNetRule.h"
#include "dbTechVia.h"
#include "odb/db.h"

namespace odb {

template class dbTable<_dbTechNonDefaultRule>;

_dbTechNonDefaultRule::_dbTechNonDefaultRule(_dbDatabase*,
                                             const _dbTechNonDefaultRule& r)
    : _flags(r._flags),
      _name(nullptr),
      _layer_rules(r._layer_rules),
      _vias(r._vias),
      _samenet_rules(r._samenet_rules),
      _samenet_matrix(r._samenet_matrix),
      _use_vias(r._use_vias),
      _use_rules(r._use_rules),
      _cut_layers(r._cut_layers),
      _min_cuts(r._min_cuts)
{
  if (r._name) {
    _name = safe_strdup(r._name);
  }
}

_dbTechNonDefaultRule::_dbTechNonDefaultRule(_dbDatabase*)
{
  _flags._spare_bits = 0;
  _flags._hard_spacing = 0;
  _flags._block_rule = 0;
  _name = nullptr;
}

_dbTechNonDefaultRule::~_dbTechNonDefaultRule()
{
  if (_name) {
    free((void*) _name);
  }
}

dbOStream& operator<<(dbOStream& stream, const _dbTechNonDefaultRule& rule)
{
  uint* bit_field = (uint*) &rule._flags;
  stream << *bit_field;
  stream << rule._name;
  stream << rule._layer_rules;
  stream << rule._vias;
  stream << rule._samenet_rules;
  stream << rule._samenet_matrix;
  stream << rule._use_vias;
  stream << rule._use_rules;
  stream << rule._cut_layers;
  stream << rule._min_cuts;
  return stream;
}

dbIStream& operator>>(dbIStream& stream, _dbTechNonDefaultRule& rule)
{
  uint* bit_field = (uint*) &rule._flags;
  stream >> *bit_field;
  stream >> rule._name;
  stream >> rule._layer_rules;
  stream >> rule._vias;
  stream >> rule._samenet_rules;
  stream >> rule._samenet_matrix;
  stream >> rule._use_vias;
  stream >> rule._use_rules;
  stream >> rule._cut_layers;
  stream >> rule._min_cuts;

  return stream;
}

bool _dbTechNonDefaultRule::operator==(const _dbTechNonDefaultRule& rhs) const
{
  if (_flags._hard_spacing != rhs._flags._hard_spacing) {
    return false;
  }

  if (_flags._block_rule != rhs._flags._block_rule) {
    return false;
  }

  if (_name && rhs._name) {
    if (strcmp(_name, rhs._name) != 0) {
      return false;
    }
  } else if (_name || rhs._name) {
    return false;
  }

  if (_layer_rules != rhs._layer_rules) {
    return false;
  }

  if (_vias != rhs._vias) {
    return false;
  }

  if (_samenet_rules != rhs._samenet_rules) {
    return false;
  }

  if (_samenet_matrix != rhs._samenet_matrix) {
    return false;
  }

  if (_use_vias != rhs._use_vias) {
    return false;
  }

  if (_use_rules != rhs._use_rules) {
    return false;
  }

  if (_cut_layers != rhs._cut_layers) {
    return false;
  }

  if (_min_cuts != rhs._min_cuts) {
    return false;
  }

  return true;
}

_dbTech* _dbTechNonDefaultRule::getTech()
{
  if (_flags._block_rule == 0) {
    return (_dbTech*) getOwner();
  }

  return (_dbTech*) getBlock()->getTech();
}

_dbBlock* _dbTechNonDefaultRule::getBlock()
{
  assert(_flags._block_rule == 1);
  return (_dbBlock*) getOwner();
}

void _dbTechNonDefaultRule::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  info.children_["name"].add(_name);
  info.children_["_layer_rules"].add(_layer_rules);
  info.children_["_vias"].add(_vias);
  info.children_["_samenet_rules"].add(_samenet_rules);
  info.children_["_samenet_matrix"].add(_samenet_matrix);
  info.children_["_use_vias"].add(_use_vias);
  info.children_["_use_rules"].add(_use_rules);
  info.children_["_cut_layers"].add(_cut_layers);
  info.children_["_min_cuts"].add(_min_cuts);
}

bool _dbTechNonDefaultRule::operator<(const _dbTechNonDefaultRule& rhs) const
{
  return strcmp(_name, rhs._name) < 0;
}

////////////////////////////////////////////////////////////////////
//
// dbTechNonDefaultRule - Methods
//
////////////////////////////////////////////////////////////////////

std::string dbTechNonDefaultRule::getName()
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  return rule->_name;
}

const char* dbTechNonDefaultRule::getConstName()
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  return rule->_name;
}

bool dbTechNonDefaultRule::isBlockRule()
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  return rule->_flags._block_rule == 1;
}

dbTechLayerRule* dbTechNonDefaultRule::getLayerRule(dbTechLayer* layer_)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  _dbTechLayer* layer = (_dbTechLayer*) layer_;
  dbId<_dbTechLayerRule> id = rule->_layer_rules[layer->_number];

  if (id == 0) {
    return nullptr;
  }

  if (rule->_flags._block_rule == 0) {
    return (dbTechLayerRule*) rule->getTech()->_layer_rule_tbl->getPtr(id);
  }
  return (dbTechLayerRule*) rule->getBlock()->_layer_rule_tbl->getPtr(id);
}

void dbTechNonDefaultRule::getLayerRules(
    std::vector<dbTechLayerRule*>& layer_rules)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;

  layer_rules.clear();

  auto add_rules = [rule, &layer_rules](auto& tbl) {
    for (const auto& id : rule->_layer_rules) {
      if (id.isValid()) {
        layer_rules.push_back((dbTechLayerRule*) tbl->getPtr(id));
      }
    }
  };

  if (rule->_flags._block_rule == 0) {
    add_rules(rule->getTech()->_layer_rule_tbl);
  } else {
    add_rules(rule->getBlock()->_layer_rule_tbl);
  }
}

void dbTechNonDefaultRule::getVias(std::vector<dbTechVia*>& vias)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;

  vias.clear();

  if (rule->_flags._block_rule == 1) {  // not supported on block rules
    return;
  }

  _dbTech* tech = rule->getTech();

  for (const auto& id : rule->_vias) {
    vias.push_back((dbTechVia*) tech->_via_tbl->getPtr(id));
  }
}

dbTechSameNetRule* dbTechNonDefaultRule::findSameNetRule(dbTechLayer* l1_,
                                                         dbTechLayer* l2_)
{
  _dbTechNonDefaultRule* ndrule = (_dbTechNonDefaultRule*) this;

  if (ndrule->_flags._block_rule == 1) {  // not supported on block rules
    return nullptr;
  }

  _dbTech* tech = ndrule->getTech();
  _dbTechLayer* l1 = (_dbTechLayer*) l1_;
  _dbTechLayer* l2 = (_dbTechLayer*) l2_;
  dbId<_dbTechSameNetRule> rule
      = ndrule->_samenet_matrix(l1->_number, l2->_number);

  if (rule == 0) {
    return nullptr;
  }

  return (dbTechSameNetRule*) tech->_samenet_rule_tbl->getPtr(rule);
}

void dbTechNonDefaultRule::getSameNetRules(
    std::vector<dbTechSameNetRule*>& rules)
{
  _dbTechNonDefaultRule* ndrule = (_dbTechNonDefaultRule*) this;

  rules.clear();

  if (ndrule->_flags._block_rule == 1) {  // not supported on block rules
    return;
  }

  _dbTech* tech = ndrule->getTech();

  for (const auto& id : ndrule->_samenet_rules) {
    rules.push_back((dbTechSameNetRule*) tech->_samenet_rule_tbl->getPtr(id));
  }
}

bool dbTechNonDefaultRule::getHardSpacing()
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  return rule->_flags._hard_spacing == 1;
}

void dbTechNonDefaultRule::setHardSpacing(bool value)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  rule->_flags._hard_spacing = value;
}

void dbTechNonDefaultRule::addUseVia(dbTechVia* via)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  rule->_use_vias.push_back(via->getId());
}

void dbTechNonDefaultRule::getUseVias(std::vector<dbTechVia*>& vias)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  _dbTech* tech = rule->getTech();

  for (const auto& vid : rule->_use_vias) {
    dbTechVia* via = dbTechVia::getTechVia((dbTech*) tech, vid);
    vias.push_back(via);
  }
}

void dbTechNonDefaultRule::addUseViaRule(dbTechViaGenerateRule* gen_rule)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  rule->_use_rules.push_back(gen_rule->getId());
}

void dbTechNonDefaultRule::getUseViaRules(
    std::vector<dbTechViaGenerateRule*>& rules)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;
  _dbTech* tech = rule->getTech();

  for (const auto& rid : rule->_use_rules) {
    dbTechViaGenerateRule* rule
        = dbTechViaGenerateRule::getTechViaGenerateRule((dbTech*) tech, rid);
    rules.push_back(rule);
  }
}

void dbTechNonDefaultRule::setMinCuts(dbTechLayer* cut_layer, const int count)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;

  const uint id = cut_layer->getId();
  uint idx = 0;

  for (const auto& lid : rule->_cut_layers) {
    if (lid == id) {
      rule->_min_cuts[idx] = count;
      return;
    }
    ++idx;
  }

  rule->_cut_layers.push_back(id);
  rule->_min_cuts.push_back(count);
}

bool dbTechNonDefaultRule::getMinCuts(dbTechLayer* cut_layer, int& count)
{
  _dbTechNonDefaultRule* rule = (_dbTechNonDefaultRule*) this;

  const uint id = cut_layer->getId();
  uint idx = 0;

  for (const auto& lid : rule->_cut_layers) {
    if (lid == id) {
      count = rule->_min_cuts[idx];
      return true;
    }
    ++idx;
  }

  return false;
}

dbTechNonDefaultRule* dbTechNonDefaultRule::create(dbTech* tech_,
                                                   const char* name_)
{
  if (tech_->findNonDefaultRule(name_)) {
    return nullptr;
  }

  _dbTech* tech = (_dbTech*) tech_;
  _dbTechNonDefaultRule* rule = tech->_non_default_rule_tbl->create();
  rule->_name = safe_strdup(name_);
  rule->_layer_rules.resize(tech->_layer_cnt);

  int i;
  for (i = 0; i < tech->_layer_cnt; ++i) {
    rule->_layer_rules.push_back(0);
  }

  return (dbTechNonDefaultRule*) rule;
}

dbTechNonDefaultRule* dbTechNonDefaultRule::create(dbBlock* block_,
                                                   const char* name_)
{
  if (block_->findNonDefaultRule(name_)) {
    return nullptr;
  }

  _dbBlock* block = (_dbBlock*) block_;
  _dbTech* tech = (_dbTech*) block->getDb()->getTech();
  _dbTechNonDefaultRule* rule = block->_non_default_rule_tbl->create();

  rule->_name = safe_strdup(name_);
  rule->_flags._block_rule = 1;
  rule->_layer_rules.resize(tech->_layer_cnt);

  int i;
  for (i = 0; i < tech->_layer_cnt; ++i) {
    rule->_layer_rules.push_back(0);
  }

  return (dbTechNonDefaultRule*) rule;
}

dbTechNonDefaultRule* dbTechNonDefaultRule::getTechNonDefaultRule(dbTech* tech_,
                                                                  uint dbid_)
{
  _dbTech* tech = (_dbTech*) tech_;
  return (dbTechNonDefaultRule*) tech->_non_default_rule_tbl->getPtr(dbid_);
}

dbTechNonDefaultRule* dbTechNonDefaultRule::getTechNonDefaultRule(
    dbBlock* block_,
    uint dbid_)
{
  _dbBlock* block = (_dbBlock*) block_;
  return (dbTechNonDefaultRule*) block->_non_default_rule_tbl->getPtr(dbid_);
}

}  // namespace odb
