#include "EventAction.hh"

#include <CLHEP/Units/PhysicalConstants.h>
#include <map>
#include <set>

#include "DetectorConstruction.hh"
#include "DetectorHit.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4THitsCollection.hh"
#include "G4VHitsCollection.hh"
#include "Run.hh"
#include "TrackerHit.hh"

EventAction::EventAction(DetectorConstruction* detector)
  : G4UserEventAction(),
    fAnalysisManager(G4AnalysisManager::Instance()),
    fDetectorEdep(),
    fDetector(detector)
{}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event* event)
{
  (void) event;
  fDetectorEdep.clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  std::map<G4int, G4double> detectorEdepTotal;
  std::map<G4int, G4double> detectorEdepPrim;
  std::map<G4int, G4double> detectorEdepScatter;
  std::map<G4int, std::set<G4int>> detectorGammaTracksTotal;
  std::map<G4int, std::set<G4int>> detectorGammaTracksPrimary;
  std::map<G4int, std::set<G4int>> detectorGammaTracksScatter;

  G4HCofThisEvent* hce = event->GetHCofThisEvent();
  if (!hce) {
    G4cout << "No hits collection in this event!" << G4endl;
    return;
  }

  auto* hitsCollection = static_cast<G4THitsCollection<DetectorHit>*>(hce->GetHC(0));
  if (!hitsCollection) {
    G4cout << "Failed to get DetectorHit collection or wrong HC ID!" << G4endl;
    return;
  }

  for (G4int i = 0; i < hitsCollection->entries(); ++i) {
    DetectorHit* hit = (*hitsCollection)[i];
    if (!hit) {
      continue;
    }

    const G4int detectorID = hit->GetDetectorID();
    const G4int trackID = hit->GetTrackID();
    const G4bool isGamma = hit->GetIsGamma();
    const G4double edep = hit->GetEdep();
    if (edep <= 0.) {
      continue;
    }

    detectorEdepTotal[detectorID] += edep;
    if (isGamma) {
      detectorGammaTracksTotal[detectorID].insert(trackID);
    }
    if (hit->GetIsScatter()) {
      detectorEdepScatter[detectorID] += edep;
      if (isGamma) {
        detectorGammaTracksScatter[detectorID].insert(trackID);
      }
    } else {
      detectorEdepPrim[detectorID] += edep;
      if (isGamma) {
        detectorGammaTracksPrimary[detectorID].insert(trackID);
      }
    }
  }

  for (const auto& entry : detectorEdepTotal) {
    const G4int detectorID = entry.first;
    const G4double total = entry.second * 100.;
    const G4double prim =
        detectorEdepPrim.count(detectorID) ? detectorEdepPrim[detectorID] * 100. : 0.;
    const G4double scatter =
        detectorEdepScatter.count(detectorID) ? detectorEdepScatter[detectorID] * 100. : 0.;
    const G4double totalCount =
        detectorGammaTracksTotal.count(detectorID)
            ? static_cast<G4double>(detectorGammaTracksTotal[detectorID].size())
            : 0.;
    const G4double primaryCount =
        detectorGammaTracksPrimary.count(detectorID)
            ? static_cast<G4double>(detectorGammaTracksPrimary[detectorID].size())
            : 0.;
    const G4double scatterCount =
        detectorGammaTracksScatter.count(detectorID)
            ? static_cast<G4double>(detectorGammaTracksScatter[detectorID].size())
            : 0.;

    auto* currentRun = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    if (currentRun != nullptr) {
      currentRun->AddProjection(
          detectorID, total, prim, scatter, totalCount, primaryCount, scatterCount);
    }
  }
}
