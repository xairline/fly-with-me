//
//  interpolator.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-10.
//

#include "Interpolator.h"
#include <algorithm> // for std::lower_bound (if needed)

Interpolator::Interpolator(int offset) { this->serverTimeOffset = offset; }

//------------------------------------------------------------------------------
// onWebSocketMessage
//------------------------------------------------------------------------------
void Interpolator::onWebSocketMessage(const std::string &msg) {
    // We lock our mutex so we can safely modify/read the buffer
    std::lock_guard<std::mutex> lock(m_mutex);

    try {
        std::vector<std::string> parsedMsg = splitString(msg, ',');
        EntityState newState;
        newState.timestamp = std::stod(parsedMsg[0]);
        newState.lat = std::stod(parsedMsg[2]);
        newState.lon = std::stod(parsedMsg[3]);
        newState.el = std::stod(parsedMsg[4]);
        newState.pitch = std::stod(parsedMsg[5]);
        newState.roll = std::stod(parsedMsg[6]);
        newState.heading = std::stod(parsedMsg[7]);

        // Add to our buffer, dropping if out of order
        addState(newState);
    } catch (const std::exception &e) {
        LogMsg("Error: on ws message: %s", e.what());
    }
}

//------------------------------------------------------------------------------
// addState
//------------------------------------------------------------------------------
void Interpolator::addState(const EntityState &state) {
    // If buffer is empty or the new timestamp is strictly newer, we push back
    if (m_buffer.empty() || state.timestamp > m_buffer.back().timestamp) {
        m_buffer.push_back(state);
    } else {
        // We "assume no out-of-order" data. So if it's older or equal, drop it.
        // (Alternatively, if you wanted to gracefully handle partial
        // out-of-order, you could insert with lower_bound or skip only if it's
        // truly older.)
        return;
    }

    // Optional: Trim old states to keep buffer from growing too large
    // e.g. keep only the last 1 second of data
    const double MAX_HISTORY_SEC = 1.0; // keep 1s of data
    double newestTime = m_buffer.back().timestamp;

    while (!m_buffer.empty() &&
           (newestTime - m_buffer.front().timestamp) > MAX_HISTORY_SEC) {
        m_buffer.pop_front();
    }
}

//------------------------------------------------------------------------------
// getInterpolatedState
//------------------------------------------------------------------------------
Interpolator::EntityState Interpolator::getInterpolatedState(int renderTime) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // If no data in the buffer, return a default object
    if (m_buffer.empty()) {
        return {renderTime, 0.0, 0.0, 0.0, 0.0, 0.0};
    }

    // If we only have one state or if renderTime is before the first
    if (m_buffer.size() == 1 || renderTime <= m_buffer.front().timestamp) {
        return m_buffer.front();
    }

    // If renderTime is beyond the latest known state, we can:
    // (A) Return the last state
    // (B) Or do short extrapolation
    if (renderTime >= m_buffer.back().timestamp) {
        // Return the last known state (simple approach)
        return m_buffer.back();

        // or short extrapolation, e.g.:
        /*
        const auto& last = m_buffer.back();
        double dt = renderTime - last.timestamp;
        EntityState extrap = last;
        extrap.timestamp = renderTime;
        // naive extrap (heading-based) for demonstration:
        double rad = last.heading * (3.14159265358979 / 180.0); // if heading in
        degrees extrap.x += last.groundSpeed * std::cos(rad) * dt; extrap.lon +=
        last.groundSpeed * std::sin(rad) * dt;
        // handle vertical speed if relevant
        return extrap;
        */
    }

    // Otherwise, find the two states that bracket "renderTime"
    // We know times are strictly increasing, so we can do a simple linear scan
    // or use std::lower_bound. For small buffers, a linear scan is fine.
    for (size_t i = 0; i < m_buffer.size() - 1; ++i) {
        const auto &sA = m_buffer[i];
        const auto &sB = m_buffer[i + 1];
        if (renderTime >= sA.timestamp && renderTime <= sB.timestamp) {
            // Interpolate
            double total = sB.timestamp - sA.timestamp;
            double portion = renderTime - sA.timestamp;
            double t = (total > 0.000001) ? (portion / total) : 0.0;

            EntityState interp;
            interp.timestamp = renderTime;
            interp.lat = sA.lat + (sB.lat - sA.lat) * t;
            interp.lon = sA.lon + (sB.lon - sA.lon) * t;
            interp.el = sA.el + (sB.el - sA.el) * t;
            interp.pitch = sA.pitch + (sB.pitch - sA.pitch) * t;
            interp.roll = sA.roll + (sB.roll - sA.roll) * t;
            interp.heading = sA.heading + (sB.heading - sA.heading) * t;
            return interp;
        }
    }

    // Should never get here if the buffer is consistent, but just in case:
    return m_buffer.back();
}
