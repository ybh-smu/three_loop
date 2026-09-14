#include "DetectorConstruction.hh"
#include "DetectorSD.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4PVParameterised.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4RunManager.hh"
#include "G4GeometryManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "globals.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4SubtractionSolid.hh" 
#include "DetectorConstructionMessenger.hh"
#include "DetectorMaterials.hh"
#include "PhantomData.hh"
#include "PhantomParameterisation.hh"
#include "CTPhantomData.hh"
#include "CTPhantomParameterisation.hh"
#include "BrainPhantomData.hh"
#include "BrainPhantomParameterisation.hh"

#include <algorithm>
#include <cmath>

DetectorConstruction::DetectorConstruction()
    : fVerbose(false), fMaxEnergy(120.0 * keV), fRotationMatrix(new G4RotationMatrix()), fCurrentAngle(0.0),
    fUsePhantom(false), fUseCt(false), fUseBrain(false), fPhantomData(nullptr), fCtPhantomData(nullptr),
    fUseCorgiDefrise(false), fBrainData(nullptr)
{

    DetectorMaterials materials;
    materials.DefineMaterials();
    fMessenger = new DetectorConstructionMessenger(this);
    
    
    G4NistManager *man = G4NistManager::Instance();
    fWorldMaterial = man->FindOrBuildMaterial("G4_AIR");
    fObjectMaterial = man->FindOrBuildMaterial("G4_WATER");
    // fDetectorMaterial = G4Material::GetMaterial("CsI_Tl");
    fDetectorMaterial = man->FindOrBuildMaterial("G4_CESIUM_IODIDE");


    materials.DefineMaterials();
    // 材料（请确保已定义）
    mat_Glass = man->FindOrBuildMaterial("G4_Pyrex_Glass");
    mat_Al    = man->FindOrBuildMaterial("G4_Al");
    mat_Mo    = man->FindOrBuildMaterial("G4_Mo");
    vacuum = man->FindOrBuildMaterial("G4_Galactic"); // 真空材料

    fDetectorMaterial = G4Material::GetMaterial("CsI_Tl");

    if (!fDetectorMaterial)
    {
        G4cerr << "Error: Material CsI_Tl not found!" << G4endl;
        // Fallback to avoid crash if custom material not found
        fDetectorMaterial = man->FindOrBuildMaterial("G4_CESIUM_IODIDE");
    }
    fCtPhantomMetaFile = "../date/cq500_ct418_hu_noborder.meta";
    fBrainPhantomFile = "../date/brain_meta.txt";
    fBrainWidth = 256;
    fBrainHeight = 256;
    fBrainDepth = 256;
    fBrainVoxelSizeX = 1.0 * mm;
    fBrainVoxelSizeY = 1.0 * mm;
    fBrainVoxelSizeZ = 1.0 * mm;
    fPhantomData = new PhantomData("../date/test1.dat");
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

DetectorConstruction::~DetectorConstruction()
{
    delete fBrainData;
    delete fCtPhantomData;
    delete fPhantomData;
    delete fRotationMatrix;
    delete fMessenger;
}

void DetectorConstruction::InitialiseGeometryParameters()
{
    // World大小
    worldSize = 200. * cm;
}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    InitialiseGeometryParameters();

    // 清理旧几何
    G4GeometryManager::GetInstance()->OpenGeometry();
    G4PhysicalVolumeStore::GetInstance()->Clean();
    G4LogicalVolumeStore::GetInstance()->Clean();
    G4SolidStore::GetInstance()->Clean();
    G4NistManager* nist = G4NistManager::Instance(); // 获取NIST材料管理器单例

    // ====================== 1. 世界体 ======================
    G4cout << "World volume created" << G4endl;
    G4Box *solidWorld = new G4Box("World", worldSize, worldSize, worldSize);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, fWorldMaterial, "World");
    fPhysWorld = new G4PVPlacement(nullptr, G4ThreeVector(), "World",
                                   logicWorld, nullptr, false, 0);

    // ====================== 2. 弧形探测器参数 (固定 0.2mm 像素尺寸) ======================

    // --- 半径与厚度 ---
    G4double detRmin = 350 * mm;      // 探测器内径
    G4double detThickness = 0.5 * mm; // 探测器厚度
    G4double detRmax = detRmin + detThickness;

    // --- 像素物理尺寸设定 ---
    G4double targetPixelArcLength = 0.5 * mm; // 目标横向(弧长)尺寸
    G4double targetPixelHeightZ = 0.5 * mm;   // 目标纵向(Z)尺寸

    // --- 计算单像素角度 ---
    // 弧长公式: L = R * theta (弧度)  =>  theta = L / R
    G4double deltaPhi = (targetPixelArcLength / detRmin) * rad;
    G4double deltaZ = targetPixelHeightZ;

    // --- 计算像素数量以维持大约 30° 的覆盖范围 ---
    G4double desiredSpanPhi = 83.68 * deg; // 希望保持的总覆盖角度

    // 自动计算横向像素数 (取整)
    // 例如：30度 / 单像素角度 ≈ 785 个像素
    const G4int nPixelTheta = std::round(desiredSpanPhi / deltaPhi);

    // 保持 Z 方向 100mm 总高 (100mm / 0.2mm = 500)
    const G4int nPixelZ = 400;

    // --- 反向计算精确的总角度跨度 ---
    // 确保总角度 = 像素数 * 单像素角度，避免浮点误差导致的重叠
    G4double spanPhi = nPixelTheta * deltaPhi;

    // 重新居中起始角度
    G4double startPhi = -spanPhi / 2.0;

    G4double detHalfZ = deltaZ * nPixelZ / 2.0;

    // --- 输出计算结果供检查 ---
    G4cout << "\n================ CONFIGURATION ================" << G4endl;
    G4cout << "Pixel Transverse Size (Arc) : " << targetPixelArcLength / mm << " mm" << G4endl;
    G4cout << "Pixel Longitudinal Size (Z) : " << deltaZ / mm << " mm" << G4endl;
    G4cout << "Detector Radius             : " << detRmin / mm << " mm" << G4endl;
    G4cout << "Single Pixel Angle          : " << deltaPhi / deg << " deg" << G4endl;
    G4cout << "Total Pixels (Theta x Z)    : " << nPixelTheta << " x " << nPixelZ << G4endl;
    G4cout << "Total Angle Span            : " << spanPhi / deg << " deg" << G4endl;
    G4cout << "===============================================\n"
           << G4endl;

    // ====================== 3. 母体（整块弧形探测器） ======================
    G4Tubs *solidDetector = new G4Tubs("Detector",
                                       detRmin,
                                       detRmax,
                                       detHalfZ,
                                       startPhi,
                                       spanPhi);

    fLogicDetector = new G4LogicalVolume(solidDetector,
                                         fDetectorMaterial,
                                         "Detector");

    // 红色实心显示（方便可视化）
    G4VisAttributes *detVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));
    detVis->SetForceWireframe(true);
    fLogicDetector->SetVisAttributes(detVis);

    // ====================== 4. 像素（敏感单元） ======================
    // 每个像素是一个很薄的扇形柱体
    G4Tubs *solidPixel = new G4Tubs("Pixel",
                                    detRmin,
                                    detRmax,
                                    deltaZ / 2.0,    // Z 半宽
                                    -deltaPhi / 2.0, // Phi 起始 (局部坐标系)
                                    deltaPhi);       // Phi 跨度

    // ====================== 5. 放置像素 ======================
    // ======================定义可视化属性======================
    G4VisAttributes *blueVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // 蓝色
    blueVis->SetForceWireframe(true);                                        // 实心显示，更易观察

    G4VisAttributes *invisibleVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.0));
    invisibleVis->SetVisibility(false); // 设为完全不可见

    currentLogic = new G4LogicalVolume(solidPixel, fDetectorMaterial, "PixelNormal");
    currentLogic->SetVisAttributes(invisibleVis);


    G4cout << "===  正在放置每个探测器单元像素，先列再层 ===" << G4endl;

    G4int copyNo = 0;
    for (G4int iz = 0; iz < nPixelZ; ++iz)
    {
        G4double zCenter = -detHalfZ + (iz + 0.5) * deltaZ;

        for (G4int iphi = 0; iphi < nPixelTheta; ++iphi)
        {

            // 计算全局角度位置
            G4double phiCenter = startPhi + (iphi + 0.5) * deltaPhi;

            // 旋转矩阵：让像素对准圆心
            G4RotationMatrix *rot = new G4RotationMatrix();
            rot->rotateZ(phiCenter);

            new G4PVPlacement(rot,
                              G4ThreeVector(0, 0, zCenter),
                              currentLogic,
                              "Pixel",
                              fLogicDetector,
                              false,
                              copyNo++,
                              false);
        }
    }

    // ====================== 6. 把弧形探测器放到世界中 ======================
    G4RotationMatrix *detPlacementRot = new G4RotationMatrix();
    new G4PVPlacement(detPlacementRot,
                      G4ThreeVector(0, 0, 0),
                      fLogicDetector,
                      "Detector",
                      logicWorld,
                      false,
                      0,
                      true);

    // ====================== 8. Object location ======================
    if (fUsePhantom) {
            if (!fPhantomData->ReadData()) {
                 G4Exception("DetectorConstruction::Construct", "PhantomError", 
                             FatalException, "无法读取模体数据！");
            }
        
        // 修改模体物理尺寸为 282mm*141mm*936mm
        G4double phantomSizeX = 282 * mm;
        G4double phantomSizeY = 141 * mm;
        G4double phantomSizeZ = 936 * mm;
        
        G4double s = 0.5;
        
        // 体素尺寸为 1mm*1mm*1mm
        G4double voxSizeX = 1 * mm;
        G4double voxSizeY = 1 * mm;
        G4double voxSizeZ = 2 * mm; 


        G4Box* phantomBox = new G4Box("PhantomBox", 
                                      phantomSizeX/2, 
                                      phantomSizeY/2, 
                                      phantomSizeZ/2);
                                      
        G4LogicalVolume* phantomLogical = new G4LogicalVolume(phantomBox, fWorldMaterial, "PhantomBox");
        
        
        G4VisAttributes* phantomVisAtt = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // 蓝色
        phantomVisAtt->SetForceWireframe(true); // 设置为线框模式
        phantomLogical->SetVisAttributes(phantomVisAtt);
        
        
        G4Box* voxelSolid = new G4Box("PhantomVoxel", voxSizeX/2, voxSizeY/2, voxSizeZ/2);
        G4LogicalVolume* voxelLogical = new G4LogicalVolume(voxelSolid, fWorldMaterial, "PhantomVoxel");
        
        // 体素设为不可见
        voxelLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
        
        PhantomParameterisation* voxelParam = new PhantomParameterisation(fPhantomData);

        G4int totalVoxels = voxelParam->GetNonAirVoxelCount();
        G4cout << "参数化体素数量： " << totalVoxels << "（仅非空气体素）" << G4endl;
        
        new G4PVParameterised("PhantomVoxel", voxelLogical, phantomLogical,
                              kUndefined, totalVoxels, voxelParam);
        
         // 确保旋转矩阵应用90度的旋转
        *fRotationMatrix = G4RotationMatrix();
        fRotationMatrix->rotateZ(fCurrentAngle); 
        const G4ThreeVector phantomTranslation(0, 0, -1 * cm);
        const G4ThreeVector worldOriginInPhantom =
        fRotationMatrix->inverse() * (G4ThreeVector() - phantomTranslation);
        fPhantomData->SaveOrthogonalSlicesMontageAsPGM("phantom_orthogonal_slices.pgm",
                                                       worldOriginInPhantom.x(),
                                                       worldOriginInPhantom.y(),
                                                       worldOriginInPhantom.z(),
                                                       voxSizeX,
                                                       voxSizeY,
                                                       voxSizeZ);
        
        fPhysObject = new G4PVPlacement(fRotationMatrix, phantomTranslation,
                                        phantomLogical, "PhantomObject", logicWorld, false, 0);
        
        logicObject = phantomLogical;
        
        G4cout << "已创建数字模体，大小为 " << phantomSizeX/mm << "mm × " 
               << phantomSizeY/mm << "mm × " << phantomSizeZ/mm << "mm" << G4endl;
        G4cout << "包含 " << totalVoxels << " 个非空气体素" << G4endl;
        G4cout << "创建的对象绕Z轴旋转 " << fCurrentAngle/deg << " 度" << G4endl;
        G4cout << "数字模体轮廓显示为线框，体素仍然设为不可见" << G4endl;
    }
    else if(fUseCorgiDefrise) {
        G4cout << "=== 开始创建CorgiDefrise Phantom ===" << G4endl;
        
        // 获取聚氨酯材料
        G4Material* polystyrene = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRU-4");
        if (!polystyrene) {
            G4Exception("DetectorConstruction::Construct", "MaterialError", 
                        FatalException, "无法找到G4_POLYSTYRENE材料！");
        }
        
        // 获取特氟龙(PTFE)材料
        G4Material* ptfe = nist->FindOrBuildMaterial("G4_TEFLON");
        if (!ptfe) {
            G4Exception("DetectorConstruction::Construct", "MaterialError", 
                        FatalException, "无法找到G4_TEFLON材料！");
        }
        
        // CorgiDefrise phantom参数
        const G4double cylinderRadius = 3.0 * cm;    // 圆柱半径
        const G4double cylinderLength = 10.0 * cm;   // 圆柱长度
        const G4double insertRadius = 3.5 * cm;      // 圆柱中心位置
        
        // 特氟龙圆盘参数
        const G4double diskRadius = 3.0 * cm;        // 圆盘半径
        const G4double diskThickness = 0.1 * cm;     // 圆盘厚度
        const G4double diskSpacing = 1.5 * mm;       // 圆盘间距
        
        // 创建聚苯乙烯圆柱体
        G4Tubs* solidCorgiDefriseCylinder = new G4Tubs("CorgiDefriseCylinder",
                                                       0.0 * cm,
                                                       cylinderRadius,
                                                       cylinderLength/2,
                                                       0 * deg,
                                                       360 * deg);
        
        // 创建逻辑体积
        G4LogicalVolume* logicCorgiDefriseCylinder = new G4LogicalVolume(solidCorgiDefriseCylinder,
                                                                         polystyrene,
                                                                         "CorgiDefriseCylinder");
        
        // 设置聚苯乙烯圆柱体的视觉属性 - 红色
        G4VisAttributes* corgiDefriseVisAtt = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.8));
        corgiDefriseVisAtt->SetForceSolid(true);
        logicCorgiDefriseCylinder->SetVisAttributes(corgiDefriseVisAtt);
        
        // 计算圆柱体位置
        G4double angle = 0.0 * deg;
        G4ThreeVector cylinderPosition = G4ThreeVector(insertRadius * std::cos(angle),
                                                       insertRadius * std::sin(angle),
                                                       0.0);
        
        // 放置聚苯乙烯圆柱体
        new G4PVPlacement(nullptr,
                          cylinderPosition,
                          logicCorgiDefriseCylinder,
                          "CorgiDefriseCylinder",
                          logicWorld,
                          false,
                          0,
                          true);
        
        // 创建特氟龙圆盘实体
        G4Tubs* solidDefriseDisk = new G4Tubs("DefriseDisk",
                                              0.0 * cm,
                                              diskRadius,
                                              diskThickness/2,
                                              0 * deg,
                                              360 * deg);
        
        // 创建逻辑体积
        G4LogicalVolume* logicDefriseDisk = new G4LogicalVolume(solidDefriseDisk,
                                                                ptfe,
                                                                "DefriseDisk");
        
        // 设置特氟龙圆盘的视觉属性 - 白色
        G4VisAttributes* diskVisAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.9));
        diskVisAtt->SetForceSolid(true);
        logicDefriseDisk->SetVisAttributes(diskVisAtt);
        
        // 计算八个圆盘的z位置
        // Z轴正半轴两个圆盘，距离原点3cm
        G4double centerPosZ = 3.0 * cm;  // 中心距离原点3cm
        G4double zPos1 = centerPosZ - diskSpacing/2 - diskThickness/2;  // 第一个圆盘（靠近原点）
        G4double zPos2 = centerPosZ + diskSpacing/2 + diskThickness/2;  // 第二个圆盘（远离原点）
        
        // Z轴负半轴两个圆盘，距离原点3cm
        G4double zPos3 = -centerPosZ + diskSpacing/2 + diskThickness/2;  // 第三个圆盘（靠近原点）
        G4double zPos4 = -centerPosZ - diskSpacing/2 - diskThickness/2;  // 第四个圆盘（远离原点）
        
        // 原点附近四个圆盘
        G4double zPos5 = 1.075 * cm;
        G4double zPos6 = 0.825 * cm;
        G4double zPos7 = -1.075 * cm;
        G4double zPos8 = -0.825 * cm;
        
        // 放置八个特氟龙圆盘到聚苯乙烯圆柱体内部（使用局部坐标系）
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos1),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_1",
                          logicCorgiDefriseCylinder,
                          false,
                          0,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos2),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_2",
                          logicCorgiDefriseCylinder,
                          false,
                          1,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos3),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_3",
                          logicCorgiDefriseCylinder,
                          false,
                          2,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos4),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_4",
                          logicCorgiDefriseCylinder,
                          false,
                          3,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos5),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_5",
                          logicCorgiDefriseCylinder,
                          false,
                          4,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos6),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_6",
                          logicCorgiDefriseCylinder,
                          false,
                          5,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos7),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_7",
                          logicCorgiDefriseCylinder,
                          false,
                          6,
                          true);
        
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0, 0, zPos8),
                          logicDefriseDisk,
                          "CorgiDefriseDisk_8",
                          logicCorgiDefriseCylinder,
                          false,
                          7,
                          true);
        
        G4cout << "=== CorgiDefrise Phantom创建完成 ===" << G4endl;
        G4cout << "聚苯乙烯圆柱半径: " << cylinderRadius/cm << " cm" << G4endl;
        G4cout << "聚苯乙烯圆柱长度: " << cylinderLength/cm << " cm" << G4endl;
        G4cout << "特氟龙圆盘半径: " << diskRadius/cm << " cm" << G4endl;
        G4cout << "特氟龙圆盘厚度: " << diskThickness/mm << " mm" << G4endl;
        G4cout << "特氟龙圆盘数量: 8" << G4endl;
        G4cout << "圆盘间距: " << diskSpacing/mm << " mm" << G4endl;
        G4cout << "Z轴正半轴圆盘位置: " << zPos2/mm << " mm, " << zPos1/mm << " mm" << G4endl;
        G4cout << "Z轴负半轴圆盘位置: " << zPos3/mm << " mm, " << zPos4/mm << " mm" << G4endl;
        G4cout << "原点附近圆盘位置: " << zPos5/mm << " mm, " << zPos6/mm << " mm, " << zPos7/mm << " mm, " << zPos8/mm << " mm" << G4endl;
        G4cout << "圆柱中心位置半径: " << insertRadius/cm << " cm (角度0度)" << G4endl;
        G4cout << "材料: 聚苯乙烯 (G4_POLYSTYRENE) + 特氟龙 (G4_TEFLON)" << G4endl;
           }
    else if (fUseBrain) {
        delete fBrainData;
        fBrainData = new BrainPhantomData(fBrainPhantomFile);

        if (!fBrainData->ReadData()) {
            G4Exception("DetectorConstruction::Construct",
                        "BrainPhantomError",
                        FatalException,
                        "Failed to read brain phantom data.");
        }
        fBrainData->SavePreviewPGM("brain_preview.pgm");
        fBrainWidth = fBrainData->GetWidth();
        fBrainHeight = fBrainData->GetHeight();
        fBrainDepth = fBrainData->GetDepth();
        fBrainVoxelSizeX = fBrainData->GetVoxelSizeX();
        fBrainVoxelSizeY = fBrainData->GetVoxelSizeY();
        fBrainVoxelSizeZ = fBrainData->GetVoxelSizeZ();

        const G4double phantomSizeX = fBrainWidth * fBrainVoxelSizeX;
        const G4double phantomSizeY = fBrainHeight * fBrainVoxelSizeY;
        const G4double phantomSizeZ = fBrainDepth * fBrainVoxelSizeZ;
        fCurrentObjectSizeX = phantomSizeX;
        fCurrentObjectSizeY = phantomSizeY;
        fCurrentObjectSizeZ = phantomSizeZ;

        G4Box* solidPhantom = new G4Box("PhantomContainer",
                                        phantomSizeX / 2.0 + 1.0 * mm,
                                        phantomSizeY / 2.0 + 1.0 * mm,
                                        phantomSizeZ / 2.0 + 1.0 * mm);
        G4LogicalVolume* logicPhantom =
            new G4LogicalVolume(solidPhantom, fWorldMaterial, "PhantomContainer");
        *fRotationMatrix = G4RotationMatrix();
        fRotationMatrix->rotateZ(90 * deg);
        fPhysObject = new G4PVPlacement(fRotationMatrix,
                                        G4ThreeVector(),
                                        logicPhantom,
                                        "PhantomContainer",
                                        logicWorld,
                                        false,
                                        0);

        G4Box* voxelSolid = new G4Box("Voxel",
                                      fBrainVoxelSizeX / 2.0,
                                      fBrainVoxelSizeY / 2.0,
                                      fBrainVoxelSizeZ / 2.0);
        G4LogicalVolume* voxelLogic = new G4LogicalVolume(voxelSolid, fWorldMaterial, "VoxelLogic");
        voxelLogic->SetVisAttributes(G4VisAttributes::GetInvisible());

        BrainPhantomParameterisation* param =
            new BrainPhantomParameterisation(fBrainData);

        new G4PVParameterised("BrainVoxels",
                              voxelLogic,
                              logicPhantom,
                              kUndefined,
                              param->GetNonAirVoxelCount(),
                              param);

        logicObject = logicPhantom;
        G4cout << "Brain phantom configured from " << fBrainPhantomFile << G4endl;
        G4cout << "Dimensions: " << fBrainWidth << " x " << fBrainHeight << " x " << fBrainDepth << G4endl;
        G4cout << "Voxel size: "
               << fBrainVoxelSizeX / mm << " x "
               << fBrainVoxelSizeY / mm << " x "
               << fBrainVoxelSizeZ / mm << " mm" << G4endl;
        G4cout << "Brain phantom physical size (mm): "
            << fCurrentObjectSizeX/mm << " x "
            << fCurrentObjectSizeY/mm << " x "
            << fCurrentObjectSizeZ/mm << G4endl;
    }

    else if (fUseCt) {
        const G4double phantomSizeX = fCtPhantomData->GetWidth() * fCtPhantomData->GetVoxelSizeX();
        const G4double phantomSizeY = fCtPhantomData->GetHeight() * fCtPhantomData->GetVoxelSizeY();
        const G4double phantomSizeZ = fCtPhantomData->GetDepth() * fCtPhantomData->GetVoxelSizeZ();
        fCurrentObjectSizeX = phantomSizeX;
        fCurrentObjectSizeY = phantomSizeY;
        fCurrentObjectSizeZ = phantomSizeZ;

        G4Box* solidPhantom = new G4Box("CTPhantomContainer",
                                        phantomSizeX / 2.0 + 1.0 * mm,
                                        phantomSizeY / 2.0 + 1.0 * mm,
                                        phantomSizeZ / 2.0 + 1.0 * mm);
        G4LogicalVolume* logicPhantom =
            new G4LogicalVolume(solidPhantom, fWorldMaterial, "CTPhantomContainer");
        *fRotationMatrix = G4RotationMatrix();
        fRotationMatrix->rotateZ(fCurrentAngle);
        fPhysObject = new G4PVPlacement(fRotationMatrix,
                                        G4ThreeVector(),
                                        logicPhantom,
                                        "CTPhantomContainer",
                                        logicWorld,
                                        false,
                                        0);

        G4Box* voxelSolid = new G4Box("CTVoxel",
                                      fCtPhantomData->GetVoxelSizeX() / 2.0,
                                      fCtPhantomData->GetVoxelSizeY() / 2.0,
                                      fCtPhantomData->GetVoxelSizeZ() / 2.0);
        G4LogicalVolume* voxelLogic = new G4LogicalVolume(voxelSolid, fWorldMaterial, "CTVoxelLogic");
        voxelLogic->SetVisAttributes(G4VisAttributes::GetInvisible());

        CTPhantomParameterisation* param = new CTPhantomParameterisation(fCtPhantomData);
        new G4PVParameterised("CTVoxels",
                              voxelLogic,
                              logicPhantom,
                              kUndefined,
                              param->GetVoxelCount(),
                              param);

        logicObject = logicPhantom;
        G4cout << "CT phantom configured from " << fCtPhantomMetaFile << G4endl;
    }
    else {
        
        //水球体
        // G4Sphere *solidObject = new G4Sphere("Object",
        //                                       0 * cm,
        //                                      7.5 * cm,
        //                                       0 * deg,
        //                                      360 * deg,
        //                                      0 * deg,
        //                                      360 * deg);

        // logicObject = new G4LogicalVolume(solidObject,
        //                                   fObjectMaterial,
        //                                   "Object");

        // *fRotationMatrix = G4RotationMatrix();
        // fRotationMatrix->rotateZ(0 * deg);
        // fCurrentAngle = 0 * deg;

        // fPhysObject = new G4PVPlacement(fRotationMatrix, G4ThreeVector(0, 0, 0),
        //                            logicObject, "Object", logicWorld, false, 0);
        // G4cout << "=== Starting Detector Construction ===" << G4endl;
        // G4cout << "Object created with rotation: " << fCurrentAngle / deg << " degrees around Z axis" << G4endl;

        //水立方体
        G4Box *solidObject = new G4Box("Object",
                                8.0 * cm,   // x方向半边长，总长16 cm
                                8.0 * cm,   // y方向半边长，总长16 cm
                                8.0 * cm);  // z方向半边长，总长16 cm

        logicObject = new G4LogicalVolume(solidObject,
                                    fObjectMaterial,
                                    "Object");

            *fRotationMatrix = G4RotationMatrix();
            fRotationMatrix->rotateZ(0 * deg);
            fCurrentAngle = 0 * deg;

            fPhysObject = new G4PVPlacement(fRotationMatrix,
                                G4ThreeVector(0, 0, 0),
                                logicObject,
                                "Object",
                                logicWorld,
                                false,
                                0);

        //五个圆柱体水立方体的材料和位置设置
        // G4NistManager* nist = G4NistManager::Instance();
        // G4Element *elH = nist->FindOrBuildElement("H");
        // G4Element *elO = nist->FindOrBuildElement("O");
        // auto BuildWaterEquivalentMaterial =
        //     [&](const G4String &name, G4double density) -> G4Material *
        // {
        //     G4Material *mat = G4Material::GetMaterial(name, false);
        //     if (mat)
        //         return mat;

        //     mat = new G4Material(name, density, 2);
        //     mat->AddElement(elH, 2);
        //     mat->AddElement(elO, 1);
        //     return mat;
        // };
        // // 这里用不同密度的 water-equivalent material 来近似不同线性衰减系数
        // // 对应 Python 中 0.035, 0.040, 0.045, 0.050, 0.055 五个圆柱灰度/衰减值
        // G4Material *cylMat0 = BuildWaterEquivalentMaterial("Cyl_Material_0035", 1.75 * g / cm3);
        // G4Material *cylMat1 = BuildWaterEquivalentMaterial("Cyl_Material_0040", 2.00 * g / cm3);
        // G4Material *cylMat2 = BuildWaterEquivalentMaterial("Cyl_Material_0045", 2.25 * g / cm3);
        // G4Material *cylMat3 = BuildWaterEquivalentMaterial("Cyl_Material_0050", 2.50 * g / cm3);
        // G4Material *cylMat4 = BuildWaterEquivalentMaterial("Cyl_Material_0055", 2.75 * g / cm3);
    
        // // ================================
        // // 2. 创建外部水正方体
        // // ================================
        // G4Box *solidObject = new G4Box("Object",
        //                             8.0 * cm,  // x 半边长，总长 16 cm
        //                             8.0 * cm,  // y 半边长，总长 16 cm
        //                             8.0 * cm); // z 半边长，总长 16 cm

        // logicObject = new G4LogicalVolume(solidObject,
        //                                 fObjectMaterial,
        //                                 "Object");

        // // ================================
        // // 3. 创建 5 个沿 z 方向贯穿的圆柱体
        // // ================================
        // G4double cylRadius = 1.0 * cm; // 半径 10 mm
        // G4double cylHalfZ = 8.0 * cm;  // 半高 8 cm，总高 16 cm

        // G4Material *cylMats[5] = {
        //     cylMat0,
        //     cylMat1,
        //     cylMat2,
        //     cylMat3,
        //     cylMat4};

        // // 对应 Python 中 centers_cm:
        // // (0,0), (0,5), (5,0), (-5,0), (0,-5)
        // G4double cylX[5] = {
        //     0.0 * cm,
        //     0.0 * cm,
        //     5.0 * cm,
        //     -5.0 * cm,
        //     0.0 * cm};

        // G4double cylY[5] = {
        //     0.0 * cm,
        //     5.0 * cm,
        //     0.0 * cm,
        //     0.0 * cm,
        //     -5.0 * cm};

        // for (G4int i = 0; i < 5; ++i)
        // {
        //     G4String solidName = "CylinderSolid_" + std::to_string(i);
        //     G4String logicName = "CylinderLogic_" + std::to_string(i);
        //     G4String physName = "CylinderPhys_" + std::to_string(i);

        //     G4Tubs *solidCylinder = new G4Tubs(solidName,
        //                                     0.0 * cm,  // 内半径
        //                                     cylRadius, // 外半径
        //                                     cylHalfZ,  // z 方向半高
        //                                     0.0 * deg,
        //                                     360.0 * deg);

        //     G4LogicalVolume *logicCylinder =
        //         new G4LogicalVolume(solidCylinder,
        //                             cylMats[i],
        //                             logicName);

        //     // 注意：圆柱作为 Object 的子体积放置
        //     // 因此坐标是相对于正方体中心的局部坐标
        //     new G4PVPlacement(nullptr,
        //                     G4ThreeVector(cylX[i], cylY[i], 0.0 * cm),
        //                     logicCylinder,
        //                     physName,
        //                     logicObject,
        //                     false,
        //                     i,
        //                     true);
        // }

        // // ================================
        // // 4. 将整个体模放入世界坐标系
        // // ================================
        // *fRotationMatrix = G4RotationMatrix();
        // fRotationMatrix->rotateZ(0* deg);
        // fCurrentAngle = 0 * deg;

        // fPhysObject = new G4PVPlacement(fRotationMatrix,
        //                                 G4ThreeVector(0, 0, 0),
        //                                 logicObject,
        //                                 "Object",
        //                                 logicWorld,
        //                                 false,
        //                                 0,
        //                                 true);

        }


    return fPhysWorld;
}

