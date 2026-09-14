/// PhantomData.cc
#include "PhantomData.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

PhantomData::PhantomData(const G4String& filename)
    : fFilename(filename), fWidth(282), fHeight(141), fDepth(468),
      fMinValue(0), fMaxValue(85)
{
    // Pre-allocate memory for the data
    fData.resize(fWidth * fHeight * fDepth, 0);
}

PhantomData::~PhantomData()
{
    // Empty destructor
}

G4bool PhantomData::ReadData()
{
    std::ifstream file(fFilename, std::ios::binary);
    if (!file.is_open()) {
        G4cerr << "ERROR: Could not open phantom data file: " << fFilename << G4endl;
        return false;
    }
    
    // Read the binary data as uint8
    std::vector<unsigned char> rawData(fWidth * fHeight * fDepth);
    file.read(reinterpret_cast<char*>(rawData.data()), rawData.size());
    
    if (file.gcount() != static_cast<std::streamsize>(rawData.size())) {
        G4cerr << "ERROR: Read only " << file.gcount() << " bytes, expected " 
               << rawData.size() << G4endl;
        file.close();
        return false;
    }
    file.close();
    
    // Convert uint8 to G4int and find min/max values
    fMinValue = 255;
    fMaxValue = 0;
    
    for (size_t i = 0; i < rawData.size(); ++i) {
        G4int value = static_cast<G4int>(rawData[i]);
        fData[i] = value;
        
        if (value < fMinValue) fMinValue = value;
        if (value > fMaxValue) fMaxValue = value;
    }
    
    G4cout << "Phantom data loaded: " << fWidth << " x " << fHeight << " x " << fDepth 
           << " voxels" << G4endl;
    G4cout << "Value range: " << fMinValue << " to " << fMaxValue << G4endl;
    
    return true;
}

G4int PhantomData::GetVoxelValue(G4int x, G4int y, G4int z) const
{
    if (x < 0 || x >= fWidth || y < 0 || y >= fHeight || z < 0 || z >= fDepth) {
        return 0; // Return 0 for out-of-bounds indices
    }
    
    // Index calculation needs to match the data layout in the file
    // Based on MATLAB code, the storage is [width, height, depth]
    return fData[x + y*fWidth + z*fWidth*fHeight];
}

