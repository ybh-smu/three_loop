#include "BrainPhantomData.hh"

#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <sstream>

BrainPhantomData::BrainPhantomData(const G4String& metaFile)
    : fMetaFile(metaFile) {}

BrainPhantomData::~BrainPhantomData() = default;

size_t BrainPhantomData::Index(G4int x, G4int y, G4int z) const {
    return static_cast<size_t>(x)
         + static_cast<size_t>(y) * static_cast<size_t>(fWidth)
         + static_cast<size_t>(z) * static_cast<size_t>(fWidth) * static_cast<size_t>(fHeight);
}

G4bool BrainPhantomData::ReadData() {
    namespace fs = std::filesystem;

    std::ifstream meta(fMetaFile);
    if (!meta.is_open()) {
        G4cerr << "Error: Could not open brain phantom meta file: " << fMetaFile << G4endl;
        return false;
    }

    fs::path metaPath{std::string(fMetaFile)};
    fs::path baseDir = metaPath.parent_path();

    G4String key;
    while (meta >> key) {
        if (key == "NX") meta >> fWidth;
        else if (key == "NY") meta >> fHeight;
        else if (key == "NZ") meta >> fDepth;
        else if (key == "DX_MM") { G4double v; meta >> v; fVoxelSizeX = v * mm; }
        else if (key == "DY_MM") { G4double v; meta >> v; fVoxelSizeY = v * mm; }
        else if (key == "DZ_MM") { G4double v; meta >> v; fVoxelSizeZ = v * mm; }
        else if (key == "ORIGIN_X_MM") { G4double v; meta >> v; fOriginX = v * mm; }
        else if (key == "ORIGIN_Y_MM") { G4double v; meta >> v; fOriginY = v * mm; }
        else if (key == "ORIGIN_Z_MM") { G4double v; meta >> v; fOriginZ = v * mm; }
        else if (key == "MATERIAL_FILE") meta >> fMaterialFile;
        else if (key == "DENSITY_FILE") meta >> fDensityFile;
        else {
            std::string dummy;
            std::getline(meta, dummy);
        }
    }

    if (fWidth <= 0 || fHeight <= 0 || fDepth <= 0) {
        G4cerr << "Error: Invalid phantom dimensions in meta file." << G4endl;
        return false;
    }

    if (fMaterialFile.empty()) fMaterialFile = "brain_material.raw";
    if (fDensityFile.empty())  fDensityFile  = "brain_density.raw";

    fs::path matPath = baseDir / fs::path{std::string(fMaterialFile)};
    fs::path denPath = baseDir / fs::path{std::string(fDensityFile)};   

    const size_t nvox = static_cast<size_t>(fWidth) * static_cast<size_t>(fHeight) * static_cast<size_t>(fDepth);
    fMaterialIds.resize(nvox, 0);
    fDensities.resize(nvox, 0.0012f);

    {
        std::ifstream fin(matPath, std::ios::binary);
        if (!fin.is_open()) {
            G4cerr << "Error: Could not open material raw file: " << matPath.string() << G4endl;
            return false;
        }
        fin.read(reinterpret_cast<char*>(fMaterialIds.data()), static_cast<std::streamsize>(nvox));
        if (fin.gcount() != static_cast<std::streamsize>(nvox)) {
            G4cerr << "Error: Material raw size mismatch. Expected " << nvox << " bytes." << G4endl;
            return false;
        }
    }

    {
        std::ifstream fin(denPath, std::ios::binary);
        if (!fin.is_open()) {
            G4cerr << "Error: Could not open density raw file: " << denPath.string() << G4endl;
            return false;
        }
        fin.read(reinterpret_cast<char*>(fDensities.data()), static_cast<std::streamsize>(nvox * sizeof(float)));
        if (fin.gcount() != static_cast<std::streamsize>(nvox * sizeof(float))) {
            G4cerr << "Error: Density raw size mismatch. Expected " << nvox * sizeof(float) << " bytes." << G4endl;
            return false;
        }
    }

    G4cout << "Brain phantom loaded from meta: " << fMetaFile << G4endl
           << "  size   = " << fWidth << " x " << fHeight << " x " << fDepth << G4endl
           << "  voxel  = " << fVoxelSizeX / mm << " x " << fVoxelSizeY / mm << " x " << fVoxelSizeZ / mm << " mm^3" << G4endl
           << "  origin = (" << fOriginX / mm << ", " << fOriginY / mm << ", " << fOriginZ / mm << ") mm" << G4endl
           << "  material raw = " << matPath.string() << G4endl
           << "  density raw  = " << denPath.string() << G4endl;
    
    size_t nAir = 0, nSoft = 0, nBone = 0, nDenseBone = 0;

    for (size_t i = 0; i < fMaterialIds.size(); ++i) {
        switch (fMaterialIds[i]) {
            case 0: ++nAir; break;
            case 1: ++nSoft; break;
            case 2: ++nBone; break;
            case 3: ++nDenseBone; break;
            default: break;
        }
    }

    G4cout << "Brain phantom voxel statistics:" << G4endl;
    G4cout << "  Air        = " << nAir << G4endl;
    G4cout << "  SoftTissue = " << nSoft << G4endl;
    G4cout << "  Bone       = " << nBone << G4endl;
    G4cout << "  DenseBone  = " << nDenseBone << G4endl;

    return true;
}

