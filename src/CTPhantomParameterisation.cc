#include "CTPhantomParameterisation.hh"

#include "CTPhantomData.hh"
#include "G4Box.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"

#include <algorithm>
#include <sstream>

CTPhantomParameterisation::CTPhantomParameterisation(CTPhantomData* data)
    : fPhantomData(data),
      fVoxelHalfX(data->GetVoxelSizeX() / 2.0),
      fVoxelHalfY(data->GetVoxelSizeY() / 2.0),
      fVoxelHalfZ(data->GetVoxelSizeZ() / 2.0),
      fAirThresholdHU(-300) {
    FilterRelevantVoxels();
}

CTPhantomParameterisation::~CTPhantomParameterisation() = default;

void CTPhantomParameterisation::FilterRelevantVoxels()
{
    fVoxels.clear();
    for (G4int z = 0; z < fPhantomData->GetDepth(); ++z) {
        for (G4int y = 0; y < fPhantomData->GetHeight(); ++y) {
            for (G4int x = 0; x < fPhantomData->GetWidth(); ++x) {
                const G4int hu = fPhantomData->GetVoxelHU(x, y, z);
                if (hu > fAirThresholdHU) {
                    fVoxels.push_back({x, y, z, hu});
                }
            }
        }
    }

    G4cout << "CT phantom kept " << fVoxels.size()
           << " voxels above HU " << fAirThresholdHU << G4endl;
}

void CTPhantomParameterisation::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const
{
    const CTVoxelInfo& info = fVoxels[copyNo];
    const G4double xp = (info.x - (fPhantomData->GetWidth() - 1) / 2.0) * (2 * fVoxelHalfX);
    const G4double yp = (info.y - (fPhantomData->GetHeight() - 1) / 2.0) * (2 * fVoxelHalfY);
    const G4double zp = (info.z - (fPhantomData->GetDepth() - 1) / 2.0) * (2 * fVoxelHalfZ);
    physVol->SetTranslation(G4ThreeVector(xp, yp, zp));
}

void CTPhantomParameterisation::ComputeDimensions(G4Box& box, const G4int, const G4VPhysicalVolume*) const
{
    box.SetXHalfLength(fVoxelHalfX);
    box.SetYHalfLength(fVoxelHalfY);
    box.SetZHalfLength(fVoxelHalfZ);
}

G4double CTPhantomParameterisation::EstimateDensityFromHU(G4int hu) const
{
    if (hu < -100) {
        return std::max(0.88, 1.0 + 0.0010 * hu);
    }
    if (hu < 100) {
        return std::max(0.95, std::min(1.05, 1.0 + 0.0005 * hu));
    }
    if (hu < 300) {
        return std::min(1.15, 1.03 + 0.0005 * (hu - 100));
    }
    if (hu < 1200) {
        return std::min(1.90, 1.15 + 0.00065 * (hu - 300));
    }
    return std::min(2.20, 1.90 + 0.00025 * (hu - 1200));
}

G4String CTPhantomParameterisation::ClassifyMaterialFamily(G4int hu) const
{
    if (hu < -100) {
        return "LowDensity";
    }
    if (hu < 150) {
        return "SoftTissue";
    }
    if (hu < 300) {
        return "DenseSoft";
    }
    if (hu < 1200) {
        return "Spongiosa";
    }
    return "CorticalBone";
}

G4Material* CTPhantomParameterisation::BuildMaterialForHU(G4int hu)
{
    // Use coarser bins to keep the material table bounded.
    const G4int binWidth = (hu < 300) ? 25 : 100;
    const G4int bin = binWidth * static_cast<G4int>(hu / binWidth);
    const auto it = fMaterialCache.find(bin);
    if (it != fMaterialCache.end()) {
        return it->second;
    }

    G4NistManager* nist = G4NistManager::Instance();
    const G4double density = EstimateDensityFromHU(bin) * g / cm3;
    const G4String family = ClassifyMaterialFamily(bin);
    const G4String materialName = "CT_" + family + "_" + std::to_string(bin);

    G4Material* material = G4Material::GetMaterial(materialName, false);
    if (!material) {
        G4Material* baseMaterial = nullptr;

        if (family == "LowDensity") {
            baseMaterial = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
        } else if (family == "SoftTissue") {
            baseMaterial = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
        } else if (family == "DenseSoft") {
            baseMaterial = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
        } else if (family == "Spongiosa") {
            baseMaterial = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
        } else {
            baseMaterial = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
        }

        material = nist->BuildMaterialWithNewDensity(materialName, baseMaterial->GetName(), density);
    }

    fMaterialCache[bin] = material;
    return material;
}

G4Material* CTPhantomParameterisation::ComputeMaterial(const G4int copyNo,
                                                       G4VPhysicalVolume*,
                                                       const G4VTouchable*)
{
    return BuildMaterialForHU(fVoxels[copyNo].hu);
}
