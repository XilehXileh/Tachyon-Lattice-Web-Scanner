#include <emscripten/bind.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

using namespace emscripten;

const float PHI = 1.61803398875f;

struct Singularity {
    int angle;
    float accuracy;
    long node;
    int drift;
};

class UnifiedLattice {
private:
    std::vector<int> resonance_map;
    float device_pulse = 23.8396f; // Default fallback
    float threshold = 0.999995f;

public:
    UnifiedLattice() {}

    // --- STAGE 1: THE TUNER ---
    // Instead of reading files, we take the pulse from the Browser's JS timer
    void calibrate(float measured_pulse) {
        device_pulse = measured_pulse;
        resonance_map.clear();
        
        // Scan for the 12-face alignment points (-180 to 180)
        for (int d = -180; d < 180; d++) {
            float acc = std::abs(std::sin(PHI + d * 0.0001f));
            if (acc > 0.999f) {
                resonance_map.push_back(d);
            }
        }
    }

    // --- STAGE 2: THE TESTER ---
    // Scans a specific orbit and returns the Lock data if found
    Singularity probe(long orbit) {
        int cluster_size = 0;
        
        for (int d : resonance_map) {
            // High-precision wave interference calculation
            float acc = std::abs(std::sin(orbit * 0.001f + d * 0.0001f));
            
            if (acc > threshold) {
                return {d, acc, orbit, d}; // We found a Lock
            }
        }
        return {0, 0.0f, orbit, 0}; // No lock at this coordinate
    }

    int getMapSize() { return resonance_map.size(); }
};

// Binding code to allow JavaScript to call these functions
EMSCRIPTEN_BINDINGS(lattice_module) {
    value_object<Singularity>("Singularity")
        .field("angle", &Singularity::angle)
        .field("accuracy", &Singularity::accuracy)
        .field("node", &Singularity::node)
        .field("drift", &Singularity::drift);

    class_<UnifiedLattice>("UnifiedLattice")
        .constructor()
        .function("calibrate", &UnifiedLattice::calibrate)
        .function("probe", &UnifiedLattice::probe)
        .function("getMapSize", &UnifiedLattice::getMapSize);
}
