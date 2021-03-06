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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.6 $
// $Date: 2002/12/05 22:49:10 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/nD/ElasticIsotropicPlateFiber.cpp,v $
                                                                        
                                                                        

#include <material/nD/elastic_isotropic/ElasticIsotropicPlateFiber.h>           

#include "utility/matrix/Matrix.h"
#include "material/nD/NDMaterialType.h"

XC::Vector XC::ElasticIsotropicPlateFiber::sigma(5);
XC::Matrix XC::ElasticIsotropicPlateFiber::D(5,5);

XC::ElasticIsotropicPlateFiber::ElasticIsotropicPlateFiber(int tag, double E, double nu, double rho)
  : ElasticIsotropicMaterial(tag, ND_TAG_ElasticIsotropicPlateFiber,5, E, nu, rho)
  {}

XC::ElasticIsotropicPlateFiber::ElasticIsotropicPlateFiber(int tag)
  : ElasticIsotropicMaterial(tag, ND_TAG_ElasticIsotropicPlateFiber,5, 0.0, 0.0)
  {}

XC::ElasticIsotropicPlateFiber::ElasticIsotropicPlateFiber()
  : ElasticIsotropicMaterial(0, ND_TAG_ElasticIsotropicPlateFiber,5, 0.0, 0.0)
  {}

int XC::ElasticIsotropicPlateFiber::setTrialStrainIncr(const XC::Vector &strain)
{
  epsilon += strain;
  return 0;
}

int XC::ElasticIsotropicPlateFiber::setTrialStrainIncr(const XC::Vector &strain, const XC::Vector &rate)
{
  epsilon += strain;
  return 0;
}

const XC::Matrix &XC::ElasticIsotropicPlateFiber::getTangent(void) const
{
    double d00 = E/(1.0-v*v);
    double d01 = v*d00;
    double d22 = 0.5*(d00-d01);
    
    D(0,0) = D(1,1) = d00;
    D(0,1) = D(1,0) = d01;
    D(2,2) = d22;
    D(3,3) = d22;
    D(4,4) = d22;

    return D;
}

const XC::Matrix &XC::ElasticIsotropicPlateFiber::getInitialTangent(void) const
{
    double d00 = E/(1.0-v*v);
    double d01 = v*d00;
    double d22 = 0.5*(d00-d01);
    
    D(0,0) = D(1,1) = d00;
    D(0,1) = D(1,0) = d01;
    D(2,2) = d22;
    D(3,3) = d22;
    D(4,4) = d22;

    return D;
}

const XC::Vector &XC::ElasticIsotropicPlateFiber::getStress(void) const
  {
    const double d00 = E/(1.0-v*v);
    const double d01 = v*d00;
    const double d22 = 0.5*(d00-d01);

    const double eps0 = epsilon(0);
    const double eps1 = epsilon(1);

    //sigma = D*epsilon;
    sigma(0) = d00*eps0 + d01*eps1;
    sigma(1) = d01*eps0 + d00*eps1;

    sigma(2) = d22*epsilon(2);
    sigma(3) = d22*epsilon(3);
    sigma(4) = d22*epsilon(4);
	
    return sigma;
  }

int XC::ElasticIsotropicPlateFiber::commitState(void)
{
  return 0;
}

int XC::ElasticIsotropicPlateFiber::revertToLastCommit(void)
{
  return 0;
}

int XC::ElasticIsotropicPlateFiber::revertToStart(void)
{
  epsilon.Zero();
  return 0;
}

XC::NDMaterial *XC::ElasticIsotropicPlateFiber::getCopy(void) const
  { return new ElasticIsotropicPlateFiber(*this); }

const std::string &XC::ElasticIsotropicPlateFiber::getType(void) const
  { return strTypePlateFiber; }

int XC::ElasticIsotropicPlateFiber::getOrder(void) const
  { return 5; }
