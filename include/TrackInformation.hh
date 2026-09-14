#ifndef TrackInformation_h
#define TrackInformation_h 1

#include "G4VUserTrackInformation.hh"

class TrackInformation : public G4VUserTrackInformation
{
  public:
    TrackInformation() = default;
    explicit TrackInformation(G4bool hasScattered)
      : fHasScattered(hasScattered)
    {}
    TrackInformation(const TrackInformation&) = default;
    ~TrackInformation() override = default;

    void MarkScattered() { fHasScattered = true; }
    G4bool HasScattered() const { return fHasScattered; }

    void Print() const override {}

  private:
    G4bool fHasScattered = false;
};

#endif
