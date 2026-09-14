#include "DetectorMaterials.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4SystemOfUnits.hh"

DetectorMaterials::DetectorMaterials() {}

DetectorMaterials::~DetectorMaterials() {}

void DetectorMaterials::DefineMaterials()
{
    // 获取NIST材料管理器
    G4NistManager* nist = G4NistManager::Instance();

    // 定义基本元素
    G4Element* elCs = new G4Element("Cesium", "Cs", 55., 132.90545*g/mole);
    G4Element* elI  = new G4Element("Iodine", "I", 53., 126.90447*g/mole);
    G4Element* elTl = new G4Element("Thallium", "Tl", 81., 204.3833*g/mole);

    // 定义纯CsI材料
    G4Material* CsI = new G4Material("CsI", 4.51*g/cm3, 2);
    CsI->AddElement(elCs, 1);
    CsI->AddElement(elI, 1);

    // 定义掺杂了Tl的 CsI_Tl 材料
    G4double density = 4.51*g/cm3;
    G4int ncomponents = 3;
    G4Material* CsI_Tl = new G4Material("CsI_Tl", density, ncomponents);

    G4double wTl  = 0.002;     // Tl 质量分数
    G4double wCsI = 1.0 - wTl; // CsI 质量分数
    G4double wCs  = 0.5 * wCsI; // Cs 占比
    G4double wI   = 0.5 * wCsI; // I 占比

    CsI_Tl->AddElement(elCs, wCs);
    CsI_Tl->AddElement(elI,  wI);
    CsI_Tl->AddElement(elTl, wTl);

    // 可选：打印材料信息，确认定义正确
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}
