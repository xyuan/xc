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
//RegionContainer.cc

#include "RegionContainer.h"
#include <material/section/repres/geom_section/region/RgSccQuad.h>
#include <material/section/repres/geom_section/region/RgSccCirc.h>
#include <material/section/repres/geom_section/region/RgSccPoligono.h>
#include "material/uniaxial/UniaxialMaterial.h" 
#include "preprocessor/prep_handlers/MaterialHandler.h"
#include "utility/matrix/Vector.h"
#include "xc_utils/src/geom/pos_vec/Dir2d.h"
#include "xc_utils/src/geom/pos_vec/Vector2d.h"
#include "xc_utils/src/geom/pos_vec/Pos2d.h"
#include "xc_utils/src/geom/sis_ref/Ref2d2d.h"



#include "xc_utils/src/geom/d2/BND2d.h"
#include "xc_utils/src/geom/d2/poligonos2d/Poligono2d.h"
#include "xc_utils/src/geom/d2/poligonos2d/bool_op_poligono2d.h"

//! @brief Liberta todas las pociciones.
void XC::RegionContainer::free_mem(void)
  {
    for(iterator i=begin();i!=end();i++)
      delete *i;
    erase(begin(),end());
  }

//! @brief Copy the regions from another container.
void XC::RegionContainer::copia(const RegionContainer &otra)
  {
    free_mem();
    for(const_iterator i=otra.begin();i!=otra.end();i++)
      push_back(*(*i)->getCopy());
  }

//! @brief Constructor.
XC::RegionContainer::RegionContainer(MaterialHandler *ml)
  : l_reg(), material_handler(ml) {}

//! @brief Copy constructor.
XC::RegionContainer::RegionContainer(const RegionContainer  &otro)
  : l_reg(), material_handler(otro.material_handler)
  { copia(otro); }

//! @brief Assignment operator.
XC::RegionContainer &XC::RegionContainer::operator=(const RegionContainer &otro)
  {
    SectionMassProperties::operator=(otro);
    material_handler= otro.material_handler;
    copia(otro);
    return *this;
  }

//! @brief Aggregates a new quadrilateral region.
XC::RgSccQuad *XC::RegionContainer::newQuadRegion(const std::string &cod_mat)
  {
    Material *mat= material_handler->find_ptr(cod_mat);
    if(!mat)
      std::cerr << getClassName() << __FUNCTION__
	        << "; warning!, material: '"
                << cod_mat << "' not found. Material definition pending.\n";
    RgSccQuad tmp(mat);
    RgSccQuad *ptr= dynamic_cast<XC::RgSccQuad *>(push_back(tmp));
    ptr->set_owner(this);
    return ptr;
  }

//! @brief Aggregates a new circularl region.
XC::RgSccCirc *XC::RegionContainer::newCircularRegion(const std::string &cod_mat)
  {
    Material *mat= material_handler->find_ptr(cod_mat);
    if(!mat)
      std::cerr << getClassName() << __FUNCTION__
	        << "; warning!, material: '"
                << cod_mat << "' not found. Material definition pending.\n";
    RgSccCirc tmp(mat);
    RgSccCirc *ptr= dynamic_cast<XC::RgSccCirc *>(push_back(tmp));
    ptr->set_owner(this);
    return ptr;
  }

//! @brief Destructor.
XC::RegionContainer::~RegionContainer(void)
  { free_mem(); }

//! @brief Erases all regions.
void XC::RegionContainer::clear(void)
  { free_mem(); }

//! @brief Adds a region to the container.
XC::RegionSecc *XC::RegionContainer::push_back(const RegionSecc &reg)
  {
    RegionSecc *tmp= reg.getCopy();
    l_reg::push_back(tmp);
    return tmp;
  }

XC::RegionContainer::const_iterator XC::RegionContainer::begin(void) const
  { return l_reg::begin(); }
XC::RegionContainer::const_iterator XC::RegionContainer::end(void) const
  { return l_reg::end(); }
XC::RegionContainer::iterator XC::RegionContainer::begin(void)
  { return l_reg::begin(); }
XC::RegionContainer::iterator XC::RegionContainer::end(void)
  { return l_reg::end(); }

//! @brief Returns the número total de celdas.
size_t XC::RegionContainer::getNumCells(void) const
  {
    size_t ncells= 0;
    for(const_iterator i=begin();i!=end();i++)
      {
        const RegionSecc *ptr= *i;
        ncells+= ptr->getNumCells();
      }
    return ncells;
  }

//! @brief Returns a list with the regions contours.
std::list<Poligono2d> XC::RegionContainer::getRegionsContours(void) const
  {
    std::list<Poligono2d> retval;
    for(const_iterator i= begin();i!=end();i++)
      retval.push_back((*i)->getPolygon());
    return retval;
  }

//! @brief Return the regions contours.
std::list<Poligono2d> XC::RegionContainer::getContours(void) const
  {
    std::list<Poligono2d> retval= join(getRegionsContours());
    return retval;
  }

BND2d XC::RegionContainer::getBnd(void) const
  {
    BND2d retval;
    if(!empty())
      {
        const_iterator i= begin();
        retval= (*i)->getPolygon().Bnd();
        i++;
        for(;i!=end();i++)
          retval+= (*i)->getPolygon().Bnd();
      }
    else
      std::cerr << getClassName() << "::" << __FUNCTION__
	        << "; region container is empty. Boundary has no sense."
		<< std::endl;
    return retval;
  }

XC::RegionContainer XC::RegionContainer::Intersection(const Semiplano2d &sp) const
  {
    RegionContainer retval(material_handler);
    for(const_iterator i= begin();i!=end();i++)
      retval.push_back((*i)->Intersection(sp));
    return retval;
  }

//! @brief Returns the regions area.
double XC::RegionContainer::getAreaGrossSection(void) const
  {
    double retval= 0.0;
    for(const_iterator i= begin();i!=end();i++)
      retval+= (*i)->Area();
    return retval;
  }

//! @brief Returns the centro de gravedad of the gross cross-section.
XC::Vector XC::RegionContainer::getCdgGrossSection(void) const
  {
    Vector retval(2);
    double weight= 0.0;
    double divisor= 0.0;
    for(const_iterator i= begin();i!=end();i++)
      {
        weight= (*i)->Area();
        if(weight>0)
          {
            retval+= weight*(*i)->Cdg();
            divisor+= weight;
          }
        else
          std::cerr << getClassName() << __FUNCTION__
		    << "; region: "
                    << *i << " has zero or negative weight." 
                    << std::endl;
      }
    retval/= divisor;
    return retval;
  }

//! @brief Returns the moment of inertia of the gross cross-section with respect to the axis parallel to y passing through the centroid.
double XC::RegionContainer::getIyGrossSection(void) const
  {
    double retval= 0.0;
    double d= 0.0;
    const double zCdg= getCdgGrossSection()[1];
    for(const_iterator i= begin();i!=end();i++)
      {
        d= (*i)->Cdg()[1]-zCdg;
        retval+= (*i)->Iy()+(*i)->Area()*sqr(d);
      }
    return retval;
  }

//! @brief Returns the moment of inertia of the gross cross-section with respect to the axis paralelo al z por el centroid.
double XC::RegionContainer::getIzGrossSection(void) const
  {
    double retval= 0.0;
    double d= 0.0;
    const double yCdg= getCdgGrossSection()[0];
    for(const_iterator i= begin();i!=end();i++)
      {
        d= (*i)->Cdg()[0]-yCdg;
        retval+= (*i)->Iz()+(*i)->Area()*sqr(d);
      }
    return retval;
  }

//! @brief Returns the producto de inercia of the gross cross-section respecto a los axis parallel to the y y al z por el centroid.
double XC::RegionContainer::getPyzGrossSection(void) const
  {
    double retval= 0.0;
    double d2= 0.0;
    const Vector cooCdg= getCdgGrossSection();
    const double zCdg= cooCdg[1];
    const double yCdg= cooCdg[0];
    for(const_iterator i= begin();i!=end();i++)
      {
        d2= ((*i)->Cdg()[0]-yCdg)*((*i)->Cdg()[1]-zCdg);
        retval+= (*i)->Pyz()+(*i)->Area()*d2;
      }
    return retval;
  }

//! @brief Returns the homogenized area of the regions.
double XC::RegionContainer::getAreaHomogenizedSection(const double &E0) const
  {
    if(fabs(E0)<1e-6)
      std::clog << "homogenization reference modulus too small; E0= "
		<< E0 << std::endl; 
    double retval= 0.0;
    double n= 0.0;
    for(const_iterator i= begin();i!=end();i++)
      {
        const UniaxialMaterial *mat= dynamic_cast<const UniaxialMaterial *>((*i)->getMaterialPtr());
        if(mat)
          {
            n= mat->getTangent()/E0;
            retval+= n*(*i)->Area();
          }
        else
	  std::cerr << getClassName() << __FUNCTION__
		    << "; can't get region material." << std::endl; 
      }
    return retval;
  }

