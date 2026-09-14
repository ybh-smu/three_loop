#ifndef PHANTOM_PARAMETERISATION_HH
#define PHANTOM_PARAMETERISATION_HH

#include "G4VPVParameterisation.hh"
#include <vector>
#include <map>

class PhantomData;
class G4VPhysicalVolume;
class G4Box;
class G4Material;

class PhantomParameterisation : public G4VPVParameterisation {
public:
    PhantomParameterisation(PhantomData* data);
    // PhantomParameterisation(PhantomData* data,
    //                       G4double voxX, G4double voxY, G4double voxZ);
    //
    virtual ~PhantomParameterisation();

    void ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const override;
    void ComputeDimensions(G4Box& box, const G4int copyNo, const G4VPhysicalVolume* physVol) const override;
    G4Material* ComputeMaterial(const G4int copyNo, G4VPhysicalVolume* physVol, const G4VTouchable* parentTouch = 0) override;

    G4int GetNonAirVoxelCount() const { return fNonAirVoxels.size(); }

private:
    void DefineMaterials();
    void GetVoxelIndices(const G4int copyNo, G4int& x, G4int& y, G4int& z) const;
    void FilterNonAirVoxels();

    struct VoxelInfo {
        G4int x, y, z;
        G4int organID;
    };

    PhantomData* fPhantomData;
    G4double fVoxelHalfX, fVoxelHalfY, fVoxelHalfZ;
    std::vector<G4Material*> fMaterials;
    std::map<G4int, std::pair<G4int, G4double>> fOrganToMaterial;
    std::vector<VoxelInfo> fNonAirVoxels;
    
    //aaaaaa
    // G4double fVoxX, fVoxY, fVoxZ;
    //aaaa
};

#endif