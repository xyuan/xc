//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  This program derives from OpenSees <http://opensees.berkeley.edu>
//  developed by the  «Pacific earthquake engineering research center».
//
//  Except for the restrictions that may arise from the copyright
//  of the original program (see copyright_opensees.txt)
//  XC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or 
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful, but 
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details. 
//
//
// You should have received a copy of the GNU General Public License 
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//ModelWrapper.cc

#include "ModelWrapper.h"
#include "solution/SoluMethod.h"
#include <solution/analysis/model/AnalysisModel.h>

//Gestor coacciones.
#include <solution/analysis/handler/ConstraintHandler.h>
#include <solution/analysis/handler/LagrangeConstraintHandler.h>
#include <solution/analysis/handler/PenaltyConstraintHandler.h>
#include <solution/analysis/handler/PlainHandler.h>
#include <solution/analysis/handler/TransformationConstraintHandler.h>

//Numerador.
#include "solution/analysis/numberer/DOF_Numberer.h"
#include "solution/analysis/numberer/ParallelNumberer.h"
#include "solution/analysis/numberer/PlainNumberer.h"

#include <utility/actor/objectBroker/FEM_ObjectBroker.h>
#include "utility/matrix/ID.h"

void XC::ModelWrapper::libera_analysis_model(void)
  {
    if(theModel)
      {
        delete theModel;
        theModel= nullptr;
      }
  }

void XC::ModelWrapper::alloc_analysis_model(void)
  {
    libera_analysis_model();
    theModel= new AnalysisModel(this);
  }

void XC::ModelWrapper::copia_analysis_model(AnalysisModel *ptr)
  {
    if(ptr)
      {
        libera_analysis_model();
        theModel= ptr->getCopy();
        theModel->set_owner(this);
      }
    else
      std::cerr << "ModelWrapper::copia_analysis_model; se pasó a null pointer." << std::endl;
  }

void XC::ModelWrapper::libera_constraint_handler(void)
  {
    if(theHandler)
      {
        delete theHandler;
        theHandler= nullptr;
      }
  }

bool XC::ModelWrapper::alloc_constraint_handler(const std::string &nmb)
  {
    libera_constraint_handler();

    if(nmb=="lagrange_constraint_handler")
      theHandler=new LagrangeConstraintHandler(this);
    else if(nmb=="penalty_constraint_handler")
      theHandler=new PenaltyConstraintHandler(this);
    else if(nmb=="plain_handler")
      theHandler=new PlainHandler(this);
    else if(nmb=="transformation_constraint_handler")
      theHandler=new TransformationConstraintHandler(this);
    else
      std::cerr << "Constraint handler: '"
                << nmb << " unknown." << std::endl;

    if(theHandler)
      theHandler->set_owner(this);

    return (theHandler!=nullptr);
  }

void XC::ModelWrapper::copia_constraint_handler(const ConstraintHandler *ptr)
  {
    if(ptr)
      {
        libera_constraint_handler();
        theHandler= ptr->getCopy();
        theHandler->set_owner(this);
      }
    else
      std::cerr << "ModelWrapper::copia_constraint_handler; se pasó a null pointer." << std::endl;
  }

//! @brief Crea un numerador del tipo being passed as parameter.
XC::ConstraintHandler &XC::ModelWrapper::newConstraintHandler(const std::string &nmb)
  {
    alloc_constraint_handler(nmb);
    assert(theHandler);
    return *theHandler;
  }

void XC::ModelWrapper::libera_numerador(void)
  {
    if(theDOFNumberer)
      {
        delete theDOFNumberer;
        theDOFNumberer= nullptr;
      }
  }

bool XC::ModelWrapper::setup_numerador(void)
  {
    bool retval= false;
    if(theDOFNumberer)
      {
        assert(theModel);
        retval= true;
      }
    else
      std::cerr << "ModelWrapper::setup_numerador; el numerador no está definido."
                << std::endl;
    return retval;
  }

bool XC::ModelWrapper::alloc_numerador(const std::string &nmb)
  {
    libera_numerador();

    if(nmb=="default_numberer")
      theDOFNumberer= new DOF_Numberer(this);
    else if(nmb=="plain_numberer")
      theDOFNumberer= new PlainNumberer(this);
    else if(nmb=="parallel_numberer")
      theDOFNumberer= new ParallelNumberer(this);
    else
      std::cerr << "ModelWrapper::alloc_numerador. ERROR; no se pudo"
                << " interpretar la cadena: '" << nmb << "'\n";
    return setup_numerador();
  }

