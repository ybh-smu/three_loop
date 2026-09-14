#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"
#include <vector>
#include <fstream>
#include <sstream>
#include "G4SystemOfUnits.hh"

class G4Event;
class DetectorConstruction;
class G4ParticleDefinition;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    // 构造函数
    PrimaryGeneratorAction(DetectorConstruction* detector);
    
    // 析构函数
    virtual ~PrimaryGeneratorAction();
    
    // 主要的粒子生成函数
    void GeneratePrimaries(G4Event* anEvent);
    
    // 设置光束能量（保留接口，但主要由能谱文件决定）
    void SetBeamEnergy(G4double val);
    void SetSourcePosition(const G4ThreeVector& position);
    void SetSourceView(const G4String& view);
    static void SetDefaultSourcePosition(const G4ThreeVector& position);
    static void SetDefaultSourceView(const G4String& view);

private:
    // 初始化函数
    void InitializeMe();
    
    // 从能谱文件随机选择能量
    G4double SampleEnergyFromSpectrum();

private:
    DetectorConstruction* fDetector;
    static G4ThreeVector fDefaultSourcePosition;
    
    // 能谱相关数据
    std::vector<G4double> energies;
    std::vector<G4double> intensities;
    std::vector<G4double> probabilities;
    std::vector<G4double> cumulativeProbabilities;
    
    // 圆形锥形光源参数
    G4double fSourceRadius;      // 圆形源的半径
    G4double fConeAngle;         // 锥束的半角
    G4ThreeVector fSourcePosition;  // 光源中心位置
    
    // 其他参数
    G4double fEnergy;
    G4bool fVerbose;
    G4int fCounter;
    // === 新增：控制束型 ===
    G4bool fUseParallelBeam = false; 
};

#endif // PrimaryGeneratorAction_h
