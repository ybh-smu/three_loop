#ifndef CTPhantomParameterisation_h
#define CTPhantomParameterisation_h 1

#include "G4VPVParameterisation.hh"
#include "globals.hh"

#include <map>
#include <vector>

class CTPhantomData;
class G4Box;
class G4Material;
class G4VPhysicalVolume;

struct CTVoxelInfo {
    G4int x;
    G4int y;
    G4int z;
    G4int hu;
};

class CTPhantomParameterisation : public G4VPVParameterisation {
public:
    explicit CTPhantomParameterisation(CTPhantomData* data);
    ~CTPhantomParameterisation() override;

    void ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const override;
    void ComputeDimensions(G4Box& box, const G4int copyNo, const G4VPhysicalVolume* physVol) const override;
    G4Material* ComputeMaterial(const G4int copyNo,
                                G4VPhysicalVolume* physVol,
                                const G4VTouchable* parentTouch = nullptr) override;

    G4int GetVoxelCount() const { return static_cast<G4int>(fVoxels.size()); }

private:
    G4double EstimateDensityFromHU(G4int hu) const;
    G4String ClassifyMaterialFamily(G4int hu) const;
    G4Material* BuildMaterialForHU(G4int hu);
    void FilterRelevantVoxels();

    CTPhantomData* fPhantomData;
    std::vector<CTVoxelInfo> fVoxels;
    std::map<G4int, G4Material*> fMaterialCache;
    G4double fVoxelHalfX;
    G4double fVoxelHalfY;
    G4double fVoxelHalfZ;
    G4int fAirThresholdHU;
};

#endif
