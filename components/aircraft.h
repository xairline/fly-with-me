//
//  aircraft.h
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-06.
//

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

// Include XPMP2 headers
#include "util.h"

static constexpr float UPDATE_INTERVAL = 1.0f / 25.0f; // 30 FPS

using namespace XPMP2;

class RemoteAircraft : public Aircraft {
  private:
    float serverTimeOffset;

  public:
    /// Constructor
    RemoteAircraft(const std::string &_icaoType,
                   const std::string &_icaoAirline, const std::string &_livery,
                   XPMPPlaneID _modeS_id = 0, const std::string &_cslId = "");

    /// Custom implementation for updating aircraft position and state
    virtual void UpdatePosition(float, int) override;
};

#endif // AIRCRAFT_H