G4bool PhantomData::SaveOrthogonalSlicesMontageAsPGM(const G4String& outputFilename,
                                                     G4double localX,
                                                     G4double localY,
                                                     G4double localZ,
                                                     G4double voxelSizeX,
                                                     G4double voxelSizeY,
                                                     G4double voxelSizeZ) const
{
    if (fData.empty()) {
        G4cerr << "ERROR: Phantom data is empty, cannot export slice montage." << G4endl;
        return false;
    }

    const G4double sliceX = localX / voxelSizeX + fWidth / 2.0 - 0.5;
    const G4double sliceY = localY / voxelSizeY + fHeight / 2.0 - 0.5;
    const G4double sliceZ = localZ / voxelSizeZ + fDepth / 2.0 - 0.5;
    const G4int gap = 10;
    const G4int montageWidth = fWidth + gap + fWidth + gap + fHeight;
    const G4int montageHeight = std::max(fHeight, fDepth);
    std::ofstream out(outputFilename, std::ios::binary);
    if (!out.is_open()) {
        G4cerr << "ERROR: Could not open output image file: " << outputFilename << G4endl;
        return false;
    }

    const G4int valueRange = std::max(1, fMaxValue - fMinValue);
    std::vector<unsigned char> pixels(montageWidth * montageHeight, 0);

    const auto toGray = [&](G4int value) -> unsigned char {
        const G4int gray = (255 * (value - fMinValue)) / valueRange;
        return static_cast<unsigned char>(std::clamp(gray, 0, 255));
    };

    const auto setPixel = [&](G4int x, G4int y, unsigned char gray) {
        if (x >= 0 && x < montageWidth && y >= 0 && y < montageHeight) {
            pixels[y * montageWidth + x] = gray;
        }
    };

    const auto sampleAlongX = [&](G4int y, G4int z, G4double xIndex) {
        const G4double clamped = std::clamp(xIndex, 0.0, static_cast<G4double>(fWidth - 1));
        const G4int x0 = static_cast<G4int>(std::floor(clamped));
        const G4int x1 = std::min(x0 + 1, fWidth - 1);
        const G4double alpha = clamped - x0;
        return static_cast<G4int>(std::lround((1.0 - alpha) * GetVoxelValue(x0, y, z) +
                                              alpha * GetVoxelValue(x1, y, z)));
    };

    const auto sampleAlongY = [&](G4int x, G4int z, G4double yIndex) {
        const G4double clamped = std::clamp(yIndex, 0.0, static_cast<G4double>(fHeight - 1));
        const G4int y0 = static_cast<G4int>(std::floor(clamped));
        const G4int y1 = std::min(y0 + 1, fHeight - 1);
        const G4double alpha = clamped - y0;
        return static_cast<G4int>(std::lround((1.0 - alpha) * GetVoxelValue(x, y0, z) +
                                              alpha * GetVoxelValue(x, y1, z)));
    };

    const auto sampleAlongZ = [&](G4int x, G4int y, G4double zIndex) {
        const G4double clamped = std::clamp(zIndex, 0.0, static_cast<G4double>(fDepth - 1));
        const G4int z0 = static_cast<G4int>(std::floor(clamped));
        const G4int z1 = std::min(z0 + 1, fDepth - 1);
        const G4double alpha = clamped - z0;
        return static_cast<G4int>(std::lround((1.0 - alpha) * GetVoxelValue(x, y, z0) +
                                              alpha * GetVoxelValue(x, y, z1)));
    };

    const G4int axialOffsetX = 0;
    const G4int coronalOffsetX = fWidth + gap;
    const G4int sagittalOffsetX = fWidth + gap + fWidth + gap;
    const G4int axialOffsetY = (montageHeight - fHeight) / 2;
    const G4int coronalOffsetY = 0;
    const G4int sagittalOffsetY = 0;

    // Axial: z = sliceZ, image[y, x]
    for (G4int y = 0; y < fHeight; ++y) {
        for (G4int x = 0; x < fWidth; ++x) {
            setPixel(axialOffsetX + x,
                     axialOffsetY + (fHeight - 1 - y),
                     toGray(sampleAlongZ(x, y, sliceZ)));
        }
    }

    // Coronal: y = sliceY, image[z, x]
    for (G4int z = 0; z < fDepth; ++z) {
        for (G4int x = 0; x < fWidth; ++x) {
            setPixel(coronalOffsetX + x,
                     coronalOffsetY + (fDepth - 1 - z),
                     toGray(sampleAlongY(x, z, sliceY)));
        }
    }

    // Sagittal: x = sliceX, image[z, y]
    for (G4int z = 0; z < fDepth; ++z) {
        for (G4int y = 0; y < fHeight; ++y) {
            setPixel(sagittalOffsetX + y,
                     sagittalOffsetY + (fDepth - 1 - z),
                     toGray(sampleAlongX(y, z, sliceX)));
        }
    }

    out << "P5\n";
    out << "# local(mm): x=" << localX / mm
        << ", y=" << localY / mm
        << ", z=" << localZ / mm
        << " | index: x=" << sliceX
        << ", y=" << sliceY
        << ", z=" << sliceZ << "\n";
    out << montageWidth << " " << montageHeight << "\n";
    out << "255\n";
    out.write(reinterpret_cast<const char*>(pixels.data()),
              static_cast<std::streamsize>(pixels.size()));

    G4cout << "Orthogonal slice montage exported to " << outputFilename
           << " at local position ("
           << localX / mm << ", "
           << localY / mm << ", "
           << localZ / mm << ") mm" << G4endl;
    return true;
}
