// SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include <fcntl.h>

#include <list>
#include <sstream>

#include "NajaIF.h"
// #include <boost/asio.hpp>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "db.h"
#include "dbTypes.h"
#include "naja_nl_interface.capnp.h"


//using boost::asio::ip::tcp;

// namespace {

// using namespace naja;
// using namespace naja::NL;

using namespace odb;

// Init term2busbit_
std::map<uint /*dbModBTerm id*/, std::pair<dbBusPort*, size_t /*bit index*/>> NajaIF::term2busbit_;

// void dumpProperty(
//   Property::Builder& property,
//   NajaDumpableProperty* najaProperty) {
//   property.setName(najaProperty->getName());
//   auto& propertyValues = najaProperty->getValues();
//   auto values = property.initValues(propertyValues.size());
//   size_t id = 0;
//   for (auto propertyValue: propertyValues) {
//     auto valueBuilder = values[id++];
//     if (propertyValue.index() == NajaDumpableProperty::String) {
//       valueBuilder.setText(std::get<NajaDumpableProperty::String>(propertyValue));
//     } else if (propertyValue.index() == NajaDumpableProperty::UInt64) {
//       valueBuilder.setUint64(std::get<NajaDumpableProperty::UInt64>(propertyValue));
//     }
//   }
// }

// template<typename T> void dumpProperties(
//   T& dumpObjectInterface,
//   NajaObject* object,
//   auto& initProperties) {
//   using NajaProperties = std::list<NajaDumpableProperty*>;
//   NajaProperties najaProperties(object->getDumpableProperties().begin(), object->getDumpableProperties().end());
//   auto properties = initProperties(dumpObjectInterface, najaProperties.size());
//   size_t id = 0;
//   for (auto najaProperty: najaProperties) {
//     auto propertyBuilder = properties[id++];
//     dumpProperty(propertyBuilder, najaProperty);
//   }
// }

Direction SNLtoCapnPDirection(dbIoType direction) {
  switch (direction.getValue()) {
    case dbIoType::Value::INPUT:
      return Direction::INPUT;
    case dbIoType::Value::OUTPUT:
      return Direction::OUTPUT;
    case dbIoType::Value::INOUT:
      return Direction::INOUT;
    default:
      return Direction::INPUT; // TODO: Why?
  }
  return Direction::INPUT; //LCOV_EXCL_LINE // TODO: Why?
}

// SNLDesignInterface::ParameterType SNLtoCapNpParameterType(SNLParameter::Type type) {
//   switch (type) {
//     case SNLParameter::Type::Decimal:
//       return SNLDesignInterface::ParameterType::DECIMAL;
//     case SNLParameter::Type::Binary:
//       return SNLDesignInterface::ParameterType::BINARY;
//     case SNLParameter::Type::Boolean:
//       return SNLDesignInterface::ParameterType::BOOLEAN;
//     case SNLParameter::Type::String:
//       return SNLDesignInterface::ParameterType::STRING;
//   }
//   return SNLDesignInterface::ParameterType::DECIMAL; //LCOV_EXCL_LINE
// }

void dumpScalarTerm(
  SNLDesignInterface::Term::Builder& term,
  dbModBTerm* scalarTerm) {
  auto scalarTermBuilder = term.initScalarTerm();
  scalarTermBuilder.setId(scalarTerm->getId());
  if (std::string(scalarTerm->getName()) != "") {
    scalarTermBuilder.setName(std::string(scalarTerm->getName()));
  }
  scalarTermBuilder.setDirection(SNLtoCapnPDirection(scalarTerm->getIoType()));
}

void dumpBusTerm(
  SNLDesignInterface::Term::Builder& term,
  dbBusPort* busTerm) {
  auto busTermBuilder = term.initBusTerm();
  busTermBuilder.setId(busTerm->getPort()->getId());
  if (std::string(busTerm->getPort()->getName()) != "") {
    busTermBuilder.setName(std::string(busTerm->getPort()->getName()));
  }
  busTermBuilder.setMsb(busTerm->getFrom());
  busTermBuilder.setLsb(busTerm->getTo());
  busTermBuilder.setDirection(SNLtoCapnPDirection(busTerm->getPort()->getIoType()));
  for (size_t i = busTerm->getFrom(); i <= busTerm->getTo(); i++) {
    dbModBTerm* bitTerm = busTerm->getBusIndexedElement(i);
    NajaIF::term2busbit_[bitTerm->getId()] = std::pair<dbBusPort*, size_t>(busTerm, i);
  }
}

// void dumpParameter(
//   SNLDesignInterface::Parameter::Builder& parameter,
//   SNLParameter* snlParameter) {
//   parameter.setName(snlParameter->getName().getString());
//   parameter.setType(SNLtoCapNpParameterType(snlParameter->getType()));
//   parameter.setValue(snlParameter->getValue());
// }

// DesignType SNLtoCapNpDesignType(SNLDesign::Type type) {
//   switch (type) {
//     case SNLDesign::Type::Standard:
//       return DesignType::STANDARD;
//     case SNLDesign::Type::Primitive:
//       return DesignType::PRIMITIVE;
//     case SNLDesign::Type::UserBlackBox:
//       return DesignType::USER_BLACKBOX;
//     case SNLDesign::Type::AutoBlackBox:
//       return DesignType::AUTO_BLACKBOX;
//   }
//   return DesignType::STANDARD; //LCOV_EXCL_LINE
// }

