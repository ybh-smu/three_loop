// fanoCavity 00000000000000000

#include "ActionInitialization.hh"

#include "ActionInitializationMessenger.hh"
#include "DetectorConstruction.hh"
#include "G4Gamma.hh"
#include "G4LogicalVolume.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4TrackVector.hh"
#include "G4TrackingManager.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackInformation.hh"
#include "TrackingAction.hh"

namespace {
bool IsPhantomOrObjectInteractionVolume(const G4String& logicalName,
                                        const G4String& physicalName)
{
  if (logicalName == "Object" || logicalName == "PhantomBox" ||
      logicalName == "PhantomVoxel" || logicalName == "PhantomContainer" ||
      logicalName == "VoxelLogic" || logicalName == "CorgiDefriseCylinder" ||
      logicalName == "DefriseDisk" || logicalName =="PhantomContainer") {
    return true;
  }

  if (physicalName == "Object" || physicalName == "PhantomObject" ||
      physicalName == "PhantomVoxel" || physicalName == "PhantomContainer" ||
      physicalName == "CorgiDefriseCylinder" || physicalName == "BrainVoxels" || physicalName =="PhantomContainer") {
    return true;
  }

  return physicalName.rfind("CorgiDefriseDisk_", 0) == 0 ||
         physicalName.rfind("BrainVoxels", 0) == 0;
}
}  // namespace

ActionInitializationMessenger::ActionInitializationMessenger()
{
  fSourceDir = new G4UIdirectory("/source/");
  fSourceDir->SetGuidance("Source configuration commands.");

  fRunActionDir = new G4UIdirectory("/runAction/");
  fRunActionDir->SetGuidance("Run output configuration commands.");

  fSourcePositionCmd = new G4UIcmdWith3VectorAndUnit("/source/setPosition", this);
  fSourcePositionCmd->SetGuidance("Set source center position.");
  fSourcePositionCmd->SetParameterName("x", "y", "z", false);
  fSourcePositionCmd->SetDefaultUnit("mm");
  fSourcePositionCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fSourceViewCmd = new G4UIcmdWithAString("/source/setView", this);
  fSourceViewCmd->SetGuidance("Set source view: Front or Back.");
  fSourceViewCmd->SetParameterName("view", false);
  fSourceViewCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fOutputDirectoryCmd = new G4UIcmdWithAString("/runAction/setOutputDirectory", this);
  fOutputDirectoryCmd->SetGuidance("Set ROOT output directory.");
  fOutputDirectoryCmd->SetParameterName("directory", false);
  fOutputDirectoryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fOutputPrefixCmd = new G4UIcmdWithAString("/runAction/setOutputPrefix", this);
  fOutputPrefixCmd->SetGuidance("Set output filename prefix: Front or Back.");
  fOutputPrefixCmd->SetParameterName("prefix", false);
  fOutputPrefixCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

ActionInitializationMessenger::~ActionInitializationMessenger()
{
  delete fOutputPrefixCmd;
  delete fOutputDirectoryCmd;
  delete fSourceViewCmd;
  delete fSourcePositionCmd;
  delete fRunActionDir;
  delete fSourceDir;
}

void ActionInitializationMessenger::SetNewValue(G4UIcommand* command,
                                                G4String newValue)
{
  if (command == fSourcePositionCmd) {
    PrimaryGeneratorAction::SetDefaultSourcePosition(
      fSourcePositionCmd->GetNew3VectorValue(newValue));
    return;
  }

  if (command == fSourceViewCmd) {
    PrimaryGeneratorAction::SetDefaultSourceView(newValue);
    return;
  }

  if (command == fOutputDirectoryCmd) {
    RunAction::SetDefaultOutputDirectory(newValue);
    return;
  }

  if (command == fOutputPrefixCmd) {
    RunAction::SetDefaultOutputPrefix(newValue);
  }
}

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
  : G4VUserActionInitialization(),
    fDetector(detector),
    fMessenger(new ActionInitializationMessenger())
{}

ActionInitialization::~ActionInitialization()
{
  delete fMessenger;
}

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction(fDetector));
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction(fDetector));
  SetUserAction(new EventAction(fDetector));
  SetUserAction(new RunAction(fDetector));
  SetUserAction(new TrackingAction());
  SetUserAction(new SteppingAction());
}

TrackingAction::TrackingAction()
  : G4UserTrackingAction()
{}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  if (track->GetUserInformation() == nullptr) {
    auto* mutableTrack = const_cast<G4Track*>(track);
    mutableTrack->SetUserInformation(new TrackInformation(false));
  }
}

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  const auto* info = dynamic_cast<const TrackInformation*>(track->GetUserInformation());
  const G4bool hasScattered = (info != nullptr) ? info->HasScattered() : false;

  G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
  if (secondaries == nullptr) {
    return;
  }

  for (auto* secondary : *secondaries) {
    if (secondary->GetUserInformation() == nullptr) {
      secondary->SetUserInformation(new TrackInformation(hasScattered));
    } else {
      auto* secondaryInfo =
          dynamic_cast<TrackInformation*>(secondary->GetUserInformation());
      if (secondaryInfo != nullptr && hasScattered) {
        secondaryInfo->MarkScattered();
      }
    }
  }
}

SteppingAction::SteppingAction() = default;

SteppingAction::~SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4Track* track = step->GetTrack();
  if (track->GetDefinition() != G4Gamma::Gamma()) {
    return;
  }

  const auto* preVolume = step->GetPreStepPoint()->GetPhysicalVolume();
  if (preVolume == nullptr) {
    return;
  }

  const auto* process = step->GetPostStepPoint()->GetProcessDefinedStep();
  if (process == nullptr || process->GetProcessName() == "Transportation") {
    return;
  }

  const G4String logicalName = preVolume->GetLogicalVolume()->GetName();
  const G4String physicalName = preVolume->GetName();
  if (logicalName == "PixelNormal" || logicalName == "PixelWithHoles") {
    return;
  }
  if (!IsPhantomOrObjectInteractionVolume(logicalName, physicalName)) {
    return;
  }

  auto* info = dynamic_cast<TrackInformation*>(track->GetUserInformation());
  if (info == nullptr) {
    info = new TrackInformation(false);
    track->SetUserInformation(info);
  }
  info->MarkScattered();
}
