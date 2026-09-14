//
// ********************************************************************
// * License and Disclaimer                                           *
// ********************************************************************

#include "DetectorSD.hh"

#include "DetectorHit.hh"
#include "G4Gamma.hh"
#include "G4LogicalVolume.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4Positron.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4THitsCollection.hh"
#include "G4Track.hh"
#include "G4VHitsCollection.hh"
#include "G4VPhysicalVolume.hh"
#include "Run.hh"
#include "TrackInformation.hh"
#include "globals.hh"

G4ThreadLocal DetectorSD* DetectorSD::instance = nullptr;

DetectorSD::DetectorSD(const G4String& name)
  : G4VSensitiveDetector(name),
    PixelWithHoles(true),
    fHitsCollection(nullptr),
    hcID(-1)
{
  collectionName.push_back("DetectorHitsCollection");
}

DetectorSD::~DetectorSD() {}

void DetectorSD::Initialize(G4HCofThisEvent* hce)
{
  if (!instance) {
    instance = this;
  }

  fHitsCollection =
      new G4THitsCollection<DetectorHit>(SensitiveDetectorName, collectionName[0]);

  if (hcID < 0) {
    hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  }

  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool DetectorSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{

  const G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep <= 0.) {
    return false;
  }

  auto* touchable =
      (G4TouchableHistory*)(aStep->GetPreStepPoint()->GetTouchable());
  const G4int detectorID = touchable->GetCopyNumber();
  const G4int copyNo = touchable->GetCopyNumber();
  const G4int trackID = aStep->GetTrack()->GetTrackID();

  const G4ThreeVector worldPos = aStep->GetPostStepPoint()->GetPosition();
  const G4ThreeVector localPos =
      touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);
  const G4ThreeVector pos = aStep->GetPostStepPoint()->GetPosition();

  DetectorHit* hit = new DetectorHit();
  const auto* trackInfo =
      dynamic_cast<const TrackInformation*>(aStep->GetTrack()->GetUserInformation());
  const G4bool isGammaTrack = (aStep->GetTrack()->GetDefinition() == G4Gamma::Gamma());
  hit->SetDetectorID(detectorID);
  hit->SetTrackID(trackID);
  hit->SetIsGamma(isGammaTrack);
  hit->SetIsScatter(trackInfo != nullptr && trackInfo->HasScattered());
  hit->RecordStepInfo(aStep);
  hit->SetCopyNo(copyNo);
  hit->SetWorldPos(worldPos);
  hit->SetLocalPos(localPos);
  hit->SetPosition(pos);

  fHitsCollection->insert(hit);
  return true;
}

void DetectorSD::EndOfEvent(G4HCofThisEvent*) {}

void DetectorSD::clear() {}

void DetectorSD::PrintAll() {}
