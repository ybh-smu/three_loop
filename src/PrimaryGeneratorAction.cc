#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "Randomize.hh"
#include "G4PhysicalConstants.hh" 
#include <cmath>                  

G4ThreeVector PrimaryGeneratorAction::fDefaultSourcePosition(-350 * mm, 0, 100 * mm);

PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* detector)
    : fDetector(detector),
      fSourceRadius(0.5 * mm),                 
      fSourcePosition(fDefaultSourcePosition) // Front by default
{
    InitializeMe();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
}


void PrimaryGeneratorAction::InitializeMe()
{
    fVerbose = true;
    fCounter = 0;
    fEnergy = 120 * keV; 

    // 加载能谱文件 
  std::ifstream file("../date/xcist_kVp120_tar7_bin1.dat");
    if (!file.is_open()) {
        G4cout << "能谱文件无法打开 Cannot open spectrum file xcist_kVp120_tar7_bin1.dat! Please check the file path." << G4endl;
        // 设置默认单能（120 keV）
        energies.clear();
        energies.push_back(50 * keV);
        probabilities.clear();
        probabilities.push_back(1.0);
        cumulativeProbabilities.clear();
        cumulativeProbabilities.push_back(1.0);
    } else {
        energies.clear();
        intensities.clear();

        std::string line;
        if (!std::getline(file, line)) {
            G4cout << "Error: Failed to read the first line of spectrum file!" << G4endl;
            file.close();
            energies.push_back(120 * keV);
            probabilities.push_back(1.0);
            cumulativeProbabilities.push_back(1.0);
            return;
        }

       
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string energyStr, intensityStr;

          
            if (!std::getline(iss, energyStr, ',') || !std::getline(iss, intensityStr)) {
                // 忽略无效行
                continue;
            }

           
            std::istringstream energyStream(energyStr);
            std::istringstream intensityStream(intensityStr);
            G4double energy, intensity;
            if (energyStream >> energy && intensityStream >> intensity) {
                energies.push_back(energy * keV);
                intensities.push_back(intensity);
            } else {
                continue;
            }

            if (energies.size() >= 120) break; 
        }
        file.close();

        
        if (energies.empty()) {
            G4cout << "Error: No valid data loaded from spectrum file!" << G4endl;
            energies.push_back(120 * keV);
            probabilities.push_back(1.0);
            cumulativeProbabilities.push_back(1.0);
        } else {
          
            G4double totalIntensity = 0.0;
            for (const auto& intensity : intensities) {
                totalIntensity += intensity;
            }

            if (totalIntensity <= 0.0) {
                G4cout << "Error: Total intensity is zero or negative!" << G4endl;
                energies.clear();
                energies.push_back(120 * keV);
                probabilities.push_back(1.0);
                cumulativeProbabilities.push_back(1.0);
            } else {
                // 计算概率并构建累积分布函数 (CDF)
                probabilities.clear();
                cumulativeProbabilities.clear();
                G4double cumulative = 0.0;
                for (const auto& intensity : intensities) {
                    G4double probability = intensity / totalIntensity;
                    probabilities.push_back(probability);
                    cumulative += probability;
                    cumulativeProbabilities.push_back(cumulative);
                }
            }
        }
    }

   
}

