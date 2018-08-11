//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
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
//MEDMeshing

#include "MEDMeshing.h"
#include "MEDDblFieldInfo.h"
#include "MEDIntFieldInfo.h"
#include "MEDGaussModel.h"


#include "preprocessor/set_mgmt/MapSet.h"
#include "post_process/MapFields.h"
#include "preprocessor/set_mgmt/Set.h"
#include "domain/domain/Domain.h"
#include "domain/mesh/element/Element.h"
#include "domain/mesh/element/utils/gauss_models/GaussModel.h"
#include "domain/mesh/node/Node.h"
#include "FEProblem.h"

const std::string XC::MEDMeshing::str_node_group= "_nodes";
const std::string XC::MEDMeshing::str_element_group= "_elements";

// @brief Returns med mesh to export.
XC::MEDMeshing::MEDMeshing(const FEProblem &prb) 
  : sets(prb.getPreprocessor().get_sets()), fields(prb.getFields()), vertices(prb.getDomain()->getMesh()), cells(prb.getDomain()->getMesh(),vertices.getMapIndices())
  {}

//! Constructor.
XC::MEDMeshing::MEDMeshing(const Mesh &mesh,const MapSet &s, const MapFields &f)
  : sets(s), fields(f), vertices(mesh), cells(mesh,vertices.getMapIndices()) {} 

//! Returns a reference to the MEDMEM mesh.
MEDMEM::MESHING &XC::MEDMeshing::getMEDMesh(void) const
  { return mesh; }

//! Erases la mesh.
void XC::MEDMeshing::clear(void)
  {
    vertices.clear();
    cells.clear();
    med_groups.clear();
    for(std::deque<MEDFieldInfo *>::iterator i= med_fields.begin();i!=med_fields.end();i++)
      delete *i;
    med_fields.clear();
    //mesh= MEDMEM::MESHING(); //Not working 2012/10/03
  }
//! Constructor.
XC::MEDMeshing::~MEDMeshing(void)
  { clear(); }

const XC::MEDMapIndices &XC::MEDMeshing::getMapIndicesVertices(void) const
  { return vertices.getMapIndices(); }

const XC::MEDMapIndices &XC::MEDMeshing::getMapIndicesCeldas(void) const
  { return cells.getMapIndices(); }

//! @brief Define the MEDMEM group corresponding to the XC ones.
void XC::MEDMeshing::defineMEDGroups(void)
  {
    for(MapSet::const_iterator i= sets.begin();i!=sets.end();i++)
      {
        const Set *set= dynamic_cast<const Set *>(i->second);
        if(set)
          {
            const std::string nmb= set->getName();
            const size_t numNodes= set->getNumberOfNodes();
            const size_t numElements= set->getNumberOfElements();
            if((numElements==0) && (numNodes==0))
              continue;
            else if((numElements==0) || (numNodes==0))
              med_groups.push_back(MEDGroupInfo(this,*set));
            else
              {
                Set set_nodes(*set);
                set_nodes.clearElements();
                set_nodes.setName(nmb+str_node_group);
                med_groups.push_back(MEDGroupInfo(this,set_nodes));
                Set set_elements(*set);
                set_elements.clearNodes();
                set_elements.setName(nmb+str_element_group);
                med_groups.push_back(MEDGroupInfo(this,set_elements));
              }
          }
      }
  }

XC::MEDGroupInfo *XC::MEDMeshing::getGroupInfo(const std::string &nmb) const
  {
    MEDGroupInfo *retval= nullptr;
    std::deque<MEDGroupInfo>::iterator i= med_groups.begin();
    for(;i!=med_groups.end();i++)
      if(i->getName()==nmb)
        break;
    if(i!=med_groups.end())
      retval= &(*i);
    return retval;
  }

