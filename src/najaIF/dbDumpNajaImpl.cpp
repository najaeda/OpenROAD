#include <fcntl.h>

#include <iostream>
#include <sstream>

#include "NajaIF.h"

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <cassert>

#include "db.h"
#include "dbTypes.h"
#include "naja_nl_implementation.capnp.h"

using namespace odb;

DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType ODBtoCapnPNetType(dbSigType type) {
  switch (type.getValue()) {
    case dbSigType::Value::SIGNAL:
      return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::STANDARD;
    // case dbSigType::Value::GROUND:
    //   return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::ASSIGN0;
    // case dbSigType::Value::POWER:
    //   return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::ASSIGN1;
    case dbSigType::Value::GROUND:
      return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::SUPPLY0;
    case dbSigType::Value::POWER:
      return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::SUPPLY1;
    default:
      break;
  }
  return DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::STANDARD; //LCOV_EXCL_LINE
}

// void dumpInstParameter(
//   DBImplementation::LibraryImplementation::SNLDesignImplementation::Instance::InstParameter::Builder& instParameter,
//   const SNLInstParameter* snlInstParameter) {
//   instParameter.setName(snlInstParameter->getName().getString());
//   instParameter.setValue(snlInstParameter->getValue());
// }

void dumpInstance(
  DBImplementation::LibraryImplementation::SNLDesignImplementation::Instance::Builder& instance,
  dbInst* dbInst) {
  instance.setId(dbInst->getId());
  if (std::string(dbInst->getName()) != "") {
    instance.setName(std::string(dbInst->getName()));
  }
  auto model = dbInst->getMaster();
  auto modelReferenceBuilder = instance.initModelReference();
  modelReferenceBuilder.setDbID(0 /* no multiple DBs in OR*/); // TODO: Verify
  modelReferenceBuilder.setLibraryID(0 /* no multiple libraries in OR*/); // TODO: Verify
  modelReferenceBuilder.setDesignID(model->getId());
//   modelReferenceBuilder.setDbID(modelReference.dbID_);
//   modelReferenceBuilder.setLibraryID(modelReference.getDBDesignReference().libraryID_);
//   modelReferenceBuilder.setDesignID(modelReference.getDBDesignReference().designID_);
  size_t id = 0;
//   auto instParameters = instance.initInstParameters(dbInst->getInstParameters().size());
//   for (auto instParameter: dbInst->getInstParameters()) {
//     auto instParameterBuilder = instParameters[id++];
//     dumpInstParameter(instParameterBuilder, instParameter);
//   }
}

void dumpDbModBTerm(DBImplementation::NetComponentReference::Builder& componentReference,
  dbModBTerm* term) {
  auto termRefenceBuilder = componentReference.initTermReference();
  termRefenceBuilder.setTermID(term->getId());
  if (NajaIF::term2busbit_.find(term->getId()) != NajaIF::term2busbit_.end()) {
    assert(NajaIF::term2busbit_[term->getId()].first->getBusPortMember(NajaIF::term2busbit_[term->getId()].second) 
        == term);
    termRefenceBuilder.setBit(NajaIF::term2busbit_[term->getId()].second);
  }
}

void dumpDbModITerm(
  DBImplementation::NetComponentReference::Builder& componentReference,
  dbModITerm* instTerm) {
  auto instTermRefenceBuilder = componentReference.initInstTermReference();
  instTermRefenceBuilder.setInstanceID(instTerm->getParent()->getId());
  auto term = instTerm->getChildModBTerm();
  instTermRefenceBuilder.setTermID(term->getId());
  if (NajaIF::term2busbit_.find(term->getId()) != NajaIF::term2busbit_.end()) {
    assert(NajaIF::term2busbit_[term->getId()].first->getBusPortMember(NajaIF::term2busbit_[term->getId()].second) 
        == term);
    instTermRefenceBuilder.setBit(NajaIF::term2busbit_[term->getId()].second);
  }
}

void dumpDbIterm(
  DBImplementation::NetComponentReference::Builder& componentReference,
  dbITerm* instTerm) {
  auto instTermRefenceBuilder = componentReference.initInstTermReference();
  instTermRefenceBuilder.setInstanceID(instTerm->getInst()->getId());
  auto term = instTerm->getMTerm();
  instTermRefenceBuilder.setTermID(term->getId());
  //dbMaster* master = NajaIF::db_->findMaster(instTerm->getInst()->getMaster());
  // handle bus bits -> TODO: verify that does not exist for dbInst level
}

// void dumpNetComponentReference(
//   DBImplementation::NetComponentReference::Builder& componentReference,
//   const SNLNetComponent* component) {
//   if (auto instTerm = dynamic_cast<const SNLInstTerm*>(component)) {
//     dumpInstTermReference(componentReference, instTerm);
//   } else {
//     auto term = dynamic_cast<const SNLBitTerm*>(component);
//     dumpBitTermReference(componentReference, term);
//   }
// }

