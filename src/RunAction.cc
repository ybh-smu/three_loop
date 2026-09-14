#include "RunAction.hh"

#include <iomanip>
#include <cmath>
#include <sstream>

#include "DetectorConstruction.hh"
#include "G4AnalysisManager.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "HistoManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "Randomize.hh"
#include "Run.hh"
#include "globals.hh"

G4String RunAction::fOutputDirectory = "../output3";
G4String RunAction::fOutputPrefix = "Front";

RunAction::RunAction(DetectorConstruction* det, PrimaryGeneratorAction* prim)
  : G4UserRunAction(),
    fDetector(det),
    fRun(nullptr),
    fHistoManager(nullptr),
    fPrimary(prim)
{
  fHistoManager = new HistoManager();
}

RunAction::~RunAction()
{
  delete fHistoManager;
}

G4Run* RunAction::GenerateRun()
{
  fRun = new Run(fDetector);
  return fRun;
}

void RunAction::BeginOfRunAction(const G4Run* run)
{
  (void) run;
  G4AnalysisManager::Instance()->Reset();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  if (!isMaster) {
    return;
  }

  fRun->EndOfRun();
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  const Run* mergedRun = static_cast<const Run*>(run);
  const G4DataVector& total = mergedRun->GetProjectionTotal();
  const G4DataVector& prim = mergedRun->GetProjectionPrim();
  const G4DataVector& scatter = mergedRun->GetProjectionScatter();
  const G4DataVector& totalCount = mergedRun->GetCountTotal();
  const G4DataVector& primaryCount = mergedRun->GetCountPrimary();
  const G4DataVector& scatterCount = mergedRun->GetCountScatter();
  
  // Get current angle from detector (in radians)
  G4double angleRad = fDetector->GetCurrentAngle();
  // Convert to degrees for display and filename
  G4double angleDeg = angleRad * (180.0 / CLHEP::pi);
  std::ostringstream prefix;
  prefix << fOutputDirectory;
  if (!fOutputDirectory.empty() &&
      fOutputDirectory.back() != '/' &&
      fOutputDirectory.back() != '\\') {
    prefix << "/";
  }
  prefix << fOutputPrefix
         << "_"
         << std::setfill('0') << std::setw(3) << static_cast<int>(std::round(angleDeg))<<"_";

  for (G4int i = 0; i < static_cast<G4int>(total.size()); ++i) {
    if (total[i] > 0.) {
      analysisManager->FillH1(0, i, total[i]);
    }
  }
  for (G4int i = 0; i < static_cast<G4int>(prim.size()); ++i) {
    if (prim[i] > 0.) {
      analysisManager->FillH1(1, i, prim[i]);
    }
  }
  for (G4int i = 0; i < static_cast<G4int>(scatter.size()); ++i) {
    if (scatter[i] > 0.) {
      analysisManager->FillH1(2, i, scatter[i]);
    }
  }
  for (G4int i = 0; i < static_cast<G4int>(totalCount.size()); ++i) {
    if (totalCount[i] > 0.) {
      analysisManager->FillH1(3, i, totalCount[i]);
    }
  }
  for (G4int i = 0; i < static_cast<G4int>(primaryCount.size()); ++i) {
    if (primaryCount[i] > 0.) {
      analysisManager->FillH1(4, i, primaryCount[i]);
    }
  }
  for (G4int i = 0; i < static_cast<G4int>(scatterCount.size()); ++i) {
    if (scatterCount[i] > 0.) {
      analysisManager->FillH1(5, i, scatterCount[i]);
    }
  }

  analysisManager->SetH1Activation(0, true);
  analysisManager->SetH1Activation(1, false);
  analysisManager->SetH1Activation(2, false);
  analysisManager->SetH1Activation(3, false);
  analysisManager->SetH1Activation(4, false);
  analysisManager->SetH1Activation(5, false);
  analysisManager->OpenFile(prefix.str() + "total.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, false);
  analysisManager->SetH1Activation(1, true);
  analysisManager->SetH1Activation(2, false);
  analysisManager->SetH1Activation(3, false);
  analysisManager->SetH1Activation(4, false);
  analysisManager->SetH1Activation(5, false);
  analysisManager->OpenFile(prefix.str() + "prim.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, false);
  analysisManager->SetH1Activation(1, false);
  analysisManager->SetH1Activation(2, true);
  analysisManager->SetH1Activation(3, false);
  analysisManager->SetH1Activation(4, false);
  analysisManager->SetH1Activation(5, false);
  analysisManager->OpenFile(prefix.str() + "scatter.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, false);
  analysisManager->SetH1Activation(1, false);
  analysisManager->SetH1Activation(2, false);
  analysisManager->SetH1Activation(3, true);
  analysisManager->SetH1Activation(4, false);
  analysisManager->SetH1Activation(5, false);
  analysisManager->OpenFile(prefix.str() + "total_count.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, false);
  analysisManager->SetH1Activation(1, false);
  analysisManager->SetH1Activation(2, false);
  analysisManager->SetH1Activation(3, false);
  analysisManager->SetH1Activation(4, true);
  analysisManager->SetH1Activation(5, false);
  analysisManager->OpenFile(prefix.str() + "primary_count.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, false);
  analysisManager->SetH1Activation(1, false);
  analysisManager->SetH1Activation(2, false);
  analysisManager->SetH1Activation(3, false);
  analysisManager->SetH1Activation(4, false);
  analysisManager->SetH1Activation(5, true);
  analysisManager->OpenFile(prefix.str() + "scatter_count.root");
  analysisManager->Write();
  analysisManager->CloseFile(false);

  analysisManager->SetH1Activation(0, true);
  analysisManager->SetH1Activation(1, true);
  analysisManager->SetH1Activation(2, true);
  analysisManager->SetH1Activation(3, true);
  analysisManager->SetH1Activation(4, true);
  analysisManager->SetH1Activation(5, true);

  G4Random::showEngineStatus();
}

void RunAction::SetOutputDirectory(const G4String& directory)
{
  SetDefaultOutputDirectory(directory);
}

void RunAction::SetOutputPrefix(const G4String& prefix)
{
  SetDefaultOutputPrefix(prefix);
}

void RunAction::SetDefaultOutputDirectory(const G4String& directory)
{
  fOutputDirectory = directory;
}

void RunAction::SetDefaultOutputPrefix(const G4String& prefix)
{
  
  if (prefix.empty()) {
    G4cout << "Warning: empty output prefix ignored." << G4endl;
    return;
  }

  fOutputPrefix = prefix;

  G4cout << "use output prefix '" << prefix << G4endl;
}
