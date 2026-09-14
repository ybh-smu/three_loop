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
/// \file medical/GammaTherapy/GammaTherapy.cc
/// \brief Main program of the medical/GammaTherapy example
//
//
// -------------------------------------------------------------
//      GEANT4 ibrem
//
//      History: based on object model of
//      2nd December 1995, G.Cosmo
//      ---------- ibrem ----------------------------
//
//  Modified: 05.04.01 Vladimir Ivanchenko new design of ibrem
//
// -------------------------------------------------------------

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "G4Types.hh"
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

int main(int argc,char** argv) {
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  G4cout << "=== Program Start ===" << G4endl;
  G4UIExecutive* ui = nullptr;
  if (argc == 1) ui = new G4UIExecutive(argc,argv);

   auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT);
   G4int nThreads = 50;
   runManager->SetNumberOfThreads(nThreads);

  DetectorConstruction* detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  runManager->SetUserInitialization(new PhysicsList());
  G4cout << "PhysicsList set" << G4endl;

  ActionInitialization* userActions = new ActionInitialization(detector);
  runManager->SetUserInitialization(userActions); 
  G4cout << "ActionInitialization set" << G4endl;

  runManager->Initialize();


  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();


  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  // 添加计时器
  auto startTime = std::chrono::steady_clock::now();  // 记录开始时间
  G4cout << "=== 仿真计时开始 ===" << G4endl;
  if (ui)  {
   UImanager->ApplyCommand("/control/execute vis.mac");
   ui->SessionStart();
   delete ui;
  }
  else  {
  //batch mode
   G4String command = "/control/execute ";
   G4String fileName = argv[1];
   UImanager->ApplyCommand(command+fileName);
   ui = new G4UIExecutive(argc, argv);
   ui->SessionStart();
   delete ui;
  }
    // 记录结束时间并计算耗时
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    G4cout << "=== 仿真计时结束 ===" << G4endl;
    G4cout << "总耗时: " << duration << " 毫秒" << G4endl;
  //job termination
  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
