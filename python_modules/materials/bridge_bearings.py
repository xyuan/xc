# -*- coding: utf-8 -*-
from __future__ import division

__author__= "Ana Ortega (AO_O) and Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2015, AO_O and LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= " ana.Ortega.Ort@gmail.com, l.pereztato@gmail.com"

import scipy.interpolate
from materials import typical_materials
from model import predefined_spaces
import math
import xc

# Points that define the values of V2 as a function of b/a
#   see "Puentes" book from Javier Manterola Armisén page 591
x2= [1,1.5,2,3,4,6,8,10,10000]
y2= [0.208,0.231,0.246,0.267,0.282,0.299,0.307,0.313,1/3.0]

# Points that define the values of V3 as a function of b/a
#   see "Puentes" book from Javier Manterola Armisén page 591
x3= [1,1.5,2,3,4,6,8,10,10000]
y3= [0.14,0.196,0.229,0.263,0.281,0.299,0.307,0.313,1/3.0]

# Points that define the values of V4 as a function of b/a
#   see "Puentes" book from Javier Manterola Armisén page 592
x4= [1,2,4,8,10000]
y4= [85.7,71.4,64.5,61.2,60]

# Points that define the values of beta as a function of h/b
#   see ALPHA in "Hormigón" book from Jiménez Montoya et al.
#   14a. edición page 405
xBeta= [1,1.25,1.5,2,3,4,6,10,10000]
yBeta= [0.14,0.171,0.196,0.229,0.263,0.281,0.299,0.313,1/3.0]

# From HP-28s calculator ;-)
def ifte(a,b,c):
    if(a):
      return b
    else:
      return c

class Bearing(object):
    """Bearings base class.

    """
    def __init__(self):
      self.materials= list()
      
    
class ElastomericBearing(Bearing):
    """Rectangular elastomeric bearing.

    """
    v2table= scipy.interpolate.interp1d(x2,y2)
    v3table=  scipy.interpolate.interp1d(x3,y3)
    v4table= scipy.interpolate.interp1d(x4,y4)
    betaTable= scipy.interpolate.interp1d(xBeta,yBeta)

    def __init__(self,G,a,b,e):
        """Class constructor.

        Args:
            G (float): Elastomer shear modulus.
            a (float): Width of the bearing (parallel to lintel axis).
            b (float): Length of the bearing (parallel to bridge axis).
            e (float): Net thickness of the bearing (without
               steel plates).

        """
        super(ElastomericBearing,self).__init__()
        self.G= G
        self.a= a
        self.b= b
        self.e= e

    def getKhoriz(self):
        '''Return horizontal stiffness of the bearing.'''
        return self.G*self.a*self.b/self.e
    def getV2(self):
        '''Return the V2 shape factor of the bearing. '''
        return self.v2table(self.b/self.a)
    def getV3(self):
        '''Return the V3 shape factor of the bearing. '''
        return self.v3table(self.b/self.a)
    def getEbearing(self):
        ''' Return the elastic modulus with respect to the vertical
        displacement.
        '''
        return 3*self.G*(self.a/self.e)**2*self.getV3()
    def getKvert(self):
        ''' Return the vertical stiffness.'''
        return self.getEbearing()*self.a*self.b/self.e
    def getV4(self):
        ''' Return the v4 shape factor of the bearing.'''
        return self.v4table(self.b/self.a)
    def getKrotationLintelAxis(self):
        ''' Stiffness with respect to the rotation around an axis
            parallel to the lintel by the center of the bearing.
        '''
        return self.G*self.a*pow(self.b,5.0)/(self.getV4()*pow(self.e,3.0))
    def getKrotationBridgeAxis(self):
        ''' Stiffness with respect to the rotation around an axis
            parallel to the bridge by the center of the bearing.
        '''
        return self.G*self.b*pow(self.a,5.0)/(self.getV4()*pow(self.e,3.0))
    def getBeta(self):
        ''' Return the value of the beta factor.'''
        return ifte(self.a<self.b,self.betaTable(self.b/self.a),self.betaTable(self.a/self.b))
    def getKrotationVerticalAxis(self):
        ''' Stiffness  with respect to the rotation around a vertical axis.'''
        return self.getBeta()*self.G*self.a*pow(self.b,3.0)/self.e
    def defineMaterials(self,preprocessor,matKX, matKY, matKZ, matKTHX, matKTHY, matKTHZ):
        '''Define the materials to modelize the elastomeric bearing.

        Args:
            preprocessor (:obj:'Preprocessor'): preprocessor to use.
            matKX (str): name for the uniaxial material in direction X.
            matKY (str): name for the uniaxial material in direction Y.
            matKZ (str): name for the uniaxial material in direction Z.
            matKTHX (str): name for the uniaxial material in direction ThetaX.
            matKTHY (str): name for the uniaxial material in direction ThetaY.
            matKTHZ (str): name for the uniaxial material in direction ThetaZ.
        '''
        self.materials.extend([matKX, matKY, matKZ, matKTHX, matKTHY, matKTHZ])
        if(matKX!=None):
          self.matXName= matKX
          self.matKX= typical_materials.defElasticMaterial(preprocessor, matKX, self.getKhoriz())
        if(matKY!=None):
          self.matYName= matKY
          self.matKY= typical_materials.defElasticMaterial(preprocessor, matKY, self.getKhoriz())
        if(matKZ!=None):
          self.matZName= matKZ
          self.matKZ= typical_materials.defElasticMaterial(preprocessor, matKZ, self.getKvert())
        if(matKTHX!=None):
          self.matTHXName= matKTHX
          self.matKTHX= typical_materials.defElasticMaterial(preprocessor, matKTHX, self.getKrotationLintelAxis())
        if(matKTHY!=None):
          self.matTHYName= matKTHY
          self.matKTHY= typical_materials.defElasticMaterial(preprocessor, matKTHY, self.getKrotationBridgeAxis())
        if(matKTHZ!=None):
          self.matTHZName= matKTHZ
          self.matKTHZ= typical_materials.defElasticMaterial(preprocessor, matKTHZ, self.getKrotationVerticalAxis())

    def putBetweenNodes(self,modelSpace,iNodA, iNodB, iElem):
        ''' Puts the bearing between the nodes.

        Args:
            modelSpace (:obj:'PredefinedSpace'): space dimension and number
                of DOFs.
            iNodA (int): first node identifier (tag).
            iNodB (int): second node identifier (tag).
            iElem (int): new zero length elem identifier (tag).
        '''
        modelSpace.setBearingBetweenNodes(iNodA,iNodB,iElem,self.materials)

