#ifndef DetectorConstructionMessenger_h
#define DetectorConstructionMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIcmdWithABool;
class G4UIcmdWithADouble;
class G4UIcmdWithAString;

class DetectorConstructionMessenger : public G4UImessenger {
public:
    explicit DetectorConstructionMessenger(DetectorConstruction* detector);
    ~DetectorConstructionMessenger() override;
    void SetNewValue(G4UIcommand* cmd, G4String newValue) override;

private:
    DetectorConstruction* fDetector;
    G4UIcmdWithADouble* fAngleCmd;
    G4UIcmdWithABool* fUseCtCmd;
    G4UIcmdWithAString* fCtMetaFileCmd;
    G4UIcmdWithABool* fUseBrainCmd;
    G4UIcmdWithAString* fBrainFileCmd;
    G4UIcmdWithAString* fBrainDimsCmd;
    G4UIcmdWithAString* fBrainVoxelSizeCmd;
};

#endif