void dumpScalarNet(
  DBImplementation::LibraryImplementation::SNLDesignImplementation::Net::Builder& net,
  dbModNet* scalarNet) {
  auto scalarNetBuilder = net.initScalarNet();
  scalarNetBuilder.setId(scalarNet->getId());
  if (std::string(scalarNet->getName()) != "") {
    scalarNetBuilder.setName(std::string(scalarNet->getName()));
  }
  scalarNetBuilder.setType(DBImplementation::LibraryImplementation::SNLDesignImplementation::NetType::STANDARD);
  size_t componentsSize = scalarNet->connectionCount();
  if (componentsSize > 0) {
    auto components = scalarNetBuilder.initComponents(componentsSize);
    size_t id = 0;
    for (auto component: scalarNet->getModITerms()) {
      auto componentRefBuilder = components[id++];
      dumpDbModITerm(componentRefBuilder, component);
    }
    for (auto component: scalarNet->getModBTerms()) {
      auto componentRefBuilder = components[id++];
      dumpDbModBTerm(componentRefBuilder, component);
    }
    for (auto component: scalarNet->getITerms()) {
      auto componentRefBuilder = components[id++];
      dumpDbIterm(componentRefBuilder, component);
    }
  }
}

// void dumpBusNetBit(
//   DBImplementation::LibraryImplementation::SNLDesignImplementation::BusNetBit::Builder& bitBuilder,
//   NLID::Bit bit,
//   bitBuilder.setBit(bit);
//   if (busNetBit) {
//     bitBuilder.setDestroyed(false);
//     bitBuilder.setType(ODBtoCapnPNetType(busNetBit->getType()));
//     size_t componentsSize = busNetBit->getComponents().size();
//     if (componentsSize > 0) {
//       auto components = bitBuilder.initComponents(componentsSize);
//       size_t id = 0;
//       for (auto component: busNetBit->getComponents()) {
//         auto componentRefBuilder = components[id++];
//         dumpNetComponentReference(componentRefBuilder, component);
//       }
//     }
//   } else {
//     bitBuilder.setDestroyed(true);
//   }
// }

// void dumpBusNet(
//   DBImplementation::LibraryImplementation::SNLDesignImplementation::Net::Builder& net,
//   const SNLBusNet* busNet) {
//   auto busNetBuilder = net.initBusNet();
//   busNetBuilder.setId(busNet->getId());
//   if (not busNet->isUnnamed()) {
//     busNetBuilder.setName(busNet->getName().getString());
//   }
//   busNetBuilder.setMsb(busNet->getMSB());
//   busNetBuilder.setLsb(busNet->getLSB());
//   auto bits = busNetBuilder.initBits(busNet->getWidth());
//   size_t id = 0;
//   for (size_t i=0; i<busNet->getWidth(); i++) {
//     NLID::Bit bit = (busNet->getMSB()>busNet->getLSB())?
//       busNet->getMSB()-int(i):busNet->getMSB()+int(i);
//     SNLBusNetBit* busNetBit = busNet->getBit(bit);
//     auto bitBuilder = bits[id++];
//     dumpBusNetBit(bitBuilder, bit, busNetBit);
//   }
// }

void dumpDesignImplementation(
  DBImplementation::LibraryImplementation::SNLDesignImplementation::Builder& designImplementation,
 dbModule* snlDesign) {
  designImplementation.setId(snlDesign->getId());

  size_t id = 0;
  auto instances = designImplementation.initInstances(snlDesign->getInsts().size());
  for (auto instance: snlDesign->getInsts()) {
    auto instanceBuilder = instances[id++];
    dumpInstance(instanceBuilder, instance);
  }

  id = 0;
  auto nets = designImplementation.initNets(snlDesign->getModNets().size());
  for (auto net: snlDesign->getModNets()) {
    auto netBuilder = nets[id++];
    dumpScalarNet(netBuilder, net);
  }
}

void dumpLibraryImplementation(
  DBImplementation::LibraryImplementation::Builder& libraryImplementation,
  odb::dbBlock* block) {
  libraryImplementation.setId(block->getId());
//   auto subLibraries = libraryImplementation.initLibraryImplementations(snlLibrary->getLibraries().size());
//   size_t id = 0;
//   for (auto subLib: snlLibrary->getLibraries()) {
//     auto subLibraryBuilder = subLibraries[id++];
//     dumpLibraryImplementation(subLibraryBuilder, subLib);
//   }
  auto designs = libraryImplementation.initSnlDesignImplementations(block->getModules().size());
  size_t id = 0;
  for (auto design: block->getModules()) {
    auto designImplementationBuilder = designs[id++]; 
    dumpDesignImplementation(designImplementationBuilder, design);
  }
}

void NajaIF::dumpImplementation(odb::dbBlock* block, int fileDescriptor) {
  dumpImplementation(block, fileDescriptor, block->getId());
}

void NajaIF::dumpImplementation(odb::dbBlock* block, int fileDescriptor, uint8_t forceDBID) {
  ::capnp::MallocMessageBuilder message;

  DBImplementation::Builder db = message.initRoot<DBImplementation>();
  db.setId(forceDBID);
  auto libraries = db.initLibraryImplementations(1); // no multiple libraries in OR
  auto libraryImplementationBuilder = libraries[0];
  dumpLibraryImplementation(libraryImplementationBuilder, block);
  writePackedMessageToFd(fileDescriptor, message);
}

void NajaIF::dumpImplementation(odb::dbBlock* block, const std::filesystem::path& implementationPath) {
  int fd = open(
    implementationPath.c_str(),
    O_CREAT | O_WRONLY,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  dumpImplementation(block, fd);
  close(fd);
}

