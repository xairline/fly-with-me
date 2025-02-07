//
//  aircraft.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-06.
//

#include "aircraft.h"

RemoteAircraft::RemoteAircraft(const std::string &_icaoType,
                               const std::string &_icaoAirline,
                               const std::string &_livery,
                               XPMPPlaneID _modeS_id, const std::string &_cslId)
    : Aircraft(_icaoType, _icaoAirline, _livery, _modeS_id, _cslId) {
    // in our sample implementation, label, radar and info texts
    // are not dynamic. In others, they might be, then update them
    // in UpdatePosition()

    // Label
    label = "XPMP2::Aircraft";
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

    static float accumulatedTime = 0.0f;
    
    LogMsg("Elapsed: %f", _elapsedSinceLastCall);
    
    accumulatedTime += _elapsedSinceLastCall;
    if (accumulatedTime < UPDATE_INTERVAL) {
        return; // Skip update if not enough time has passed
    }
    LogMsg("Accumulated time: %f", accumulatedTime);
    accumulatedTime = 0.0f;

    // Calculate the plane's position
    const float angle = std::fmod(360.0f * GetTimeFragment(), 360.0f);
    positionTy pos = FindCenterPos(PLANE_DIST_M); // relative to user's plane
    CirclePos(pos, angle, PLANE_RADIUS_M);        // turning around a circle
    pos.y += PLANE_STACK_ALT_M * 2;               // 100m above user's aircraft

    // Strictly speaking...this is not necessary, we could just write
    // directly to drawInfo.x/y/z with above values (for y: + GetVertOfs()),
    // but typically in a real-world application you would actually
    // have lat/lon/elev...and then the call to SetLocation() is
    // what you shall do:
    double lat, lon, elev;
    // location in lat/lon/feet
    XPLMLocalToWorld(pos.x, pos.y, pos.z, &lat, &lon, &elev);
    elev /= M_per_FT; // we need elevation in feet

    // So, here we tell the plane its position, which takes care of vertical
    // offset, too
    SetLocation(lat, lon, elev, false);

    // further attitude information
    SetPitch(0.0f);
    SetHeading(std::fmod(90.0f + angle, 360.0f));
    SetRoll(20.0f);

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
