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
//LineaBase.cc

#include "LineaBase.h"
#include "preprocessor/Preprocessor.h"
#include "Pnt.h"
#include "xc_utils/src/geom/d3/BND3d.h"
#include "xc_utils/src/geom/pos_vec/MatrizPos3d.h"
#include "utility/matrix/util_matrix.h"

#include "domain/mesh/node/Node.h"
#include "domain/mesh/element/Element.h"
#include "med.h"

//! @brief Constructor.
XC::LineaBase::LineaBase(Preprocessor *m,const size_t &nd)
  : Edge(m,nd), p1(nullptr), p2(nullptr) {}

//! @brief Constructor.
XC::LineaBase::LineaBase(const std::string &nombre,Preprocessor *m,const size_t &nd)
  : Edge(nombre,m,nd), p1(nullptr), p2(nullptr) {}

//! @brief Returns a constant pointer to start point.
const XC::Pnt *XC::LineaBase::P1(void) const
  { return p1; }
//! @brief Returns a constant pointer to end point.
const XC::Pnt *XC::LineaBase::P2(void) const
  { return p2; }

//! @brief Returns the vértice de índice i.
const XC::Pnt *XC::LineaBase::GetVertice(const size_t &i) const
  {
    if(i==1)
      return p1;
    else
      return p2;
  }

//! @brief Checks that the points are defined.
bool XC::LineaBase::check_points(void) const
  {
    bool retval= false;
    if(p1 && p2)
      retval= true;
    else
      std::cerr << nombre_clase() << "::" << __FUNCTION__
	        << "; entity: '" << GetNombre()
                << " is not defined." << std::endl;
    return retval;
  }

//! @brief Asigna el vértice de índice i.
void XC::LineaBase::SetVertice(const size_t &i,Pnt *p)
  {
    if((i==1) && (p1!=p))
      {
        if(p1) p1->borra_linea(this);
        p1= p;
      }
    else if(p2!=p)
      {
        if(p2) p2->borra_linea(this);
        p2= p;
      }
    actualiza_topologia();
  }

//! @brief Asigna el vértice de índice i.
void XC::LineaBase::SetVertice(const size_t &i,const size_t &id_punto)
  {
    SetVertice(i,BuscaPnt(id_punto));
    if(!GetVertice(i))
      std::cerr << "LineaBase; no se encontró el punto: '" 
                << id_punto << "' \n";
  }

//! @brief Asigna los extremos de la línea.
void XC::LineaBase::SetVertices(Pnt *pA,Pnt *pB)
  {
    if(pA==pB)
      std::cerr << "¡Ojo!, LineaBase::SetVertices; "
                << "se asigna el mismo punto a ambos extremos de la linea: '"
                << GetNombre() << "'." << std::endl;
    SetVertice(1,pA);
    SetVertice(2,pB);
    if(p1==p2)
      std::cerr << "LineaBase::SetVertices; ¡ojo!, la linea: '" << GetNombre()
                << "' tiene ambos extremos en el mismo punto."
                << std::endl; 
    actualiza_topologia();
  }
