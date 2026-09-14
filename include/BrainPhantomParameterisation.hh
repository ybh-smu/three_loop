#ifndef BrainPhantomParameterisation_h
#define BrainPhantomParameterisation_h 1

#include "G4ThreeVector.hh"
#include "G4VPVParameterisation.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

#include <vector>

class G4Box;
class G4Material;
class G4VPhysicalVolume;
class BrainPhantomData;

struct VoxelInfo {
    G4int x;
    G4int y;
    G4int z;
    G4int materialID;
};

class BrainPhantomParameterisation : public G4VPVParameterisation {
public:
    explicit BrainPhantomParameterisation(BrainPhantomData* data);
    ~BrainPhantomParameterisation() override;

    void ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const override;
    void ComputeDimensions(G4Box& box, const G4int copyNo, const G4VPhysicalVolume* physVol) const override;
    G4Material* ComputeMaterial(const G4int copyNo,
                                G4VPhysicalVolume* physVol,
                                const G4VTouchable* parentTouch = nullptr) override;

    G4int GetNonAirVoxelCount() const { return static_cast<G4int>(fNonAirVoxels.size()); }

private:
    void DefineMaterials();
    void FilterNonAirVoxels();
    G4Material* GetBaseMaterial(G4int materialID) const;
    G4Material* BuildVoxelMaterial(G4int materialID, G4double density) const;

    BrainPhantomData* fPhantomData;
    std::vector<VoxelInfo> fNonAirVoxels;

    G4Material* fAir = nullptr;
    G4Material* fSoftBase = nullptr;
    G4Material* fBoneBase = nullptr;
    G4Material* fDenseBoneBase = nullptr;

    G4double fVoxelHalfX = 0.5 * mm;
    G4double fVoxelHalfY = 0.5 * mm;
    G4double fVoxelHalfZ = 0.5 * mm;
};

#endif
