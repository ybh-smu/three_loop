#ifndef ActionInitializationMessenger_h
#define ActionInitializationMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithAString;
class G4UIdirectory;

class ActionInitializationMessenger : public G4UImessenger
{
public:
  ActionInitializationMessenger();
  ~ActionInitializationMessenger() override;

  void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
  G4UIdirectory* fSourceDir;
  G4UIdirectory* fRunActionDir;
  G4UIcmdWith3VectorAndUnit* fSourcePositionCmd;
  G4UIcmdWithAString* fSourceViewCmd;
  G4UIcmdWithAString* fOutputDirectoryCmd;
  G4UIcmdWithAString* fOutputPrefixCmd;
};

#endif
