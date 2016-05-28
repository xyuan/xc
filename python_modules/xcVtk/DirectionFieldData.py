# -*- coding: utf-8 -*-

'''Data to represent directions (modulus doesn't matters) field in VTK.'''

import vtk
from miscUtils import LogMessages as lmsg
from xcVtk import VectorFieldData as vfd

class DirectionFieldData(vfd.VectorFieldData):
  '''Directions (modulus doesn't matters) Vectors defined at points.'''
  def __init__(self, name, color, numberOfComponents= 3, scaleFactor= 1.0):
    '''
    Parameters:
      name: name to identify VTK arrays related to this object.
      numberOrComponents: size of the vectors.
      scaleFactor: scale factor for the size of the vectors.
    '''
    super(DirectionFieldData,self).__init__(name,numberOfComponents,scaleFactor)
    self.lookupTable= vtk.vtkLookupTable()
    self.lookupTable.SetNumberOfTableValues(2)
    lutColor= [color[0],color[1],color[2],1.0]
    self.lookupTable.SetTableValue(0,lutColor)
    self.lookupTable.SetTableValue(1,lutColor)

  def setupMapper(self):
    self.setupGlyph()
    self.mapper = vtk.vtkPolyDataMapper()
    self.mapper.SetInputConnection(self.glyph.GetOutputPort())
    self.mapper.SetScalarModeToUsePointFieldData()
    self.mapper.SetColorModeToMapScalars()
    self.mapper.ScalarVisibilityOn()
    self.mapper.SetScalarRange(self.lengths.GetRange())
    self.mapper.SelectColorArray(self.lenghtsName)
    self.mapper.SetLookupTable(self.lookupTable)
    return self.mapper

  def setupActor(self):
    self.setupMapper()
    self.actor = vtk.vtkActor()
    self.actor.SetMapper(self.mapper)
    return self.actor

  def addToDisplay(self, recordDisplay):
    # Adds the direction field to the display.
    self.setupActor()
    recordDisplay.renderer.AddActor(self.actor)