//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "HistoManager.hh"
#include "DetectorConstruction.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

HistoManager::HistoManager()
  : fFileName("GammaTherapy.root")
{


  Book();
}

HistoManager::~HistoManager() {}

void HistoManager::Book()
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->CreateH1(
      "0", "Detector unit energy deposition (total)", 408800, 0, 408800);
  analysisManager->CreateH1(
      "0", "Detector unit energy deposition (primary)", 408800, 0, 408800);
  analysisManager->CreateH1(
      "0", "Detector unit energy deposition (scatter)", 408800, 0, 408800);
  analysisManager->CreateH1(
      "0", "Detector gamma counts (total)", 408800, 0, 408800);
  analysisManager->CreateH1(
      "0", "Detector gamma counts (primary)", 408800, 0, 408800);
  analysisManager->CreateH1(
      "0", "Detector gamma counts (scatter)", 408800, 0, 408800);
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetActivation(true);
}