XC::Vector XC::RegionContainer::getCdgHomogenizedSection(const double &E0) const
  {
    if(fabs(E0)<1e-6)
      std::clog << "homogenization reference modulus too small; E0= "
		<< E0 << std::endl; 
    Vector retval(2);
    double weight= 0.0;
    double divisor= 0.0;
    for(const_iterator i= begin();i!=end();i++)
      {
        const UniaxialMaterial *mat= dynamic_cast<const UniaxialMaterial *>((*i)->getMaterialPtr());
        if(mat)
          {
            weight= mat->getTangent()/E0*(*i)->Area();
            if(weight>0)
              {
                retval+= weight*(*i)->Cdg();
                divisor+= weight;
              }
            else
	      std::cerr << getClassName() << __FUNCTION__
			<< "; region: "
                        << *i << " weight is zero or negative." 
                        << std::endl;
          }
        else
	  std::cerr << getClassName() << __FUNCTION__
		    << "; can't get region material." << std::endl; 
      }
    retval/= divisor;
    return retval;
  }

//! @brief Returns homogenized moment of inertia of the cross-section with respect to the axis parallel to y passing through the centroid.
//! @param E0: Reference elastic modulus.
double XC::RegionContainer::getIyHomogenizedSection(const double &E0) const
  {
    if(fabs(E0)<1e-6)
      std::clog << "homogenization reference modulus too small; E0= "
		<< E0 << std::endl; 
    double retval= 0.0;
    double n= 0.0;
    double d= 0.0;
    const double zCdg= getCdgHomogenizedSection(E0)[1];
    for(const_iterator i= begin();i!=end();i++)
      {
        const UniaxialMaterial *mat= dynamic_cast<const UniaxialMaterial *>((*i)->getMaterialPtr());
        if(mat)
          {
            n= mat->getTangent()/E0;
            d= (*i)->Cdg()[1]-zCdg;
            retval+= n*((*i)->Iy()+(*i)->Area()*sqr(d));
          }
        else
	  std::cerr << getClassName() << __FUNCTION__
	            << "; can't get section material." << std::endl; 
      }
    return retval;
  }

//! @brief Returns homogenized moment of inertia of the cross-section with respect to the axis parallel to z passing through the centroid.
//! @param E0: Reference elastic modulus.
double XC::RegionContainer::getIzHomogenizedSection(const double &E0) const
  {
    if(fabs(E0)<1e-6)
      std::clog << "homogenization reference modulus too small; E0= "
		<< E0 << std::endl; 
    double retval= 0.0;
    double n= 0.0;
    double d= 0.0;
    const double yCdg= getCdgHomogenizedSection(E0)[0];
    for(const_iterator i= begin();i!=end();i++)
      {
        const UniaxialMaterial *mat= dynamic_cast<const UniaxialMaterial *>((*i)->getMaterialPtr());
        if(mat)
          {
            n= mat->getTangent()/E0;
            d= (*i)->Cdg()[0]-yCdg;
            retval+= n*((*i)->Iz()+(*i)->Area()*sqr(d));
          }
        else
	  std::cerr << getClassName() << __FUNCTION__
	            << "; can't get region material." << std::endl; 
      }
    return retval;
  }

//! @brief Returns homogenized product of inertia of the cross-section with respect to the axis parallel to y and z passing through the centroid.
//! @param E0: Reference elastic modulus.
double XC::RegionContainer::getPyzHomogenizedSection(const double &E0) const
  {
    if(fabs(E0)<1e-6)
      std::clog << "homogenization reference modulus too small; E0= "
		<< E0 << std::endl; 
    double retval= 0.0;
    double n= 0.0;
    double d2= 0.0;
    const Vector cooCdg= getCdgHomogenizedSection(E0);
    const double zCdg= cooCdg[1];
    const double yCdg= cooCdg[0];
    for(const_iterator i= begin();i!=end();i++)
      {
        const UniaxialMaterial *mat= dynamic_cast<const UniaxialMaterial *>((*i)->getMaterialPtr());
        if(mat)
          {
            n= mat->getTangent()/E0;
            d2= ((*i)->Cdg()[0]-yCdg)*((*i)->Cdg()[1]-zCdg);
            retval+= n*((*i)->Pyz()+(*i)->Area()*d2);
          }
        else
	  std::cerr << getClassName() << __FUNCTION__
	            << "; can't get region material." << std::endl; 
      }
    return retval;
  }

//! @brief Print stuff.
void XC::RegionContainer::Print(std::ostream &os) const
  {
    for(const_iterator i= begin();i!=end();i++)
      (*i)->Print(os);
  }

std::ostream &XC::operator<<(std::ostream &os,const RegionContainer &lr)
  {
    lr.Print(os);
    return os;
  }