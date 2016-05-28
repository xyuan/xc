# -*- coding: utf-8 -*-

__author__= "Luis C. Pérez Tato (LCPT), Ana Ortega(AOO)"
__copyright__= "Copyright 2016,LCPT, AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com, ana.Ortega.Ort@gmail.com"

import pickle
import os
from solution import predefined_solutions
from postprocess.reports import export_internal_forces as eif
from postprocess.reports import export_displacements as edisp
from miscUtils import LogMessages as lmsg


class LimitStateData(object):
  check_results_directory= './' #Path to verifRsl* files.
  internal_forces_results_directory= './' #Path to esf_el* f
  def __init__(self,limitStateLabel,outputDataBaseFileName):
    '''Limit state data constructor
       label; limit state check label; Something like "Fatigue" or "CrackControl"
       outputDataBaseFileName: name (whithout extension) of the file that contains the results to display.
    '''
    self.label= limitStateLabel
    self.outputDataBaseFileName= outputDataBaseFileName
    self.controller= None
  def getInternalForcesFileName(self):
    return self.internal_forces_results_directory+'intForce_'+ self.label +'.csv'
  def getDisplacementsFileName(self):
    return self.internal_forces_results_directory+'displ_'+ self.label +'.csv'
  def getOutputDataBaseFileName(self):
    '''Returns the output file name without extension.'''
    return self.check_results_directory+self.outputDataBaseFileName
  def getOutputDataFileName(self):
    '''Returns the Python executable file name.'''
    return self.getOutputDataBaseFileName() + '.py'
  def loadPickleObject(objName):
    with open(name + '.pkl', 'r') as f:
      return pickle.load(f)
  def saveAll(self,model,combContainer,setCalc,fConvIntForc= 1.0):
    '''Writes internal forces, displacements, .., for each combination
    Parameters:
      setCalc:      set of entities for which the analysis is going to be performed
      fConvIntForc: conversion factor between the unit of force in which the calculation
                    is performed and that one desired for the displaying of internal forces
                    (The use of this factor won't be allowed in future versions)
    '''
    if fConvIntForc != 1.0:
      lmsg.warning('fConvIntForc= ' + fConvIntForc + 'In future versions only the value 1.0 will be allowed as conversion factor between units' )
    feProblem= model.getFEProblem()
    preprocessor= model.getPreprocessor()
    loadCombinations= preprocessor.getLoadLoader.getLoadCombinations
    loadCombinations= self.dumpCombinations(combContainer,loadCombinations)
    elemSet= setCalc.getElements
    nodSet=setCalc.getNodes
    fNameInfForc= self.getInternalForcesFileName()
    fNameDispl=self.getDisplacementsFileName()
    os.system("rm -f " + fNameInfForc)
    os.system("rm -f " + fNameDispl)
    for key in loadCombinations.getKeys():
      comb= loadCombinations[key]
      feProblem.getPreprocessor.resetLoadCase()
      comb.addToDomain()
      #Solución
      analisis= predefined_solutions.simple_static_linear(feProblem)
      result= analisis.analyze(1)
      fIntF= open(fNameInfForc,"a")
      fDisp= open(fNameDispl,"a")
      eif.exportShellInternalForces(comb.getName,elemSet,fIntF,fConvIntForc)
      edisp.exportShellDisplacements(comb.getName,nodSet,fDisp)
      fIntF.close()
      fDisp.close()
      comb.removeFromDomain()