void XC::ModelWrapper::copia_numerador(const DOF_Numberer *dn)
  {
    if(dn)
      {
        libera_numerador();
        theDOFNumberer= dn->getCopy();
        theDOFNumberer->set_owner(this);
        setup_numerador();
      }
    else
     std::cerr << "ModelWrapper::copia_numerador; se pasó a null pointer." << std::endl;
  }

//! @brief Crea un numerador del tipo being passed as parameter.
XC::DOF_Numberer &XC::ModelWrapper::newNumberer(const std::string &nmb)
  {
    alloc_numerador(nmb);
    assert(theDOFNumberer);
    return *theDOFNumberer;
  }

void XC::ModelWrapper::libera(void)
  {
    libera_analysis_model();
    libera_constraint_handler();
    libera_numerador();
  }

void XC::ModelWrapper::copia(const ModelWrapper &otro)
  {
    if(otro.theModel) copia_analysis_model(otro.theModel);
    if(otro.theHandler) copia_constraint_handler(otro.theHandler);
    if(otro.theDOFNumberer) copia_numerador(otro.theDOFNumberer);
  }

XC::SoluMethod *XC::ModelWrapper::getSoluMethod(void)
  { return dynamic_cast<SoluMethod *>(Owner()); }

const XC::SoluMethod *XC::ModelWrapper::getSoluMethod(void) const
  { return dynamic_cast<const SoluMethod *>(Owner()); }


//! @brief Default constructor.
XC::ModelWrapper::ModelWrapper(SoluMethod *owr)
  : EntCmd(owr), theModel(nullptr), theHandler(nullptr), theDOFNumberer(nullptr)
  { alloc_analysis_model(); }

//! @brief Constructor de copia.
XC::ModelWrapper::ModelWrapper(const ModelWrapper &otro)
  : EntCmd(otro), theModel(nullptr), theHandler(nullptr),theDOFNumberer(nullptr)
  { copia(otro); }

//! @brief Operador asignación.
XC::ModelWrapper &XC::ModelWrapper::operator=(const ModelWrapper &otro)
  {
    EntCmd::operator=(otro);
    copia(otro);
    return *this;
  }

//! @brief Destructor.
XC::ModelWrapper::~ModelWrapper(void)
  { libera(); }

void XC::ModelWrapper::clearAll(void)
  { libera(); }

//! @brief Returns a pointer to the domain.
XC::Domain *XC::ModelWrapper::getDomainPtr(void)
  {
    SoluMethod *sm= getSoluMethod();
    assert(sm);
    return sm->getDomainPtr();
  }

//! @brief Returns a pointer to the domain.
const XC::Domain *XC::ModelWrapper::getDomainPtr(void) const
  {
    const SoluMethod *sm= getSoluMethod();
    assert(sm);
    return sm->getDomainPtr();
  }

//! @brief Returns a pointer to the integrator.
XC::Integrator *XC::ModelWrapper::getIntegratorPtr(void)
  {
    SoluMethod *sm= getSoluMethod();
    assert(sm);
    return sm->getIntegratorPtr();
  }

//! @brief Returns a pointer to the integrator.
const XC::Integrator *XC::ModelWrapper::getIntegratorPtr(void) const
  {
    const SoluMethod *sm= getSoluMethod();
    assert(sm);
    return sm->getIntegratorPtr();
  }

void XC::ModelWrapper::brokeConstraintHandler(const CommParameters &cp,const ID &data)
  {
    theHandler= cp.brokeConstraintHandler(data(0));
    theHandler->set_owner(this);
  }

void XC::ModelWrapper::brokeNumberer(const CommParameters &cp,const ID &data)
  { 
    theDOFNumberer= cp.brokeNumberer(data(1));
    theDOFNumberer->set_owner(this);
  }

void XC::ModelWrapper::brokeAnalysisModel(const CommParameters &cp,const ID &data)
  {
    theModel= cp.brokeAnalysisModel(data(2));
    theModel->set_owner(this);
  }

//! @brief Sets the renumerador to use in the analysis.
int XC::ModelWrapper::setNumberer(DOF_Numberer &theNewNumberer) 
  {
    copia_numerador(&theNewNumberer);
    return 0;
  }

//! @brief Verifica que los pointers no sean nulos.
bool XC::ModelWrapper::CheckPointers(void)
  {
    if(!theModel)
      {
        std::cerr << "ModelWrapper::check_pointers; error, no se ha establecido el analysis model." << std::endl;
        return false;
      }
    if(!theHandler)
      {
        std::cerr << "ModelWrapper::check_pointers; error, no se ha establecido el gestor de coacciones." << std::endl;
        return false;
      }
    if(!theDOFNumberer)
      {
        std::cerr << "ModelWrapper::check_pointers; error, no se ha estableciodo el renumerador." << std::endl;
        return false;
      }
    return true;
  }
