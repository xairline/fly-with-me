//
//  interpolator.h
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-10.
//

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <deque>
#include <string>
#include <mutex>
#include <algorithm> // for std::lower_bound (if needed)

#include "util.h"

class Interpolator
{
public:
    // A simple struct for your positional data
    struct EntityState
    {
        int64_t timestamp;
        double lat;
        double lon;
        double el;
        double heading;
        double pitch;
        double roll;
    };

    Interpolator(int64_t);
    ~Interpolator() = default;

    // Delete copy semantics to ensure only one instance
    Interpolator(const Interpolator&) = delete;
    Interpolator& operator=(const Interpolator&) = delete;

    // Called whenever you receive a new WebSocket message containing an EntityState
    void OnWebSocketMessage(const std::string& msg);

    // Get interpolated state at a given renderTime
    EntityState getInterpolatedState(int64_t renderTime);
    int64_t serverTimeOffset;

private:
    std::deque<EntityState> m_buffer;   // Time-sorted buffer of states
    std::mutex m_mutex;                 // Protects m_buffer from concurrent access
    

    // Helper: Insert new state (already parsed) in a sorted manner or at the back,
    // ignoring out-of-order data if timestamp < the last stored timestamp.
    void addState(const EntityState& state);
};
#endif
