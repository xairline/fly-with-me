//
//  aircraft.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-06.
//

#include "aircraft.h"

RemoteAircraft::RemoteAircraft(Interpolator *interpolator,
                               const std::string clientId,
                               const std::string &_icaoType,
                               const std::string &_icaoAirline,
                               const std::string &_livery,
                               XPMPPlaneID _modeS_id, const std::string &_cslId)
    : Aircraft(_icaoType, _icaoAirline, _livery, _modeS_id, _cslId) {

    this->clientId = clientId;
    this->interpolator = interpolator;

    // Label
    label = clientId;
    colLabel[0] = 0.0f; // green
    colLabel[1] = 1.0f;
    colLabel[2] = 0.0f;

    // Radar
    acRadar.code = 7654;
    acRadar.mode = xpmpTransponderMode_ModeC;

    // informational texts
    strScpy(acInfoTexts.icaoAcType, _icaoType.c_str(),
            sizeof(acInfoTexts.icaoAcType));
    strScpy(acInfoTexts.icaoAirline, _icaoAirline.c_str(),
            sizeof(acInfoTexts.icaoAirline));
    strScpy(acInfoTexts.tailNum, "D-EVEL", sizeof(acInfoTexts.tailNum));
}

void RemoteAircraft::UpdatePosition(float _elapsedSinceLastCall, int) {
    auto now = std::chrono::system_clock::now();
    auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count();
    int64_t epoch_ms_int = static_cast<int64_t>(epoch_ms);

    auto newState = this->interpolator->getInterpolatedState(
        epoch_ms_int - this->interpolator->serverTimeOffset - 100);

    newState.el /= M_per_FT; // we need elevation in feet

    newState.lat += 0.000449;
    newState.lon += 0.000449;

    // So, here we tell the plane its position, which takes care of vertical
    // offset, too
    SetLocation(newState.lat, newState.lon, newState.el, false);

    // further attitude information
    SetPitch(newState.pitch);
    SetHeading(newState.heading);
    SetRoll(newState.roll);

    // Plane configuration info
    // This fills a large array of float values:
    const float r = GetTimeUpDown(); // a value between 0 and 1
    SetGearRatio(r);
    SetNoseWheelAngle(r * 90.0f - 45.0f); // turn nose wheel -45°..+45°
    SetFlapRatio(r);
    SetSpoilerRatio(r);
    SetSpeedbrakeRatio(r);
    SetSlatRatio(r);
    SetWingSweepRatio(0.0f);
    SetThrustRatio(0.5f);
    SetYokePitchRatio(r);
    SetYokeHeadingRatio(r);
    SetYokeRollRatio(r);

    // lights
    SetLightsTaxi(false);
    SetLightsLanding(false);
    SetLightsBeacon(true);
    SetLightsStrobe(true);
    SetLightsNav(true);

    // tires don't roll in the air
    SetTireDeflection(0.0f);
    SetTireRotAngle(0.0f);
    SetTireRotRpm(0.0f); // also sets the rad/s value!

    // For simplicity, we keep engine and prop rotation identical...probably
    // unrealistic
    SetEngineRotRpm(1, PLANE_PROP_RPM); // also sets the rad/s value!
    // 2nd engine shall turn 4 times slower...
    SetEngineRotRpm(2, PLANE_PROP_RPM / 4); // also sets the rad/s value!

    SetPropRotRpm(PLANE_PROP_RPM); // also sets the rad/s value!

    // Current position of engine / prop: keeps turning as per engine/prop
    // speed:
    float deg = std::fmod(PLANE_PROP_RPM * PLANE_CIRCLE_TIME_MIN *
                              GetTimeFragment() * 360.0f,
                          360.0f);
    SetEngineRotAngle(1, deg);
    // 2nd engine shall turn 4 times slower...
    deg = std::fmod(PLANE_PROP_RPM / 4 * PLANE_CIRCLE_TIME_MIN *
                        GetTimeFragment() * 360.0f,
                    360.0f);
    SetEngineRotAngle(2, deg);

    SetPropRotAngle(deg);

    // no reversers and no moment of touch-down in flight
    SetThrustReversRatio(0.0f);
    SetReversDeployRatio(0.0f);
    SetTouchDown(false);
}