void dumpDesignInterface(
  SNLDesignInterface::Builder& designInterface,
  dbModule* module) {
  designInterface.setId(module->getId());
  if (std::string(module->getName()) != "") {
    designInterface.setName(std::string(module->getName()));
  }
//   designInterface.setType(SNLtoCapNpDesignType(module->getType()));
//   auto lambda = [](SNLDesignInterface::Builder& builder, size_t nbProperties) {
//     return builder.initProperties(nbProperties);
//   };
//   dumpProperties(designInterface, module, lambda);

  size_t id = 0;
//   auto parameters = designInterface.initParameters(module->getParameters().size());
//   for (auto parameter: module->getParameters()) {
//     auto parameterBuilder = parameters[id++];
//     dumpParameter(parameterBuilder, parameter);
//   }
  
  id = 0;
  auto terms = designInterface.initTerms(module->getModBTerms().size());
  for (auto term: module->getModBTerms()) {
    auto termBuilder = terms[id++];
    if (!term->isBusPort()) {
      dumpScalarTerm(termBuilder, term);
    } else {
      dumpBusTerm(termBuilder, term->getBusPort());
    }
  }
}

// DBInterface::LibraryType SNLtoCapnPLibraryType(NLLibrary::Type type) {
//   switch (type) {
//     case NLLibrary::Type::Standard:
//       return DBInterface::LibraryType::STANDARD;
//     case NLLibrary::Type::Primitives:
//       return DBInterface::LibraryType::PRIMITIVES;
//     //LCOV_EXCL_START
//     case NLLibrary::Type::InDB0:
//       throw NLException("Unexpected InDB0 Library type while loading Library");
//     //LCOV_EXCL_STOP
//   }
//   return DBInterface::LibraryType::STANDARD; //LCOV_EXCL_LINE
// }

// NLLibrary::Type CapnPtoNLLibraryType(DBInterface::LibraryType type) {
//   switch (type) {
//     case DBInterface::LibraryType::STANDARD:
//       return NLLibrary::Type::Standard;
//     case DBInterface::LibraryType::PRIMITIVES:
//       return  NLLibrary::Type::Primitives;
//   }
//   return NLLibrary::Type::Standard; //LCOV_EXCL_LINE
// }

void dumpLibraryInterface(
  DBInterface::LibraryInterface::Builder& libraryInterface,
  odb::dbBlock* block) {
//   libraryInterface.setId(snlLibrary->getID());
//   auto lambda = [](DBInterface::LibraryInterface::Builder& builder, size_t nbProperties) {
//     return builder.initProperties(nbProperties);
//   };
//   dumpProperties(libraryInterface, snlLibrary, lambda);

//   if (not snlLibrary->isUnnamed()) {
//     libraryInterface.setName(snlLibrary->getName().getString());
//   }
//   libraryInterface.setType(SNLtoCapnPLibraryType(snlLibrary->getType()));
//   auto subLibraries = libraryInterface.initLibraryInterfaces(snlLibrary->getLibraries().size());
//   size_t id = 0;
//   for (auto subLib: snlLibrary->getLibraries()) {
//     auto subLibraryBuilder = subLibraries[id++];
//     dumpLibraryInterface(subLibraryBuilder, subLib);
//   }
  size_t id = 0;
  auto designs = libraryInterface.initSnlDesignInterfaces(block->getModules().size());
  id = 0;
  for (auto module: block->getModules()) {
    auto designInterfaceBuilder = designs[id++]; 
    dumpDesignInterface(designInterfaceBuilder, module);
  }
} 

// }

void NajaIF::dumpInterface(odb::dbBlock* block, int fileDescriptor) {
  dumpInterface(block, fileDescriptor, block->getId());
}

void NajaIF::dumpInterface(odb::dbBlock* block, int fileDescriptor, uint8_t forceDBID) {
  ::capnp::MallocMessageBuilder message;

DBInterface::Builder db = message.initRoot<DBInterface>();
db.setId(forceDBID);
//   auto lambda = [](DBInterface::Builder& builder, size_t nbProperties) {
//     return builder.initProperties(nbProperties);
//   };
//   dumpProperties(db, snlDB, lambda);

//   auto libraries = db.initLibraryInterfaces(snlDB->getLibraries().size());
//   size_t id = 0;
//   for (auto snlLibrary: snlDB->getLibraries()) {
//     auto libraryInterfaceBuilder = libraries[id++];
//     dumpLibraryInterface(libraryInterfaceBuilder, snlLibrary);
//   }
  auto libraries = db.initLibraryInterfaces(1 /* no multiple libraries in OR*/); // TODO: Verify
  auto libraryInterfaceBuilder = libraries[0];
  dumpLibraryInterface(libraryInterfaceBuilder, block);
  if (auto topDesign = block->getTopModule()) {
    //auto designReference = topDesign->getReference();
    auto designReferenceBuilder = db.initTopDesignReference();
    designReferenceBuilder.setDbID(0 /* no multiple DBs in OR*/); // TODO: Verify
    designReferenceBuilder.setLibraryID(0 /* no multiple libraries in OR*/); // TODO: Verify
    designReferenceBuilder.setDesignID(topDesign->getId());
  }

  writePackedMessageToFd(fileDescriptor, message);
}

void NajaIF::dumpInterface(odb::dbBlock* block, const std::filesystem::path& interfacePath) {
  int fd = open(
    interfacePath.c_str(),
    O_CREAT | O_WRONLY,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  
  dumpInterface(block, fd);
  close(fd);
}
