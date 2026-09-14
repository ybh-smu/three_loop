#include "CTPhantomData.hh"

#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
G4String ResolveSiblingPath(const G4String& baseFile, const G4String& sibling)
{
    const std::string base = baseFile;
    const std::string leaf = sibling;
    if (leaf.find(':') != std::string::npos || (!leaf.empty() && (leaf[0] == '/' || leaf[0] == '\\'))) {
        return sibling;
    }

    const std::string::size_type pos = base.find_last_of("/\\");
    if (pos == std::string::npos) {
        return sibling;
    }
    return base.substr(0, pos + 1) + leaf;
}
}

CTPhantomData::CTPhantomData(const G4String& metaFilename)
    : fMetaFilename(metaFilename),
      fWidth(0),
      fHeight(0),
      fDepth(0),
      fVoxelSizeX(1.0 * mm),
      fVoxelSizeY(1.0 * mm),
      fVoxelSizeZ(1.0 * mm) {}

CTPhantomData::~CTPhantomData() = default;

G4bool CTPhantomData::ReadMetaFile()
{
    std::ifstream metaFile(fMetaFilename);
    if (!metaFile.is_open()) {
        G4cerr << "Error: Could not open CT phantom meta file: " << fMetaFilename << G4endl;
        return false;
    }

    std::string line;
    while (std::getline(metaFile, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "dim") {
            iss >> fWidth >> fHeight >> fDepth;
        } else if (key == "spacing_mm") {
            G4double sx = 0.0;
            G4double sy = 0.0;
            G4double sz = 0.0;
            iss >> sx >> sy >> sz;
            fVoxelSizeX = sx * mm;
            fVoxelSizeY = sy * mm;
            fVoxelSizeZ = sz * mm;
        } else if (key == "raw_file") {
            std::string rawFile;
            iss >> rawFile;
            fRawFilename = ResolveSiblingPath(fMetaFilename, rawFile);
        }
    }

    const G4bool valid = (fWidth > 0 && fHeight > 0 && fDepth > 0 && !fRawFilename.empty());
    if (!valid) {
        G4cerr << "Error: CT phantom meta file is incomplete: " << fMetaFilename << G4endl;
    }
    return valid;
}

G4bool CTPhantomData::ReadData()
{
    if (!ReadMetaFile()) {
        return false;
    }

    std::ifstream rawFile(fRawFilename, std::ios::binary);
    if (!rawFile.is_open()) {
        G4cerr << "Error: Could not open CT phantom raw file: " << fRawFilename << G4endl;
        return false;
    }

    const size_t voxelCount = static_cast<size_t>(fWidth) * fHeight * fDepth;
    std::vector<short> huRaw(voxelCount);
    rawFile.read(reinterpret_cast<char*>(huRaw.data()), static_cast<std::streamsize>(voxelCount * sizeof(short)));
    if (rawFile.gcount() != static_cast<std::streamsize>(voxelCount * sizeof(short))) {
        G4cerr << "Error: CT phantom raw file size mismatch for " << fRawFilename << G4endl;
        return false;
    }

    fData.resize(voxelCount, 0);
    for (size_t i = 0; i < voxelCount; ++i) {
        fData[i] = static_cast<G4int>(huRaw[i]);
    }

    G4cout << "CT phantom loaded from " << fRawFilename << G4endl;
    G4cout << "Dimensions: " << fWidth << " x " << fHeight << " x " << fDepth << G4endl;
    G4cout << "Voxel size: "
           << fVoxelSizeX / mm << " x "
           << fVoxelSizeY / mm << " x "
           << fVoxelSizeZ / mm << " mm" << G4endl;
    return true;
}

G4int CTPhantomData::GetVoxelHU(G4int x, G4int y, G4int z) const
{
    if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || z < 0 || z >= fDepth) {
        return -1024;
    }
    return fData[x + y * fWidth + z * fWidth * fHeight];
}
