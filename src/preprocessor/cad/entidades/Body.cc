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
//Body.cc

#include "Body.h"
#include "Pnt.h"
#include "xc_basic/src/matrices/m_int.h"
#include "xc_utils/src/geom/d3/BND3d.h"
#include "Face.h"

#include "domain/mesh/node/Node.h"
#include "domain/mesh/element/Element.h"
#include "med.h"
#include "preprocessor/Preprocessor.h"
#include "preprocessor/set_mgmt/Set.h"
#include "xc_utils/src/geom/pos_vec/MatrizPos3d.h"

//! @brief Constructor
XC::SecuenciaLados::SecuenciaLados(const size_t primero,const bool &directo)
  : dirt(directo)
  {
    if(dirt)
      {
        l1= (primero-1)%4+1;
        l2= (primero)%4+1;
        l3= (primero+1)%4+1;
        l4= (primero+2)%4+1;
      }
    else
      {
        l1= (primero+3)%4+1;
        l2= (primero+2)%4+1;
        l3= (primero+1)%4+1;
        l4= (primero)%4+1;
      }
  }

//! @brief Constructor.
XC::Body::Cara::Cara(XC::Face *ptr,const size_t &p,const bool &d)
  : superficie(ptr), sec_lados(p,d) {}

//! @brief Returns a pointer to the la superficie que limita el sólido.
XC::Face *XC::Body::Cara::Superficie(void)
  { return superficie; }
//! @brief Returns a pointer to the la superficie que limita el sólido.
const XC::Face *XC::Body::Cara::Superficie(void) const
  { return superficie; }
//! @brief Asigna la superficie que limita el sólido.
void XC::Body::Cara::SetSurf(XC::Face *s)
  { superficie= s; }
//! @brief Devuelve el nombre de la superficie que limita el sólido.
const std::string &XC::Body::Cara::GetNombre(void) const
  { return superficie->GetNombre(); }
//! @brief Devuelve verdadero si la superficie no existe.
bool XC::Body::Cara::Vacia(void) const
  { return (superficie==nullptr); }
//! @brief Devuelve el número de líneas de la superficie.
size_t XC::Body::Cara::NumLineas(void) const
  { return superficie->NumEdges(); }
//! @brief Devuelve el número de líneas de la superficie.
size_t XC::Body::Cara::NumVertices(void) const
  { return superficie->NumVertices(); }

//! @brief Devuelve the pointer al lado de la cara, cuyo índice se pasa como parámetro.
const XC::CmbEdge::Lado *XC::Body::Cara::GetLado(const size_t &i) const
  {
    if(!superficie) return nullptr;
    const CmbEdge::Lado *retval(nullptr);
    const size_t idx= (i-1)%4+1;
    switch(idx)
      {
        case 1:
          retval= superficie->GetLado(sec_lados.l1);
          break;
        case 2:
          retval= superficie->GetLado(sec_lados.l2);
          break;
        case 3:
          retval= superficie->GetLado(sec_lados.l3);
          break;
        case 4:
          retval= superficie->GetLado(sec_lados.l4);
          break;
        default:
          retval= nullptr;
          break;
      }
    return retval;
  }

//! @brief Devuelve the pointer al lado de la cara, cuyo índice se pasa como parámetro.
XC::CmbEdge::Lado *XC::Body::Cara::GetLado(const size_t &i)
  { return const_cast<CmbEdge::Lado *>(static_cast<const Cara &>(*this).GetLado(i)); }

//! @brief Devuelve the pointer al vértice de la cara, cuyo índice se pasa como parámetro.
const XC::Pnt *XC::Body::Cara::GetVertice(const size_t &i) const
  {
    const CmbEdge::Lado *l= GetLado(i);
    if(l)
      {
        if(sec_lados.Directo())
          return l->P1();
        else
          return l->P2();
      }
    return nullptr;
  }

//! @brief Devuelve the pointer al vértice de la cara, cuyo índice se pasa como parámetro.
XC::Pnt *XC::Body::Cara::GetVertice(const size_t &i)
  { return const_cast<Pnt *>(static_cast<const Cara &>(*this).GetVertice(i)); }

//! @brief Devuelve the pointer al nodo de la cara, cuyos índices se pasan como parámetro.
XC::Node *XC::Body::Cara::GetNodo(const size_t &i,const size_t &j)
  {
    assert(superficie);
    return superficie->GetNodo(i,j);
  }