# Points that define the Teflon coefficient of friction of
# from the mean compressive stress
# See the book «Aparatos de apoyo en puentes» from AIPCR page 46

# Tabla 59.8.2 del artículo 59.8.2 de EAE (página 256)
xT= [5e6,10e6,20e6,30e6,45e6,1000e6]
yT= [0.08,0.06,0.04,0.03,0.025,0.024]


class PTFEPotBearing(Bearing):
    """PTFE slide bearing.

    """
    teflonMuTable= scipy.interpolate.interp1d(xT,yT)

    def __init__(self,d):
        """Class constructor.

        Args:
            d (float): Pot diameter.

        """
        super(PTFEPotBearing,self).__init__()
        self.d= d
    def getHorizontalStiffness(self):
        '''Returns the fictitious stiffness with respect to the horizontal displacement of a PTFE slide bearing.

        Stiffness is calculated so that when the displacement reach 20 mm
        the spring reaction equals the force of friction.

        sg_media= 35 MPa mean compressive stress.
        mov= 20e-3 Desplazamiento para el que se alcanza la fuerza de rozamiento.

        '''
        return self.teflonMuTable(35e6)*math.pi*(self.d/2.0)**2*35e6/20e-3

    def defineMaterials(self,preprocessor,matKX,matKY):
        '''Define the materials to modelize the pot (Teflon).

        Args:
            preprocessor (:obj:'Preprocessor'): preprocessor to use.
            matKX (str): name for the uniaxial material in direction X.
            matKY (str): name for the uniaxial material in direction Y.
        '''
        self.materials.extend([matKX, matKY])
        if(matKX!=None):
          self.matXName= matKX
          self.matX= typical_materials.defElasticMaterial(preprocessor, self.matXName,self.getHorizontalStiffness())
        if(matKY!=None):
          self.matYName= matKY
          self.matY= typical_materials.defElasticMaterial(preprocessor, self.matYName,self.getHorizontalStiffness())

    def putBetweenNodes(self,modelSpace,iNodA, iNodB, iElem):
        ''' Puts the bearing between the nodes.

        Args:
            modelSpace (:obj:'PredefinedSpace'): space dimension and number
                of DOFs.
            iNodA (int): first node identifier (tag).
            iNodB (int): second node identifier (tag).
            iElem (int): new zero length elem identifier (tag).
        '''
        modelSpace.setBearingBetweenNodes(iNodA,iNodB,iElem,self.materials)
        eDofs= modelSpace.constraints.newEqualDOF(iNodA,iNodB,xc.ID([2]))

    def putOnXBetweenNodes(self,modelSpace,iNodA, iNodB, iElem):
        ''' Puts the bearing between the nodes only in direction X.

        Args:
            modelSpace (:obj:'PredefinedSpace'): space dimension and number
                of DOFs.
            iNodA (int): first node identifier (tag).
            iNodB (int): second node identifier (tag).
            iElem (int): new zero length elem identifier (tag).
        '''
        modelSpace.setBearingBetweenNodes(iNodA,iNodB,iElem,[self.matXName])
        eDofs= modelSpace.constraints.newEqualDOF(iNodA,iNodB,xc.ID([1,2]))

    def putOnYBetweenNodes(self,modelSpace,iNodA, iNodB, iElem):
        ''' Puts the bearing between the nodes only in direction Y.

        Args:
            modelSpace (:obj:'PredefinedSpace'): space dimension and number
                of DOFs.
            iNodA (int): first node identifier (tag).
            iNodB (int): second node identifier (tag).
            iElem (int): new zero length elem identifier (tag).
        '''
        modelSpace.setBearingBetweenNodes(iNodA,iNodB,iElem,[None,self.matYName])
        eDofs= modelSpace.constraints.newEqualDOF(iNodA,iNodB,xc.ID([0,2]))

def get_reaction_on_pot(preprocessor,iElem,inclInertia= False):
    ''' Return the element reaction.

        Args:
            preprocessor (:obj:'Preprocessor'): preprocessor to use.
            iElem (int): new zero length elem identifier (tag).
            inclInertia (bool): true if the reaction must include inertia forces.
    '''
    nodos= preprocessor.getNodeLoader
    nodos.calculateNodalReactions(inclInertia)
  
    elem= preprocessor.getElementLoader.getElement(iElem)
    reac0= elem.getNodes[0].getReaction
    return xc.Vector([reac0[0],reac0[1],reac0[2]])
