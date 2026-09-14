#ifndef BrainPhantomData_h
#define BrainPhantomData_h 1

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include <vector>

class BrainPhantomData {
public:
    // metaFile 指向 brain_meta.txt
    explicit BrainPhantomData(const G4String& metaFile);
    ~BrainPhantomData();

    G4bool ReadData();

    G4int GetMaterialId(G4int x, G4int y, G4int z) const;
    G4double GetDensity(G4int x, G4int y, G4int z) const;

    G4int GetWidth() const { return fWidth; }
    G4int GetHeight() const { return fHeight; }
    G4int GetDepth() const { return fDepth; }

    G4double GetVoxelSizeX() const { return fVoxelSizeX; }
    G4double GetVoxelSizeY() const { return fVoxelSizeY; }
    G4double GetVoxelSizeZ() const { return fVoxelSizeZ; }

    G4double GetOriginX() const { return fOriginX; }
    G4double GetOriginY() const { return fOriginY; }
    G4double GetOriginZ() const { return fOriginZ; }
    void SavePreviewPGM(const G4String& outFile) const;

private:
    size_t Index(G4int x, G4int y, G4int z) const;

    G4String fMetaFile;
    G4String fMaterialFile;
    G4String fDensityFile;

    G4int fWidth = 0;
    G4int fHeight = 0;
    G4int fDepth = 0;

    G4double fVoxelSizeX = 1.0 * mm;
    G4double fVoxelSizeY = 1.0 * mm;
    G4double fVoxelSizeZ = 1.0 * mm;

    G4double fOriginX = 0.0 * mm;
    G4double fOriginY = 0.0 * mm;
    G4double fOriginZ = 0.0 * mm;

    std::vector<unsigned char> fMaterialIds; // 0,1,2,3
    std::vector<float> fDensities;           // g/cm3
};

#endif