G4double PrimaryGeneratorAction::SampleEnergyFromSpectrum()
{
    if (cumulativeProbabilities.empty()) {
        return 120 * keV;  // 默认能量
    }

    G4double rand = G4UniformRand();
    for (size_t i = 0; i < cumulativeProbabilities.size(); ++i) {
        if (rand <= cumulativeProbabilities[i]) {
            return energies[i];
        }
    }
    
    // 如果没有找到匹配的，返回最后一个能量
    return energies.back();
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    fSourcePosition = fDefaultSourcePosition;
    fCounter++;
  if(fUseParallelBeam){
    G4double detHalfZ = 5.0 * cm;
    G4double detHalfY = 7.85 * cm;
    

    // --- 源面均匀采样：y,z 在 [-halfSide, +halfSide] ---
    G4double y = (2.0 * G4UniformRand() - 1.0) * detHalfY;
    G4double z = (2.0 * G4UniformRand() - 1.0) * detHalfZ;

    // --- 发射位置：在源平面 y-z 正方形内 ---
    G4ThreeVector position(
        fSourcePosition.x() + 3 * cm,
        fSourcePosition.y() + y,
        fSourcePosition.z() + z
    );

    // --- 平行束方向：全部沿 +X ---
    G4double dirX = 1.0;
    G4double dirY = 0.0;
    G4double dirZ = 0.0;

    // ↓↓↓ 下面这部分保持你原来的：创建 vertex / particle / 能量采样 / SetMomentumDirection 等 ↓↓↓
    G4PrimaryVertex* vertex = new G4PrimaryVertex(position, 0);

    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle("gamma");
    G4PrimaryParticle* primaryParticle = new G4PrimaryParticle(particle);

    G4double selectedEnergy = SampleEnergyFromSpectrum();
    primaryParticle->SetKineticEnergy(selectedEnergy);

    G4ThreeVector direction(dirX, dirY, dirZ);
    primaryParticle->SetMomentumDirection(direction);

    vertex->SetPrimary(primaryParticle);
    anEvent->AddPrimaryVertex(vertex);

    }
  else{
    // // === 在半径为 fSourceRadius 的圆形区域内均匀生成发射位置 ===
    // G4double y, z;
    // // 使用拒绝法在圆形区域内均匀取样
    // do {
    //     y = (2.0 * G4UniformRand() - 1.0) * fSourceRadius;
    //     z = (2.0 * G4UniformRand() - 1.0) * fSourceRadius;
    // } while (y*y + z*z > fSourceRadius * fSourceRadius);
    
    // // 发射位置（x坐标由fSourcePosition决定）
    // G4ThreeVector position(fSourcePosition.x(), 
    //                        fSourcePosition.y() + y, 
    //                        fSourcePosition.z() + z);
    
    // === 点源发射：固定在光源中心位置 ===
    G4ThreeVector position = fSourcePosition;

    // ================= 方形锥束：刚好覆盖探测器，并且每边多 1° 裕量 =================
    // 探测器几何参数
    G4double detR = 350.0 * mm;          // 探测器半径
    G4double spanPhi = 83.68 * deg;      // 探测器弧向总覆盖角
    G4double halfSpanPhi = spanPhi / 2.; // 探测器半弧角：41.84°
    G4double detZmin = -100.0 * mm;
    G4double detZmax =  100.0 * mm;
    // 自动读取当前射线源的Z位置
    G4double sourceZ = position.z();
    // 每边角度裕量
    G4double marginAngle = 1.0 * deg;
   // ================= Y方向角度范围 =================
   // 源在 (-R,0)，探测器弧段边缘在 phi = ±halfSpanPhi
   // 横向所需半角 thetaY = halfSpanPhi / 2
   G4double thetaYNeed = halfSpanPhi / 2.0;
   // 左右各多 1°
   G4double thetaYMin = -thetaYNeed - marginAngle;
   G4double thetaYMax =  thetaYNeed + marginAngle;
   // ================= Z方向角度范围 =================
   // Z方向最难覆盖点：探测器顶部 z=+100 mm，且位于弧段边缘 phi=±41.84°
   // 此时源点到探测器边缘的X向距离最小：dx = R * (1 + cos(halfSpanPhi))
   G4double dxMin = detR * (1.0 + std::cos(halfSpanPhi));
   // 源点到探测器Z上下边界的高度差
   G4double dzToZmin = detZmin - sourceZ;
   G4double dzToZmax = detZmax - sourceZ;
   // 对应的Z方向角度
   G4double thetaZToMin = std::atan(dzToZmin / dxMin);
   G4double thetaZToMax = std::atan(dzToZmax / dxMin);
   // 自动取较小值和较大值
   G4double thetaZNeedMin = std::min(thetaZToMin, thetaZToMax);
   G4double thetaZNeedMax = std::max(thetaZToMin, thetaZToMax);
   // 上下各多 1°
   G4double thetaZMin = thetaZNeedMin - marginAngle;
   G4double thetaZMax = thetaZNeedMax + marginAngle;
   // ================= 在角度范围内随机采样 =================
   G4double thetaY = thetaYMin + G4UniformRand() * (thetaYMax - thetaYMin);
   G4double thetaZ = thetaZMin + G4UniformRand() * (thetaZMax - thetaZMin);
   // ================= 构造方向向量 =================
   G4double dirX = 1.0;
   G4double dirY = std::tan(thetaY);
   G4double dirZ = std::tan(thetaZ);
   // 归一化
   G4double norm = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
   dirX /= norm;
   dirY /= norm;
   dirZ /= norm;
    
    
    
    // 创建主顶点
    G4PrimaryVertex* vertex = new G4PrimaryVertex(position, 0);
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle("gamma");
    G4PrimaryParticle* primaryParticle = new G4PrimaryParticle(particle);

    // 从能谱中随机选择能量
    G4double selectedEnergy = SampleEnergyFromSpectrum();
    primaryParticle->SetKineticEnergy(selectedEnergy);
    
    G4ThreeVector direction(dirX, dirY, dirZ);
    primaryParticle->SetMomentumDirection(direction);

    vertex->SetPrimary(primaryParticle);
    anEvent->AddPrimaryVertex(vertex);
    }
}

void PrimaryGeneratorAction::SetBeamEnergy(G4double val)
{
    fEnergy = val;
    if (fEnergy < 10*keV) {
        G4cout << "Warning: Beam energy is too low for X-ray source." << G4endl;
    }
}

void PrimaryGeneratorAction::SetSourcePosition(const G4ThreeVector& position)
{
    SetDefaultSourcePosition(position);
}

void PrimaryGeneratorAction::SetSourceView(const G4String& view)
{
    SetDefaultSourceView(view);
}

void PrimaryGeneratorAction::SetDefaultSourcePosition(const G4ThreeVector& position)
{
    fDefaultSourcePosition = position;
}

void PrimaryGeneratorAction::SetDefaultSourceView(const G4String& view)
{
    if (view == "Front" || view == "front") {
        fDefaultSourcePosition = G4ThreeVector(-350 * mm, 0, 100 * mm);
        return;
    }

    if (view == "Back" || view == "back") {
        fDefaultSourcePosition = G4ThreeVector(-350 * mm, 0, -100 * mm);
        return;
    }

    G4cout << "Warning: unknown source view '" << view
           << "'. Use Front or Back." << G4endl;
}