class NormalStressesRCLimitStateData(LimitStateData):
  ''' Reinforced concrete normal stresses limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(NormalStressesRCLimitStateData,self).__init__('ULS_normalStressesResistance','verifRsl_normStrsULS')

  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.ULS.dumpCombinations(loadCombinations)
    return loadCombinations

  def check(self,sections,sectionsNamesForEveryElement):
    intForcCombFileName= self.getInternalForcesFileName()
    out= self.getOutputDataBaseFileName()
    return sections.verifyNormalStresses(intForcCombFileName,out,sectionsNamesForEveryElement, "d",self.controller)

class ShearResistanceRCLimitStateData(LimitStateData):
  ''' Reinforced concrete shear resistance limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(ShearResistanceRCLimitStateData,self).__init__('ULS_shearResistance','verifRsl_shearULS')
  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.ULS.dumpCombinations(loadCombinations)
    return loadCombinations
  def check(self,sections,sectionsNamesForEveryElement):
    intForcCombFileName= self.getInternalForcesFileName()
    out= self.getOutputDataBaseFileName()
    return sections.shearVerification(intForcCombFileName,out,sectionsNamesForEveryElement, "d",self.controller)

class FreqLoadsCrackControlRCLimitStateData(LimitStateData):
  ''' Reinforced concrete crack control under frequent loads limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(FreqLoadsCrackControlRCLimitStateData,self).__init__('SLS_frequentLoadsCrackControl','verifRsl_crackingSLS_freq')
  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.SLS.freq.dumpCombinations(loadCombinations)
    return loadCombinations
  def check(self,sections,sectionsNamesForEveryElement):
    intForcCombFileName= self.getInternalForcesFileName()
    out= self.getOutputDataBaseFileName()
    return sections.crackControl(intForcCombFileName,out,sectionsNamesForEveryElement, "k", self.controller)

class QPLoadsCrackControlRCLimitStateData(LimitStateData):
  ''' Reinforced concrete crack control under quasi-permanent loads limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(QPLoadsCrackControlRCLimitStateData,self).__init__('SLS_quasiPermanentLoadsLoadsCrackControl','verifRsl_crackingSLS_qperm')
  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.SLS.qp.dumpCombinations(loadCombinations)
    return loadCombinations
  def check(self,sections,sectionsNamesForEveryElement):
    intForcCombFileName= self.getInternalForcesFileName()
    out= self.getOutputDataBaseFileName()
    return sections.crackControl(intForcCombFileName,out,sectionsNamesForEveryElement, "k",self.controller)

class FreqLoadsDisplacementControlLimitStateData(LimitStateData):
  ''' Displacement control under frequent loads limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(FreqLoadsDisplacementControlLimitStateData,self).__init__('SLS_frequentLoadsDisplacementControl','')
  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.SLS.freq.dumpCombinations(loadCombinations)
    return loadCombinations
  def check(self,sections,sectionsNamesForEveryElement):
    print 'FreqLoadsDisplacementControlLimitStateData.check() not implemented.'


class FatigueResistanceRCLimitStateData(LimitStateData):
  ''' Reinforced concrete shear resistance limit state data.'''
  def __init__(self):
    '''Limit state data constructor '''
    super(FatigueResistanceRCLimitStateData,self).__init__('ULS_fatigueResistance','verifRsl_fatigueULS')
  def dumpCombinations(self,combContainer,loadCombinations):
    '''Load into the solver the combinations needed for this limit state.'''
    loadCombinations.clear()
    combContainer.ULS.fatigue.dumpCombinations(loadCombinations)
    return loadCombinations
  def check(self,sections,sectionsNamesForEveryElement):
    intForcCombFileName= self.getInternalForcesFileName()
    out= self.getOutputDataBaseFileName()
    return sections.fatigueVerification(intForcCombFileName,out,sectionsNamesForEveryElement, "d",self.controller)


freqLoadsDisplacementControl= FreqLoadsDisplacementControlLimitStateData()
freqLoadsCrackControl= FreqLoadsCrackControlRCLimitStateData()
quasiPermanentLoadsCrackControl= QPLoadsCrackControlRCLimitStateData()
normalStressesResistance= NormalStressesRCLimitStateData()
shearResistance= ShearResistanceRCLimitStateData()
fatigueResistance= FatigueResistanceRCLimitStateData()



