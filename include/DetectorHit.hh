#ifndef DetectorHit_h
#define DetectorHit_h 1

#include "G4Allocator.hh"
#include "G4Step.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"

class DetectorHit : public G4VHit {
  public:
    DetectorHit();
    ~DetectorHit();
    DetectorHit(const DetectorHit& right);
    const DetectorHit& operator=(const DetectorHit& right);

    void AddEnergy(G4double edep);

    G4Step* GetStep() const;
    void SetStep(G4Step* step);

    void SetPosition(const G4ThreeVector& pos) { fPosition = pos; }
    G4ThreeVector GetPosition() const { return fPosition; }

    void SetEdep(G4double edep) { fEdep = edep; }
    G4double GetEdep() const { return fEdep; }

    void SetPos(const G4ThreeVector& pos);
    G4ThreeVector GetPos() const;

    void SetCopyNo(G4int no) { fCopyNo = no; }
    void SetWorldPos(const G4ThreeVector& pos) { fWorldPos = pos; }
    void SetLocalPos(const G4ThreeVector& pos) { fLocalPos = pos; }
    G4int GetCopyNo() const { return fCopyNo; }
    G4ThreeVector GetWorldPos() const { return fWorldPos; }
    G4ThreeVector GetLocalPos() const { return fLocalPos; }
    G4double GetEnergy() const { return fEnergy; }

    void SetDetectorID(G4int id);
    G4int GetDetectorID() const;
    void SetTrackID(G4int id) { fTrackID = id; }
    G4int GetTrackID() const { return fTrackID; }
    void SetIsGamma(G4bool isGamma) { fIsGamma = isGamma; }
    G4bool GetIsGamma() const { return fIsGamma; }

    void RecordStepInfo(const G4Step* step);
    const G4ThreeVector& GetPostStepPosition() const { return fPostStepPos; }
    G4bool IsPostStepValid() const { return fHasPostStep; }
    void SetIsScatter(G4bool isScatter) { fIsScatter = isScatter; }
    G4bool GetIsScatter() const { return fIsScatter; }

    void Draw() override;
    void Print() override;

  private:
    G4int fCopyNo = -1;
    G4ThreeVector fWorldPos;
    G4ThreeVector fLocalPos;
    G4double fEnergy = 0.0;

    G4ThreeVector fPos;
    G4double fEdep = 0.0;
    G4Step* fStep = nullptr;
    G4ThreeVector pos;
    G4ThreeVector fPostStepPos;
    G4bool fHasPostStep = false;
    G4bool fIsScatter = false;
    G4bool fIsGamma = false;
    G4int fDetectorID = -1;
    G4int fTrackID = -1;
    G4ThreeVector fPosition;
};

using DetectorHitsCollection = G4THitsCollection<DetectorHit>;

#endif
