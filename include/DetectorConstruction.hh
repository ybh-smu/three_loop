//
// ********************************************************************
// * License and Disclaimer                                           *
// ********************************************************************
//

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4Material.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

class BrainPhantomData;
class CTPhantomData;
class DetectorConstructionMessenger;
class DetectorMessenger;
class G4GlobalMagFieldMessenger;
class G4LogicalVolume;
class G4SensitiveDetector;
class G4UImessenger;
class G4VPhysicalVolume;
class PhantomData;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;
  void SetObjectRotationAngle(G4double angle);
  G4double GetCurrentAngle() const;

  void SetUsePhantom(G4bool use) { fUsePhantom = use; }
  G4bool GetUsePhantom() const { return fUsePhantom; }
  void SetUseCt(G4bool use) { fUseCt = use; if (use) fUseBrain = false; }
  void SetCtPhantomMetaFile(const G4String& path);
  void SetUseBrain(G4bool use) { fUseBrain = use; if (use) fUseCt = false; }
  void SetBrainPhantomFile(const G4String& path);
  void SetBrainPhantomDimensions(G4int width, G4int height, G4int depth);
  void SetBrainVoxelSize(G4double sizeX, G4double sizeY, G4double sizeZ);
  G4double GetCurrentObjectSizeX() const { return fCurrentObjectSizeX; }
  G4double GetCurrentObjectSizeY() const { return fCurrentObjectSizeY; }
  G4double GetCurrentObjectSizeZ() const { return fCurrentObjectSizeZ; }

  inline void SetNumberDivZ(G4int val) { fNumZ = val; };
  inline void SetNumberDivR(G4int val) { fNumR = val; };
  G4int GetNumberDivZ() const { return fNumZ; }
  G4int GetNumberDivR() const { return fNumR; }

  void SetMaxEnergy(G4double e) { fMaxEnergy = e; }
  inline G4double GetMaxEnergy() const { return fMaxEnergy; }
  G4int GetNumberDivE() const { return fNumE; }
  inline void SetNumberDivE(G4int val) { fNumE = val; };

  void SetVerbose(G4bool v) { fVerbose = v; }
  inline G4bool GetVerbose() const { return fVerbose; }

private:
  void InitialiseGeometryParameters();

  DetectorConstruction& operator=(const DetectorConstruction& right);
  DetectorConstruction(const DetectorConstruction&);
  void ConstructSDandField();

  G4bool fVerbose;

  G4int fNumZ;
  G4int fNumR;
  G4int worldSize;
  G4int fNumE;
  G4double fMaxEnergy;

  G4LogicalVolume* cellLogical;
  G4LogicalVolume* currentLogic;
  G4LogicalVolume* cellLogicalWithHoles;
  G4LogicalVolume* cellLogicalNormal;
  G4LogicalVolume* fLogicDetector;
  G4LogicalVolume* logicObject;
  G4LogicalVolume* logicWorld;
  G4VSensitiveDetector* sensitiveDetector;

  G4double fWorldX, fWorldY, fWorldZ;
  G4double fObjectRmin, fObjectRmax, fObjectDz;
  G4double fDetectorX, fDetectorY, fDetectorZ;

  G4double fObjectPosz, fDetectorPosz;

  G4Material* fWorldMaterial;
  G4Material* fObjectMaterial;
  G4Material* fDetectorMaterial;
  G4Material* mat_Glass;
  G4Material* mat_Al;
  G4Material* mat_Mo;
  G4Material* vacuum;
  G4double fDetectorRmin;
  G4VPhysicalVolume* fPhysWorld;

  G4double fCurrentAngle = 0.0;
  G4VPhysicalVolume* fPhysObject;
  G4RotationMatrix* fRotationMatrix;
  DetectorConstructionMessenger* fMessenger;
  G4bool fUsePhantom;
  G4bool fUseCt;
  G4bool fUseBrain;
  PhantomData* fPhantomData;
  CTPhantomData* fCtPhantomData;
  G4bool fUseCorgiDefrise;
  BrainPhantomData* fBrainData;
  G4String fCtPhantomMetaFile;
  G4String fBrainPhantomFile;
  G4int fBrainWidth;
  G4int fBrainHeight;
  G4int fBrainDepth;
  G4double fBrainVoxelSizeX;
  G4double fBrainVoxelSizeY;
  G4double fBrainVoxelSizeZ;
  G4double fCurrentObjectSizeX = 0.0;
  G4double fCurrentObjectSizeY = 0.0;
  G4double fCurrentObjectSizeZ = 0.0;
};

#endif
