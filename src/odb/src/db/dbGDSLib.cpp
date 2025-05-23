// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022-2025, The OpenROAD Authors

#include "dbGDSLib.h"

#include <utility>

#include "dbDatabase.h"
#include "odb/db.h"
#include "odb/dbTypes.h"

namespace odb {
template class dbTable<_dbGDSLib>;

bool _dbGDSLib::operator==(const _dbGDSLib& rhs) const
{
  if (_libname != rhs._libname) {
    return false;
  }
  if (_uu_per_dbu != rhs._uu_per_dbu) {
    return false;
  }
  if (_dbu_per_meter != rhs._dbu_per_meter) {
    return false;
  }
  if (*_gdsstructure_tbl != *rhs._gdsstructure_tbl) {
    return false;
  }
  if (_gdsstructure_hash != rhs._gdsstructure_hash) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//
// dbGDSLib - Methods
//
////////////////////////////////////////////////////////////////////

dbObjectTable* _dbGDSLib::getObjectTable(dbObjectType type)
{
  switch (type) {
    case dbGDSStructureObj:
      return _gdsstructure_tbl;
    default:
      break;
  }
  return getTable()->getObjectTable(type);
}

_dbGDSLib::_dbGDSLib(_dbDatabase* db)
{
  _uu_per_dbu = 1.0;
  _dbu_per_meter = 1e9;

  _gdsstructure_tbl = new dbTable<_dbGDSStructure>(
      db, this, (GetObjTbl_t) &_dbGDSLib::getObjectTable, dbGDSStructureObj);

  _gdsstructure_hash.setTable(_gdsstructure_tbl);
}

_dbGDSLib::_dbGDSLib(_dbDatabase* db, const _dbGDSLib& r)
    : _libname(r._libname),
      _uu_per_dbu(r._uu_per_dbu),
      _dbu_per_meter(r._dbu_per_meter),
      _gdsstructure_hash(r._gdsstructure_hash),
      _gdsstructure_tbl(r._gdsstructure_tbl)
{
}

_dbGDSLib::~_dbGDSLib()
{
  delete _gdsstructure_tbl;
}

dbIStream& operator>>(dbIStream& stream, _dbGDSLib& obj)
{
  stream >> obj._libname;
  stream >> obj._uu_per_dbu;
  stream >> obj._dbu_per_meter;
  stream >> *obj._gdsstructure_tbl;
  stream >> obj._gdsstructure_hash;
  return stream;
}

dbOStream& operator<<(dbOStream& stream, const _dbGDSLib& obj)
{
  stream << obj._libname;
  stream << obj._uu_per_dbu;
  stream << obj._dbu_per_meter;
  stream << NamedTable("_structure_tbl", obj._gdsstructure_tbl);
  stream << obj._gdsstructure_hash;
  return stream;
}

_dbGDSStructure* _dbGDSLib::findStructure(const char* name)
{
  return _gdsstructure_hash.find(name);
}

////////////////////////////////////////////////////////////////////
//
// dbLib - Methods
//
////////////////////////////////////////////////////////////////////

void dbGDSLib::setLibname(std::string libname)
{
  _dbGDSLib* obj = (_dbGDSLib*) this;

  obj->_libname = std::move(libname);
}

std::string dbGDSLib::getLibname() const
{
  _dbGDSLib* obj = (_dbGDSLib*) this;
  return obj->_libname;
}

void dbGDSLib::setUnits(double uu_per_dbu, double dbu_per_meter)
{
  _dbGDSLib* obj = (_dbGDSLib*) this;

  obj->_uu_per_dbu = uu_per_dbu;
  obj->_dbu_per_meter = dbu_per_meter;
}

std::pair<double, double> dbGDSLib::getUnits() const
{
  _dbGDSLib* obj = (_dbGDSLib*) this;
  return std::make_pair(obj->_uu_per_dbu, obj->_dbu_per_meter);
}

dbGDSStructure* dbGDSLib::findGDSStructure(const char* name) const
{
  _dbGDSLib* obj = (_dbGDSLib*) this;
  return (dbGDSStructure*) obj->_gdsstructure_hash.find(name);
}

dbSet<dbGDSStructure> dbGDSLib::getGDSStructures()
{
  _dbGDSLib* obj = (_dbGDSLib*) this;
  return dbSet<dbGDSStructure>(obj, obj->_gdsstructure_tbl);
}

dbGDSLib* dbGDSLib::create(dbDatabase* db, const std::string& name)
{
  auto* obj = (_dbDatabase*) db;
  auto lib = (dbGDSLib*) obj->_gds_lib_tbl->create();
  lib->setLibname(name);
  return lib;
}

void dbGDSLib::destroy(dbGDSLib* lib)
{
  auto* obj = (_dbGDSLib*) lib;
  auto* db = (_dbDatabase*) obj->getOwner();
  db->_gds_lib_tbl->destroy(obj);
}

void _dbGDSLib::collectMemInfo(MemInfo& info)
{
  info.cnt++;
  info.size += sizeof(*this);

  info.children_["libname"].add(_libname);
  info.children_["structure_hash"].add(_gdsstructure_hash);
  _gdsstructure_tbl->collectMemInfo(info.children_["structure"]);
}

}  // namespace odb
