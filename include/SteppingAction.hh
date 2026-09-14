#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "globals.hh"
#include "G4UserSteppingAction.hh"

class SteppingAction : public G4UserSteppingAction
{
 public:
  SteppingAction();
  virtual ~SteppingAction();

  // method from the base class
  virtual void UserSteppingAction(const G4Step*);

 private:
  G4int fVerbose = 0;
  size_t fIdxVelocity = 0;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif