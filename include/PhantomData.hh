/// PhantomData.hh
#ifndef PhantomData_H
#define PhantomData_H

#include "globals.hh"
#include <vector>

class PhantomData {
public:
    PhantomData(const G4String& filename);
    ~PhantomData();
    
    // Load data from file
    G4bool ReadData();
    
    // Access methods
    G4int GetWidth() const { return fWidth; }
    G4int GetHeight() const { return fHeight; }
    G4int GetDepth() const { return fDepth; }
    G4int GetVoxelValue(G4int x, G4int y, G4int z) const;
    G4int GetMinValue() const { return fMinValue; }
    G4int GetMaxValue() const { return fMaxValue; }
    G4bool SaveOrthogonalSlicesMontageAsPGM(const G4String& outputFilename,
                                            G4double localX,
                                            G4double localY,
                                            G4double localZ,
                                            G4double voxelSizeX,
                                            G4double voxelSizeY,
                                            G4double voxelSizeZ) const;
    
private:
    G4String fFilename;
    G4int fWidth, fHeight, fDepth;
    G4int fMinValue, fMaxValue;
    std::vector<G4int> fData;  // Store the phantom data
};

#endif