XC::MEDGroupInfo *XC::MEDMeshing::getGroupInfo(const Set &set,const FieldInfo &field) const
  {
    MEDGroupInfo *retval= nullptr;
    std::string nmb_group= "";
    if(field.isDefinedOnNodes())
      nmb_group= set.getName()+str_node_group;
    else if(field.isDefinedOnElements())
      nmb_group= set.getName()+str_element_group;
    retval= getGroupInfo(nmb_group);
    if(!retval)
      std::cerr << getClassName() << "::" << __FUNCTION__
	        << "; group: '"
                << nmb_group
		<< "' not found." << std::endl;
    return retval;  
  }

//! @brief Set the integration Gauss model for the elements on the set.
//! @param set: element set.
void XC::MEDMeshing::defineMEDGaussModels(const Set &set,MEDFieldInfo &med_fi) const
  {
    med_fi.defineGaussModels(set);
  }

//! @brief Defines a field over an element set.
void XC::MEDMeshing::defineMEDDblField(const Set &set,const FieldInfo &fi,MEDGroupInfo *group) const
  {
    MEDDblFieldInfo *med_fi= new MEDDblFieldInfo(fi,group);
    med_fields.push_back(med_fi);
    med_fi->to_med();
    if(fi.isDefinedOnNodes())
      { med_fi->populateOnNodes(set,fi); }
    else if(fi.isDefinedOnElements())
      {
        if(fi.isDefinedOnGaussPoints())
          {
            defineMEDGaussModels(set,*med_fi);
            med_fi->populateOnGaussPoints(set,fi);
          }
        else
          med_fi->populateOnElements(set,fi);
      }
  }

//! @brief Defines a field over a set.
void XC::MEDMeshing::defineMEDIntField(const Set &set,const FieldInfo &fi,MEDGroupInfo *group) const
  {
    MEDIntFieldInfo *med_fi= new MEDIntFieldInfo(fi,group);
    med_fields.push_back(med_fi);
    med_fi->to_med();
    if(fi.isDefinedOnNodes())
      { med_fi->populateOnNodes(set,fi); }
    else if(fi.isDefinedOnElements())
      {
        if(fi.isDefinedOnGaussPoints())
          {
            defineMEDGaussModels(set,*med_fi);
            med_fi->populateOnGaussPoints(set,fi);
          }
        else
          med_fi->populateOnElements(set,fi);
      }
  }

//! @brief Define los campos de MEDMEM correspondientes a los de XC.
void XC::MEDMeshing::defineMEDFields(void) const
  {
    for(MapFields::const_iterator fieldIter= fields.begin();fieldIter!=fields.end();fieldIter++)
      {
        const FieldInfo &fi= *fieldIter;
        const Set *set= dynamic_cast<const Set *>(sets.busca_set(fi.getSetName()));
        if(set)
          {
            MEDGroupInfo *group= getGroupInfo(*set,fi);
            if(group)
              {
                const std::string type_of_components= fi.getComponentsType();
                if(type_of_components=="double")
                  { defineMEDDblField(*set,fi,group); }
                else if(type_of_components=="int")
                  { defineMEDIntField(*set,fi,group); }
                else
                  std::cerr << getClassName() << "::" << __FUNCTION__
			    << "; unknown type: '" << type_of_components
                            << "'." << std::endl;
	      }
          }
        else
	  std::cerr << "MEDMeshing::defineMEDFields; set: '" 
                    << fi.getSetName() << "' for the field : '"
                    << fi.getName() << "' not found." << std::endl;
      }
  }

//! @brief Dumps vertices and cells definition onto
//! mesh MED
void XC::MEDMeshing::to_med(void)
  {
    vertices.to_med(mesh);
    cells.to_med(mesh);
    defineMEDGroups();
    for(std::deque<MEDGroupInfo>::const_iterator i=med_groups.begin();i!=med_groups.end();i++)
      i->to_med();
    defineMEDFields();
  }

void XC::MEDMeshing::write(const std::string &fileName)
  {
    to_med();
    const int id= mesh.addDriver(MEDMEM::MED_DRIVER,fileName,mesh.getName());
    mesh.write(id);
    for(std::deque<MEDFieldInfo *>::iterator i= med_fields.begin();i!=med_fields.end();i++)
      (*i)->write(fileName);
  }

