//
//  util.h
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-06.
//
#ifndef UTIL_H
#define UTIL_H
// Standard C headers
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

// X-Plane SDK
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"

// Include XPMP2 headers
#include "XPCAircraft.h"
#include "XPMPAircraft.h"
#include "XPMPMultiplayer.h"

/// Freeze all movements at the moment?
extern bool gbFreeze;

/// PI
constexpr double PI = 3.1415926535897932384626433832795028841971693993751;

/// Position of user's plane
static XPLMDataRef dr_x =
    XPLMFindDataRef("sim/flightmodel/position/local_x"); // double
static XPLMDataRef dr_y =
    XPLMFindDataRef("sim/flightmodel/position/local_y"); // double
static XPLMDataRef dr_z =
    XPLMFindDataRef("sim/flightmodel/position/local_z"); // double
static XPLMDataRef dr_heading =
    XPLMFindDataRef("sim/flightmodel/position/psi"); // float
static XPLMDataRef dr_time =
    XPLMFindDataRef("sim/time/total_running_time_sec"); // float

/// Distance of our simulated planes to the user's plane's position? [m]
constexpr float PLANE_DIST_M = 200.0f;
/// Radius of the circle the planes do [m]
constexpr float PLANE_RADIUS_M = 100.0f;
/// Altitude difference to stack the 3 planes one above the other [m]
constexpr float PLANE_STACK_ALT_M = 50.0f;
/// Time it shall take to fly/roll a full circle [seconds]
constexpr float PLANE_CIRCLE_TIME_S = 20.0f;
/// Time it shall take to fly/roll a full circle [minutes]
constexpr float PLANE_CIRCLE_TIME_MIN = PLANE_CIRCLE_TIME_S / 60.0f;
/// Assumed circumfence of one plane's tire (rough guess for commercial jet
/// planes)
constexpr float PLANE_TIRE_CIRCUMFENCE_M = 3.2f;
/// Engine / prop rotation assumptions: rotations per minute
constexpr float PLANE_PROP_RPM = 300.0f;

void LogMsg(const char *szMsg, ...);
int CBIntPrefsFunc(const char *, [[maybe_unused]] const char *item,
                   int defaultVal);
void CBPlaneNotifier(XPMPPlaneID inPlaneID,
                     XPMPPlaneNotification inNotification, void * /*inRefcon*/);
void DebugListLoadedSoundNames();

float GetTimeFragment();

float GetTimeUpDown();

/// Summarizes the 3 values of a position in the local coordinate system
struct positionTy {
    double x = 0.0f;
    double y = 0.0f;
    double z = 0.0f;
};

void CirclePos(positionTy &pos, float heading, float radius);
/// Convert local position to world coordinates
void ConvLocalToWorld(const positionTy &pos, double &lat, double &lon,
                      double &alt);

/// Finds a position 200m in front of the user's plane serving as the center for
/// further operations
positionTy FindCenterPos(float dist);

/// Convert from degree to radians
inline double deg2rad(const double deg) { return (deg * PI / 180.0); }
/// Save string copy
inline char *strScpy(char *dest, const char *src, size_t size) {
    strncpy(dest, src, size);
    dest[size - 1] = 0; // this ensures zero-termination!
    return dest;
}

#endif // UTIL_H