void DetectorConstruction::SetObjectRotationAngle(G4double angle)
{
    fCurrentAngle = angle;
    if (fPhysObject)
    {
        *fRotationMatrix = G4RotationMatrix();
        fRotationMatrix->rotateZ(angle);           // 绕 Z 轴旋转指定角度
        fPhysObject->SetRotation(fRotationMatrix); // 更新物体旋转
      
        G4RunManager::GetRunManager()->GeometryHasBeenModified();
        G4cout << "Object rotated by " << angle / deg << " degrees around Z axis" << G4endl;
    }
}

G4double DetectorConstruction::GetCurrentAngle() const
{
    return fCurrentAngle;
}

void DetectorConstruction::SetCtPhantomMetaFile(const G4String& path)
{
    fCtPhantomMetaFile = path;
}

void DetectorConstruction::SetBrainPhantomFile(const G4String& path)
{
    fBrainPhantomFile = path;
}

void DetectorConstruction::SetBrainPhantomDimensions(G4int width, G4int height, G4int depth)
{
    fBrainWidth = width;
    fBrainHeight = height;
    fBrainDepth = depth;
}

void DetectorConstruction::SetBrainVoxelSize(G4double sizeX, G4double sizeY, G4double sizeZ)
{
    fBrainVoxelSizeX = sizeX;
    fBrainVoxelSizeY = sizeY;
    fBrainVoxelSizeZ = sizeZ;
}

void DetectorConstruction::ConstructSDandField()
{
    G4SDManager *sdManager = G4SDManager::GetSDMpointer();
    // 确保 SD 名字唯一，避免重复添加错误
    G4String sdName = "detectorSD";
    DetectorSD *fDetectorSD = new DetectorSD(sdName);
    sdManager->AddNewDetector(fDetectorSD);
    // 只有像素逻辑体被设为敏感探测器
    if(currentLogic) {
        currentLogic->SetSensitiveDetector(fDetectorSD);
    }
}
