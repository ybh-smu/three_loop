//#include "G4VHitsCollection.hh"
#include "G4VHit.hh"
class TrackerHit : public G4VHit {
public:
    TrackerHit();
    TrackerHit(const TrackerHit& right);
    virtual ~TrackerHit();

    const TrackerHit& operator=(const TrackerHit& right);
    G4int operator==(const TrackerHit& right) const;

    G4Step* GetStep() const { return fStep; }

    inline void* operator new(size_t);
    inline void  operator delete(void* aTrackerHit);

    void Draw();
    void Print();

    // 其他成员函数和变量
private:
    G4Step* fStep;
    G4int fTrackID;
    G4double fEdep;
    G4ThreeVector fPos;
};