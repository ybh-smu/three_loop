#include "PhantomParameterisation.hh"
#include "PhantomData.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4VPhysicalVolume.hh"

PhantomParameterisation::PhantomParameterisation(PhantomData* data) 
: fPhantomData(data){
    // 每个体素的半尺寸设置为0.5mm (每个体素为1mm*1mm*1mm)
    // fVoxelHalfX = 0.5 * mm;
    // fVoxelHalfY = 0.5 * mm;
    // fVoxelHalfZ = 0.5 * mm;
    fVoxelHalfX = 0.5 * mm;
    fVoxelHalfY = 0.5* mm;
    fVoxelHalfZ = 1 * mm;

    FilterNonAirVoxels();
    DefineMaterials();
}

PhantomParameterisation::~PhantomParameterisation() {
}

void PhantomParameterisation::FilterNonAirVoxels() {
    G4int nx = fPhantomData->GetWidth();
    G4int ny = fPhantomData->GetHeight();
    G4int nz = fPhantomData->GetDepth();

    for (G4int z = 0; z < nz; ++z) {
        for (G4int y = 0; y < ny; ++y) {
            for (G4int x = 0; x < nx; ++x) {
                G4int organID = fPhantomData->GetVoxelValue(x, y, z);
                if (organID != 0) {
                    VoxelInfo info;
                    info.x = x;
                    info.y = y;
                    info.z = z;
                    info.organID = organID;
                    fNonAirVoxels.push_back(info);
                }
            }
        }
    }

    G4cout << "Found " << fNonAirVoxels.size() << " non-air voxels out of "
           << nx * ny * nz << " total voxels" << G4endl;
}

