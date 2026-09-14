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
//
/// \file medical/GammaTherapy/src/TargetSD.cc
/// \brief Implementation of the TargetSD class
//
// -------------------------------------------------------------
//
//
//      ---------- TargetSD -------------
//              
//  Modified:
//
// -------------------------------------------------------------

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
/// \file medical/GammaTherapy/include/DetectorSD.hh
/// \brief Definition of the DetectorSD class
//
#ifndef DetectorSD_h
#define DetectorSD_h 1

// -------------------------------------------------------------
//
//  Modified:
//
// -------------------------------------------------------------
#include "G4VSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Step.hh"
#include "globals.hh"
#include "G4THitsCollection.hh"
#include "DetectorHit.hh"            // 包含 Hit
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class DetectorSD : public G4VSensitiveDetector
{
  public:
  static G4ThreadLocal DetectorSD* instance;
  DetectorSD(const G4String&);
  virtual ~DetectorSD();

  // void Initialize(G4HCofThisEvent*);
  // G4bool ProcessHits(G4Step*, G4TouchableHistory*);

  //void Initialize(G4HCofThisEvent* hitCollection) override;
  void Initialize(G4HCofThisEvent* hce);
  G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*) override;
  // G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*){
  //     DetectorHit* hit = new DetectorHit();
  //     fHitsCollection->insert(hit);
  // }


  void EndOfEvent(G4HCofThisEvent*) override;
  void clear();
  void PrintAll();
  // G4VHitsCollection* fHitsCollection;
  // G4int hcID;
  private:
  G4bool PixelWithHoles;
  
    G4THitsCollection<DetectorHit>* fHitsCollection;
    G4int hcID;
    DetectorSD & operator=(const DetectorSD &right) = delete;
    DetectorSD(const DetectorSD&) = delete;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif