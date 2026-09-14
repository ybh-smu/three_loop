#include "DetectorHit.hh"
#include <G4UnitsTable.hh>

// 线程局部存储的分配器（必须放在.cc文件中）
G4ThreadLocal G4Allocator<DetectorHit>* DetectorHitAllocator = nullptr;


// ========== DetectorHit 类的实现 ==========
DetectorHit::DetectorHit()
  : G4VHit(), 
    fPos(G4ThreeVector()), 
    fEdep(0.) 
{}

DetectorHit::DetectorHit(const DetectorHit& right)
  : G4VHit(right), 
    fCopyNo(right.fCopyNo),
    fWorldPos(right.fWorldPos),
    fLocalPos(right.fLocalPos),
    fEnergy(right.fEnergy),
    fPos(right.fPos),
    fEdep(right.fEdep),
    fStep(right.fStep),
    pos(right.pos),
    fPostStepPos(right.fPostStepPos),
    fHasPostStep(right.fHasPostStep),
    fIsScatter(right.fIsScatter),
    fIsGamma(right.fIsGamma),
    fDetectorID(right.fDetectorID),
    fTrackID(right.fTrackID),
    fPosition(right.fPosition)
{}



DetectorHit::~DetectorHit() {}

const DetectorHit& DetectorHit::operator=(const DetectorHit& right) {
    if (this == &right) {
        return *this;
    }
    fCopyNo = right.fCopyNo;
    fWorldPos = right.fWorldPos;
    fLocalPos = right.fLocalPos;
    fEnergy = right.fEnergy;
    fPos = right.fPos;
    fEdep = right.fEdep;
    fStep = right.fStep;
    pos = right.pos;
    fPostStepPos = right.fPostStepPos;
    fHasPostStep = right.fHasPostStep;
    fIsScatter = right.fIsScatter;
    fIsGamma = right.fIsGamma;
    fDetectorID = right.fDetectorID;
    fTrackID = right.fTrackID;
    fPosition = right.fPosition;
    return *this;
}


void DetectorHit::RecordStepInfo(const G4Step* step) {
    // 记录 PostStep 位置状态
    if (G4StepPoint* postPoint = step->GetPostStepPoint()) {
        fPostStepPos = postPoint->GetPosition();
        fHasPostStep = true;
    } else {
        fHasPostStep = false;
    }
    // 其他需要记录的信息（能量、轨迹等）
    AddEnergy(step->GetTotalEnergyDeposit());
}

void DetectorHit::SetStep(G4Step* step) {
    fStep = step;
}

// 获取 Step 指针
G4Step* DetectorHit::GetStep() const {
    return fStep;
}

void DetectorHit::SetPos(const G4ThreeVector& pos) {
    this->pos = pos;
}

G4ThreeVector DetectorHit::GetPos() const {
    return pos;
}

// 实现AddEnergy方法
void DetectorHit::AddEnergy(G4double edep) {
    fEdep += edep; 
}

void DetectorHit::SetDetectorID(G4int id) {
    fDetectorID = id;
}

G4int DetectorHit::GetDetectorID() const { // 必须有const修饰符
    return fDetectorID;
}
// 实现虚函数Draw和Print
void DetectorHit::Draw() {
    // Geant4要求实现，但一般无需实际操作
}

void DetectorHit::Print() {
    //G4cout << "Hit: Edep = " << G4BestUnit(fEdep, "Energy") 
         //  << ", Position = " << G4BestUnit(fPos, "Length") << G4endl;
}
