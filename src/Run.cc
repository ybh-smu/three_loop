//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file medical/GammaTherapy/src/Run.cc
/// \brief Implementation of the Run class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "Run.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "HistoManager.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4DataVector.hh"
#include "G4EmCalculator.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <iomanip>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
Run::Run(DetectorConstruction* det)
  : G4Run(), fDetector(det)
{
  // G4cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << G4endl;

  fAnalysisManager = G4AnalysisManager::Instance();



  fMaxEnergy = fDetector->GetMaxEnergy();

  fNBinsE    = fDetector->GetNumberDivE();
  fMaxEnergy = 120.0 * MeV;

  fStepE    = fMaxEnergy / (G4double) fNBinsE;
  fScoreBin = (G4int)(fScoreZ / fStepZ + 0.5);

  fVerbose = fDetector->GetVerbose();
  fVerbose = true;

  fGamma    = G4Gamma::Gamma();
  fElectron = G4Electron::Electron();
  fPositron = G4Positron::Positron();

  fVolumeR.clear();
  fEdep.clear();
  fGammaE.clear();
  fProjectionTotal.clear();
  fProjectionPrim.clear();
  fProjectionScatter.clear();
  fCountTotal.clear();
  fCountPrimary.clear();
  fCountScatter.clear();

  const G4int nDetectorBins =
      G4SDManager::GetSDMpointer()->GetCollectionCapacity() > 0 ? 408800 : 408800;
  fProjectionTotal.resize(nDetectorBins, 0.0);
  fProjectionPrim.resize(nDetectorBins, 0.0);
  fProjectionScatter.resize(nDetectorBins, 0.0);
  fCountTotal.resize(nDetectorBins, 0.0);
  fCountPrimary.resize(nDetectorBins, 0.0);
  fCountScatter.resize(nDetectorBins, 0.0);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::~Run() {}

void Run::AddProjection(G4int detectorID, G4double total, G4double prim, G4double scatter,
                        G4double totalCount, G4double primaryCount, G4double scatterCount)
{
  if (detectorID < 0) {
    return;
  }

  if (detectorID >= static_cast<G4int>(fProjectionTotal.size())) {
    const size_t newSize = static_cast<size_t>(detectorID + 1);
    fProjectionTotal.resize(newSize, 0.0);
    fProjectionPrim.resize(newSize, 0.0);
    fProjectionScatter.resize(newSize, 0.0);
    fCountTotal.resize(newSize, 0.0);
    fCountPrimary.resize(newSize, 0.0);
    fCountScatter.resize(newSize, 0.0);
  }

  fProjectionTotal[detectorID] += total;
  fProjectionPrim[detectorID] += prim;
  fProjectionScatter[detectorID] += scatter;
  fCountTotal[detectorID] += totalCount;
  fCountPrimary[detectorID] += primaryCount;
  fCountScatter[detectorID] += scatterCount;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(const G4Run* run)
{
  // G4cout << "222222222222222222222222222222222222" << G4endl;
  const Run* localRun = static_cast<const Run*>(run);
  for(int i = 0; i < (int) localRun->fProjectionTotal.size(); i++) {
    if (i >= (int) fProjectionTotal.size()) {
      fProjectionTotal.resize(i + 1, 0.0);
      fProjectionPrim.resize(i + 1, 0.0);
      fProjectionScatter.resize(i + 1, 0.0);
      fCountTotal.resize(i + 1, 0.0);
      fCountPrimary.resize(i + 1, 0.0);
      fCountScatter.resize(i + 1, 0.0);
    }
    fProjectionTotal[i] += localRun->fProjectionTotal[i];
    fProjectionPrim[i] += localRun->fProjectionPrim[i];
    fProjectionScatter[i] += localRun->fProjectionScatter[i];
    fCountTotal[i] += localRun->fCountTotal[i];
    fCountPrimary[i] += localRun->fCountPrimary[i];
    fCountScatter[i] += localRun->fCountScatter[i];
  }

  G4Run::Merge(run);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::EndOfRun()
{
  // G4cout << "33333333333333333333333333333333333" << G4endl;
  // G4cout << "Histo: End of run actions are started" << G4endl;

  // average
  // G4cout << "========================================================"<< G4endl;

}