void PhantomParameterisation::DefineMaterials() {
    G4NistManager* nistManager = G4NistManager::Instance();

    G4Element* H = new G4Element("Hydrogen", "H", 1, 1.00794 * g/mole);
    G4Element* C = new G4Element("Carbon", "C", 6, 12.0107 * g/mole);
    G4Element* N = new G4Element("Nitrogen", "N", 7, 14.0067 * g/mole);
    G4Element* O = new G4Element("Oxygen", "O", 8, 15.9994 * g/mole);
    G4Element* Na = new G4Element("Sodium", "Na", 11, 22.9897 * g/mole);
    G4Element* P = new G4Element("Phosphorus", "P", 15, 30.9738 * g/mole);
    G4Element* S = new G4Element("Sulfur", "S", 16, 32.0650 * g/mole);
    G4Element* Cl = new G4Element("Chlorine", "Cl", 17, 35.4530 * g/mole);
    G4Element* K = new G4Element("Potassium", "K", 19, 39.0983 * g/mole);
    G4Element* Ca = new G4Element("Calcium", "Ca", 20, 40.0780 * g/mole);

    std::map<G4int, std::pair<G4int, G4double>> organToMaterial = {
        {10, {10, 1.00}}, {11, {29, 0.92}}, {14, {12, 1.02}}, {15, {13, 1.06}},
        {16, {18, 1.01}}, {18, {14, 1.05}}, {19, {15, 0.98}}, {21, {12, 1.05}},
        {22, {16, 1.05}}, {23, {16, 1.05}}, {24, {12, 1.02}}, {25, {12, 1.09}},
        {26, {19, 1.04}}, {27, {20, 1.06}}, {28, {21, 1.03}}, {29, {12, 1.07}},
        {30, {17, 1.02}}, {31, {18, 1.05}}, {32, {22, 1.05}}, {33, {23, 0.41}},
        {34, {24, 1.04}}, {35, {12, 1.04}}, {36, {12, 1.09}}, {37, {12, 1.00}},
        {38, {28, 1.05}}, {39, {25, 1.05}}, {43, {12, 1.05}}, {44, {17, 1.03}},
        {45, {18, 1.07}}, {46, {32, 1.90}}, {47, {33, 1.40}}, {63, {33, 1.40}},
        {65, {14, 1.05}}, {66, {30, 1.04}}, {67, {17, 1.05}}, {68, {18, 1.03}},
        {69, {12, 1.03}}, {70, {31, 1.01}}, {73, {12, 1.05}}, {76, {14, 1.05}},
        {77, {14, 1.05}}, {79, {14, 1.05}}, {82, {17, 1.03}}, {85, {27, 1.05}}
    };

    fMaterials.resize(100, nullptr);

    for (const auto& entry : organToMaterial) {
        G4int organID = entry.first;
        G4int materialID = entry.second.first;
        G4double density = entry.second.second * g/cm3;
        G4Material* baseMat = nullptr;

        switch (materialID) {
            case 10: {
                baseMat = new G4Material("Remainder_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.106);
                baseMat->AddElement(C, 0.315);
                baseMat->AddElement(N, 0.024);
                baseMat->AddElement(O, 0.547);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.001);
                break;
            }
            case 12: {
                baseMat = new G4Material("OtherOrgans_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.105);
                baseMat->AddElement(C, 0.256);
                baseMat->AddElement(N, 0.027);
                baseMat->AddElement(O, 0.602);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.003);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 13: {
                baseMat = new G4Material("BladderWall_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.105);
                baseMat->AddElement(C, 0.096);
                baseMat->AddElement(N, 0.026);
                baseMat->AddElement(O, 0.761);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.003);
                baseMat->AddElement(K, 0.003);
                break;
            }
            case 14: {
                baseMat = new G4Material("Brain_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.107);
                baseMat->AddElement(C, 0.145);
                baseMat->AddElement(N, 0.022);
                baseMat->AddElement(O, 0.712);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.004);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.003);
                baseMat->AddElement(K, 0.003);
                break;
            }
            case 15: {
                baseMat = new G4Material("Breasts_" + std::to_string(organID), density, 3);
                baseMat->AddElement(H, 0.114);
                baseMat->AddElement(C, 0.461);
                baseMat->AddElement(O, 0.420);
                break;
            }
            case 16: {
                baseMat = new G4Material("Eyeballs_" + std::to_string(organID), density, 7);
                baseMat->AddElement(H, 0.096);
                baseMat->AddElement(C, 0.195);
                baseMat->AddElement(N, 0.057);
                baseMat->AddElement(O, 0.646);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.003);
                break;
            }
            case 17: {
                baseMat = new G4Material("Walls_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.106);
                baseMat->AddElement(C, 0.115);
                baseMat->AddElement(N, 0.022);
                baseMat->AddElement(O, 0.751);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 18: {
                baseMat = new G4Material("Contents_" + std::to_string(organID), density, 6);
                baseMat->AddElement(H, 0.101);
                baseMat->AddElement(C, 0.111);
                baseMat->AddElement(N, 0.026);
                baseMat->AddElement(O, 0.762);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(Cl, 0.001);
                break;
            }
            case 19: {
                baseMat = new G4Material("HeartWall_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.104);
                baseMat->AddElement(C, 0.139);
                baseMat->AddElement(N, 0.029);
                baseMat->AddElement(O, 0.718);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 20: {
                baseMat = new G4Material("HeartContent_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.102);
                baseMat->AddElement(C, 0.113);
                baseMat->AddElement(N, 0.033);
                baseMat->AddElement(O, 0.745);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 21: {
                baseMat = new G4Material("Kidneys_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.103);
                baseMat->AddElement(C, 0.132);
                baseMat->AddElement(N, 0.033);
                baseMat->AddElement(O, 0.724);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                baseMat->AddElement(K, 0.002);
                break;
            }
            case 22: {
                baseMat = new G4Material("Liver_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.103);
                baseMat->AddElement(C, 0.186);
                baseMat->AddElement(N, 0.028);
                baseMat->AddElement(O, 0.671);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                baseMat->AddElement(K, 0.002);
                break;
            }
            case 23: {
                baseMat = new G4Material("Lungs_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.103);
                baseMat->AddElement(C, 0.105);
                baseMat->AddElement(N, 0.031);
                baseMat->AddElement(O, 0.749);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.003);
                baseMat->AddElement(Cl, 0.002);
                baseMat->AddElement(K, 0.002);
                break;
            }
            case 24: {
                baseMat = new G4Material("LymphNodes_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.108);
                baseMat->AddElement(C, 0.012);
                baseMat->AddElement(N, 0.031);
                baseMat->AddElement(O, 0.831);
                baseMat->AddElement(Na, 0.003);
                baseMat->AddElement(P, 0.003);
                baseMat->AddElement(S, 0.003);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 25: {
                baseMat = new G4Material("Muscle_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.102);
                baseMat->AddElement(C, 0.142);
                baseMat->AddElement(N, 0.034);
                baseMat->AddElement(O, 0.711);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.001);
                baseMat->AddElement(Cl, 0.004);
                break;
            }
            case 27: {
                baseMat = new G4Material("Testis_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.106);
                baseMat->AddElement(C, 0.103);
                baseMat->AddElement(N, 0.021);
                baseMat->AddElement(O, 0.764);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.001);
                break;
            }
            case 28: {
                baseMat = new G4Material("Pancreas_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.106);
                baseMat->AddElement(C, 0.169);
                baseMat->AddElement(N, 0.022);
                baseMat->AddElement(O, 0.694);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.002);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 29: {
                baseMat = new G4Material("Skin_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.100);
                baseMat->AddElement(C, 0.204);
                baseMat->AddElement(N, 0.042);
                baseMat->AddElement(O, 0.645);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 30: {
                baseMat = new G4Material("Spleen_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.103);
                baseMat->AddElement(C, 0.133);
                baseMat->AddElement(N, 0.032);
                baseMat->AddElement(O, 0.741);
                baseMat->AddElement(Na, 0.001);
                baseMat->AddElement(P, 0.003);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 31: {
                baseMat = new G4Material("Thyroid_" + std::to_string(organID), density, 8);
                baseMat->AddElement(H, 0.104);
                baseMat->AddElement(C, 0.119);
                baseMat->AddElement(N, 0.024);
                baseMat->AddElement(O, 0.745);
                baseMat->AddElement(Na, 0.002);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.002);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            case 32: {
                baseMat = new G4Material("BoneCortical_" + std::to_string(organID), density, 9);
                baseMat->AddElement(H, 0.035);
                baseMat->AddElement(C, 0.160);
                baseMat->AddElement(N, 0.042);
                baseMat->AddElement(O, 0.435);
                baseMat->AddElement(Na, 0.003);
                baseMat->AddElement(P, 0.050);
                baseMat->AddElement(S, 0.009);
                baseMat->AddElement(Cl, 0.003);
                baseMat->AddElement(Ca, 0.080);
                break;
            }
            case 33: {
                baseMat = new G4Material("BoneMarrow_" + std::to_string(organID), density, 7);
                baseMat->AddElement(H, 0.105);
                baseMat->AddElement(C, 0.144);
                baseMat->AddElement(N, 0.034);
                baseMat->AddElement(O, 0.439);
                baseMat->AddElement(P, 0.001);
                baseMat->AddElement(S, 0.003);
                baseMat->AddElement(Cl, 0.002);
                break;
            }
            default: {
                G4cerr << "Warning: Unknown materialID " << materialID << " for organID " << organID << G4endl;
                baseMat = nistManager->FindOrBuildMaterial("G4_AIR");
                break;
            }
        }
        fMaterials[organID] = baseMat;
    }

    fMaterials[0] = nistManager->FindOrBuildMaterial("G4_AIR");
    for (G4int i = 0; i < 100; ++i) {
        if (!fMaterials[i]) {
            fMaterials[i] = fMaterials[0];
        }
    }

    fOrganToMaterial = organToMaterial;

    G4cout << "Created " << organToMaterial.size() << " organ-specific materials" << G4endl;
}

void PhantomParameterisation::GetVoxelIndices(const G4int copyNo, G4int& x, G4int& y, G4int& z) const {
    if (copyNo >= 0 && copyNo < static_cast<G4int>(fNonAirVoxels.size())) {
        x = fNonAirVoxels[copyNo].x;
        y = fNonAirVoxels[copyNo].y;
        z = fNonAirVoxels[copyNo].z;
    } else {
        x = y = z = 0;
    }
}

void PhantomParameterisation::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const {
    G4int x, y, z;
    GetVoxelIndices(copyNo, x, y, z);

    // 每个体素尺寸为1mm，位置计算需要调整
    // 模体总大小为282mm*141mm*468mm，将体素放置在模体内部
    
    // 计算体素中心相对于模体中心的位置
    G4double xPos = (x - fPhantomData->GetWidth()/2.0 + 0.5) * 1 * mm;
    G4double yPos = (y - fPhantomData->GetHeight()/2.0 + 0.5) * 1 * mm;
    G4double zPos = (z - fPhantomData->GetDepth()/2.0 + 0.5) * 2 * mm;

    physVol->SetTranslation(G4ThreeVector(xPos, yPos, zPos));
    physVol->SetRotation(nullptr);
}



void PhantomParameterisation::ComputeDimensions(G4Box& box, const G4int copyNo, const G4VPhysicalVolume*) const {
    // 体素尺寸为1mm*1mm*1mm的一半，即0.5mm*0.5mm*0.5mm
    box.SetXHalfLength(fVoxelHalfX);
    box.SetYHalfLength(fVoxelHalfY);
    box.SetZHalfLength(fVoxelHalfZ);
}

G4Material* PhantomParameterisation::ComputeMaterial(const G4int copyNo, G4VPhysicalVolume* physVol, const G4VTouchable* parentTouch) {
    if (copyNo >= 0 && copyNo < static_cast<G4int>(fNonAirVoxels.size())) {
        G4int organID = fNonAirVoxels[copyNo].organID;
        return fMaterials[organID];
    }
    return fMaterials[0];
}