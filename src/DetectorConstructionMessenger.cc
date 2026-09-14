#include "DetectorConstructionMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAString.hh"

#include <sstream>

DetectorConstructionMessenger::DetectorConstructionMessenger(DetectorConstruction* detector)
    : fDetector(detector) {
    fAngleCmd = new G4UIcmdWithADouble("/detector/setObjectAngle", this);
    fAngleCmd->SetGuidance("Set object rotation angle in degrees.");
    fAngleCmd->SetParameterName("angle", false);

    fUseCtCmd = new G4UIcmdWithABool("/detector/useCt", this);
    fUseCtCmd->SetGuidance("Enable or disable the CT voxel phantom.");

    fCtMetaFileCmd = new G4UIcmdWithAString("/detector/setCtMetaFile", this);
    fCtMetaFileCmd->SetGuidance("Set the CT phantom meta file path.");

    fUseBrainCmd = new G4UIcmdWithABool("/detector/useBrain", this);
    fUseBrainCmd->SetGuidance("Enable or disable the voxel brain phantom.");

    fBrainFileCmd = new G4UIcmdWithAString("/detector/setBrainFile", this);
    fBrainFileCmd->SetGuidance("Set the brain phantom raw file path.");

    fBrainDimsCmd = new G4UIcmdWithAString("/detector/setBrainDimensions", this);
    fBrainDimsCmd->SetGuidance("Set brain phantom dimensions: width height depth.");

    fBrainVoxelSizeCmd = new G4UIcmdWithAString("/detector/setBrainVoxelSize", this);
    fBrainVoxelSizeCmd->SetGuidance("Set brain voxel size in mm: sizeX sizeY sizeZ.");
}

DetectorConstructionMessenger::~DetectorConstructionMessenger() {
    delete fAngleCmd;
    delete fUseCtCmd;
    delete fCtMetaFileCmd;
    delete fUseBrainCmd;
    delete fBrainFileCmd;
    delete fBrainDimsCmd;
    delete fBrainVoxelSizeCmd;
}

void DetectorConstructionMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    if (cmd == fAngleCmd) {
        const G4double angleDeg = G4UIcmdWithADouble::GetNewDoubleValue(newValue);
        fDetector->SetObjectRotationAngle(angleDeg * CLHEP::deg);
        return;
    }

    if (cmd == fUseCtCmd) {
        fDetector->SetUseCt(fUseCtCmd->GetNewBoolValue(newValue));
        return;
    }

    if (cmd == fCtMetaFileCmd) {
        fDetector->SetCtPhantomMetaFile(newValue);
        return;
    }

    if (cmd == fUseBrainCmd) {
        fDetector->SetUseBrain(fUseBrainCmd->GetNewBoolValue(newValue));
        return;
    }

    if (cmd == fBrainFileCmd) {
        fDetector->SetBrainPhantomFile(newValue);
        return;
    }

    if (cmd == fBrainDimsCmd) {
        std::istringstream iss(newValue);
        G4int width = 0;
        G4int height = 0;
        G4int depth = 0;
        if (iss >> width >> height >> depth) {
            fDetector->SetBrainPhantomDimensions(width, height, depth);
        }
        return;
    }

    if (cmd == fBrainVoxelSizeCmd) {
        std::istringstream iss(newValue);
        G4double sizeX = 0.0;
        G4double sizeY = 0.0;
        G4double sizeZ = 0.0;
        if (iss >> sizeX >> sizeY >> sizeZ) {
            fDetector->SetBrainVoxelSize(sizeX * CLHEP::mm,
                                         sizeY * CLHEP::mm,
                                         sizeZ * CLHEP::mm);
        }
    }
}
