#include "BrainPhantomParameterisation.hh"

#include "BrainPhantomData.hh"
#include "G4Box.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"

#include <sstream>

BrainPhantomParameterisation::BrainPhantomParameterisation(BrainPhantomData* data)
    : fPhantomData(data) {
    fVoxelHalfX = 0.5 * fPhantomData->GetVoxelSizeX();
    fVoxelHalfY = 0.5 * fPhantomData->GetVoxelSizeY();
    fVoxelHalfZ = 0.5 * fPhantomData->GetVoxelSizeZ();
    DefineMaterials();
    FilterNonAirVoxels();
}

BrainPhantomParameterisation::~BrainPhantomParameterisation() = default;

void BrainPhantomParameterisation::DefineMaterials() {
    G4NistManager* nist = G4NistManager::Instance();
    fAir = nist->FindOrBuildMaterial("G4_AIR");
    fSoftBase = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
    fBoneBase = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    fDenseBoneBase = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
}

void BrainPhantomParameterisation::FilterNonAirVoxels() {
    fNonAirVoxels.clear();
    for (G4int z = 0; z < fPhantomData->GetDepth(); ++z) {
        for (G4int y = 0; y < fPhantomData->GetHeight(); ++y) {
            for (G4int x = 0; x < fPhantomData->GetWidth(); ++x) {
                const G4int id = fPhantomData->GetMaterialId(x, y, z);
                if (id != 0) {
                    fNonAirVoxels.push_back({x, y, z, id});
                }
            }
        }
    }
}

void BrainPhantomParameterisation::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const {
    const VoxelInfo& info = fNonAirVoxels[copyNo];
    const G4double xp = (info.x - (fPhantomData->GetWidth()  - 1) / 2.0) * (2.0 * fVoxelHalfX);
    const G4double yp = (info.y - (fPhantomData->GetHeight() - 1) / 2.0) * (2.0 * fVoxelHalfY);
    const G4double zp = (info.z - (fPhantomData->GetDepth()  - 1) / 2.0) * (2.0 * fVoxelHalfZ);
    physVol->SetTranslation(G4ThreeVector(xp, yp, zp));
}

void BrainPhantomParameterisation::ComputeDimensions(G4Box& box, const G4int, const G4VPhysicalVolume*) const {
    box.SetXHalfLength(fVoxelHalfX);
    box.SetYHalfLength(fVoxelHalfY);
    box.SetZHalfLength(fVoxelHalfZ);
}

G4Material* BrainPhantomParameterisation::GetBaseMaterial(G4int materialID) const {
    switch (materialID) {
        case 1: return fSoftBase;      // BrainSoftTissue
        case 2: return fBoneBase;      // Bone
        case 3: return fDenseBoneBase; // DenseBone
        default: return fAir;
    }
}

G4Material* BrainPhantomParameterisation::BuildVoxelMaterial(G4int materialID, G4double density) const {
    G4Material* base = GetBaseMaterial(materialID);
    if (materialID == 0 || base == nullptr) return fAir;

    density = std::max(0.0012 * g / cm3, density);

    std::ostringstream name;
    name << "BrainVoxelMat_" << materialID << "_" << static_cast<int>(density / (0.001 * g / cm3));

    G4Material* mat = G4Material::GetMaterial(name.str(), false);
    if (mat) return mat;

    mat = new G4Material(name.str(), density, base->GetNumberOfElements(), kStateSolid);
    for (size_t i = 0; i < base->GetNumberOfElements(); ++i) {
        const G4Element* elem = base->GetElement(i);
        G4double frac = base->GetFractionVector()[i];
        mat->AddElement(const_cast<G4Element*>(elem), frac);
    }
    return mat;
}

G4Material* BrainPhantomParameterisation::ComputeMaterial(const G4int copyNo,
                                                          G4VPhysicalVolume*,
                                                          const G4VTouchable*) {
    const VoxelInfo& info = fNonAirVoxels[copyNo];
    const G4double density = fPhantomData->GetDensity(info.x, info.y, info.z);
    return BuildVoxelMaterial(info.materialID, density);
}
