#ifndef CTPhantomData_h
#define CTPhantomData_h 1

#include "globals.hh"

#include <vector>

class CTPhantomData {
public:
    explicit CTPhantomData(const G4String& metaFilename);
    ~CTPhantomData();

    G4bool ReadData();
    G4int GetVoxelHU(G4int x, G4int y, G4int z) const;

    G4int GetWidth() const { return fWidth; }
    G4int GetHeight() const { return fHeight; }
    G4int GetDepth() const { return fDepth; }
    G4double GetVoxelSizeX() const { return fVoxelSizeX; }
    G4double GetVoxelSizeY() const { return fVoxelSizeY; }
    G4double GetVoxelSizeZ() const { return fVoxelSizeZ; }
    const G4String& GetRawFilename() const { return fRawFilename; }

private:
    G4bool ReadMetaFile();

    G4String fMetaFilename;
    G4String fRawFilename;
    G4int fWidth;
    G4int fHeight;
    G4int fDepth;
    G4double fVoxelSizeX;
    G4double fVoxelSizeY;
    G4double fVoxelSizeZ;
    std::vector<G4int> fData;
};

#endif