//! @brief Devuelve la posiciones para los nodos de la cara.
MatrizPos3d XC::Body::Cara::get_posiciones(void) const
  {
    if(!superficie)
      {
        std::cerr << "The pointer a superficie es nulo." << std::endl;
        return MatrizPos3d(); 
      }
    const int numEdges= superficie->NumEdges();
    if(numEdges!=4)
      {
        std::cerr << "Can't mesh " << numEdges
	          << " edges surfaces." << std::endl;
        return MatrizPos3d();
      }
    const CmbEdge::Lado *l1= GetLado(1);
    const CmbEdge::Lado *l2= GetLado(2);
    const CmbEdge::Lado *l3= GetLado(3);
    const CmbEdge::Lado *l4= GetLado(4);
    //Filas de puntos cuasi paralelas a l2 y l4 y con indice creciente desde l4 hasta l2.
    //Columnas de puntos cuasi paralelas a l1 y l3 y con indice creciente desde l1 hasta l3.
    if(l1->NDiv()!=l3->NDiv())
      {
        std::cerr << "Lines 1 and 3 have different number of divisions. Can't generate mesh." << std::endl;
        return MatrizPos3d();
      }
    if(l2->NDiv()!=l4->NDiv())
      {
        std::cerr << "Lines 2 and 4 have different number of divisions. Can't generate mesh." << std::endl;
        return MatrizPos3d();
      }
    MatrizPos3d ptos_l1= l1->GetPosNodosDir();
    MatrizPos3d ptos_l2= l2->GetPosNodosDir();
    MatrizPos3d ptos_l3= l3->GetPosNodosInv(); //Ordenados al revés.
    MatrizPos3d ptos_l4= l4->GetPosNodosInv(); //Ordenados al revés.
    return MatrizPos3d(ptos_l1,ptos_l2,ptos_l3,ptos_l4);
  }

//! @brief Comprueba el número de divisiones de la superficie.
bool XC::Body::Cara::checkNDivs(void) const
  { return superficie->checkNDivs(); }

//! @brief Lanza la creación de nodos de las cara.
void XC::Body::Cara::crea_nodos(void)
  {
    if(superficie)
      superficie->crea_nodos();
    else
      std::cerr << ":Body::Cara::crea_nodos; the pointer "
                << "a la superficie es nulo." << std::endl;
  }

//! @brief Constructor.
XC::Body::Body(Preprocessor *m,const std::string &nombre)
  : EntMdlr(nombre,0,m) {}

//! @brief Devuelve el BND del objeto.
BND3d XC::Body::Bnd(void) const
  { 
    BND3d retval;
    const size_t nv= NumVertices();
    if(nv<1) //El conjunto está vacío.
      {
	std::cerr << "Body::Bnd(); la polilinea está vacia." << std::endl;
        return retval;
      }
    if(nv<2)
      {
	std::cerr << "Body::Bnd(); la polilinea sólo tiene un punto." << std::endl;
        retval= BND3d(GetVertice(1)->GetPos(),GetVertice(1)->GetPos());
        return retval;
      }
    retval= BND3d(GetVertice(1)->GetPos(),GetVertice(2)->GetPos());
    for(size_t i=3;i<=nv;i++)
      retval+= GetVertice(i)->GetPos();
    return retval;
  }

//! @brief Devuelve la lista de sólidos en contacto con la superficie que
//! pasa como parámetro.
std::set<const XC::Body *> XC::GetCuerposTocan(const Face &s)
  { return s.CuerposTocan(); }

//! @brief Devuelve los conjuntos a los que pertenece este cuerpo.
std::set<XC::SetBase *> XC::Body::get_sets(void) const
  {
    std::set<SetBase *> retval;
    const Preprocessor *preprocessor= GetPreprocessor();
    if(preprocessor)
      {
        MapSet &sets= const_cast<MapSet &>(preprocessor->get_sets());
        retval= sets.get_sets(this);
      }
    else
      std::cerr << "Body::get_sets; no se ha definido el preprocesador." << std::endl;
    return retval;
  }

//! @brief Agrega el cuerpo a los conjuntos que se pasan como parámetro.
void XC::Body::add_to_sets(std::set<SetBase *> &sets)
  {
    for(std::set<SetBase *>::iterator i= sets.begin();i!= sets.end();i++)
      {
        Set *s= dynamic_cast<Set *>(*i);
        if(s) s->GetCuerpos().push_back(this);
      }
  }

//! @brief Actualiza la topología de la superficie being passed as parameter.
void XC::Body::set_surf(Face *s)
  {
    if(s)
     { s->inserta_body(this); }
  }

//! Return indices of the vertices.
std::vector<int> XC::Body::getIndicesVertices(void) const
  {
    const size_t nv= NumVertices();
    std::vector<int> retval(nv);
    if(nv>=1)
      {
        for(size_t i=0;i<nv;i++)
          retval[i]= GetVertice(i+1)->getIdx();
      }
    return retval;
  }