G4int BrainPhantomData::GetMaterialId(G4int x, G4int y, G4int z) const {
    if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || z < 0 || z >= fDepth) return 0;
    return static_cast<G4int>(fMaterialIds[Index(x, y, z)]);
}

G4double BrainPhantomData::GetDensity(G4int x, G4int y, G4int z) const {
    if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || z < 0 || z >= fDepth) return 0.0012 * g / cm3;
    return static_cast<G4double>(fDensities[Index(x, y, z)]) * g / cm3;
}
void BrainPhantomData::SavePreviewPGM(const G4String& outFile) const
{
    if (fWidth <= 0 || fHeight <= 0 || fDepth <= 0 || fDensities.empty()) {
        G4cerr << "BrainPhantomData::SavePreviewPGM(): phantom data is empty." << G4endl;
        return;
    }

    const G4int xc = fWidth / 2;
    const G4int yc = fHeight / 2;
    const G4int zc = fDepth / 2;

    const G4int gap = 8;

    // 三个视图尺寸：
    // axial    : [y, x] -> height = fHeight, width = fWidth
    // coronal  : [z, x] -> height = fDepth,  width = fWidth
    // sagittal : [z, y] -> height = fDepth,  width = fHeight
    const G4int axialW = fWidth;
    const G4int axialH = fHeight;
    const G4int corW   = fWidth;
    const G4int corH   = fDepth;
    const G4int sagW   = fHeight;
    const G4int sagH   = fDepth;

    const G4int canvasW = axialW + gap + corW + gap + sagW;
    const G4int canvasH = std::max(axialH, std::max(corH, sagH));

    std::vector<unsigned char> img(static_cast<size_t>(canvasW) * static_cast<size_t>(canvasH), 0);

    auto setPixel = [&](G4int x, G4int y, unsigned char v) {
        if (x < 0 || x >= canvasW || y < 0 || y >= canvasH) return;
        img[static_cast<size_t>(y) * static_cast<size_t>(canvasW) + static_cast<size_t>(x)] = v;
    };

    // 这里用 density 映射成灰度，更容易看脑结构
    auto densityToGray = [](G4double rho_g_cm3) -> unsigned char {
        // 第一版脑部 phantom 的密度大致落在 [0, 2.2] g/cm3
        G4double v = rho_g_cm3 / (2.2 * g / cm3);
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        return static_cast<unsigned char>(std::round(v * 255.0));
    };

    // 居中摆放
    const G4int axialX0 = 0;
    const G4int axialY0 = (canvasH - axialH) / 2;

    const G4int corX0   = axialX0 + axialW + gap;
    const G4int corY0   = (canvasH - corH) / 2;

    const G4int sagX0   = corX0 + corW + gap;
    const G4int sagY0   = (canvasH - sagH) / 2;

    // -------- axial: z = zc, image[y, x] --------
    for (G4int y = 0; y < fHeight; ++y) {
        for (G4int x = 0; x < fWidth; ++x) {
            const G4double rho = GetDensity(x, y, zc);
            const unsigned char gray = densityToGray(rho);
            // 为了和常见图像方向一致，这里把 y 翻转一下
            setPixel(axialX0 + x, axialY0 + (fHeight - 1 - y), gray);
        }
    }

    // -------- coronal: y = yc, image[z, x] --------
    for (G4int z = 0; z < fDepth; ++z) {
        for (G4int x = 0; x < fWidth; ++x) {
            const G4double rho = GetDensity(x, yc, z);
            const unsigned char gray = densityToGray(rho);
            setPixel(corX0 + x, corY0 + (fDepth - 1 - z), gray);
        }
    }

    // -------- sagittal: x = xc, image[z, y] --------
    for (G4int z = 0; z < fDepth; ++z) {
        for (G4int y = 0; y < fHeight; ++y) {
            const G4double rho = GetDensity(xc, y, z);
            const unsigned char gray = densityToGray(rho);
            setPixel(sagX0 + y, sagY0 + (fDepth - 1 - z), gray);
        }
    }

    // 写 PGM（二进制 P5）
    std::ofstream fout(outFile, std::ios::binary);
    if (!fout.is_open()) {
        G4cerr << "BrainPhantomData::SavePreviewPGM(): failed to open " << outFile << G4endl;
        return;
    }

    fout << "P5\n" << canvasW << " " << canvasH << "\n255\n";
    fout.write(reinterpret_cast<const char*>(img.data()),
               static_cast<std::streamsize>(img.size()));
    fout.close();

    G4cout << "Brain phantom preview saved to: " << outFile << G4endl;
}