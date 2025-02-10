//
//  util.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-06.
//

#include "util.h"

bool gbFreeze = false;

/// Log a message to X-Plane's Log.txt with sprintf-style parameters
void LogMsg(const char *szMsg, ...) {
    char buf[512];
    char finalBuf[512];
    va_list args;
    // Write all the variable parameters
    va_start(args, szMsg);
    std::vsnprintf(buf, sizeof(buf) - 2, szMsg, args);
    va_end(args);
    std::strcat(buf, "\n");
    std::snprintf(finalBuf, sizeof(finalBuf), "[%s] %s\n", PLUGIN_NAME, buf);
    // write to log (flushed immediately -> expensive!)
    XPLMDebugString(finalBuf);
}

/// This is a callback the XPMP2 calls regularly to learn about configuration
/// settings.
int CBIntPrefsFunc(const char *, [[maybe_unused]] const char *item,
                   int defaultVal) {
    // We always want to replace dataRefs and textures upon load to make the
    // most out of the .obj files
    if (!strcmp(item, XPMP_CFG_ITM_REPLDATAREFS))
        return 1;
    if (!strcmp(item, XPMP_CFG_ITM_REPLTEXTURE))
        return 1; // actually...this is ON by default anyway, just to be sure
    // Contrails even close to the ground for demonstration purposes
    if (!strcmp(item, XPMP_CFG_ITM_CONTR_MIN_ALT))
        return 0;
    if (!strcmp(item, XPMP_CFG_ITM_CONTR_MULTI))
        return 1;
#if DEBUG
    // in debug version of the plugin we provide most complete log output
    if (!strcmp(item, XPMP_CFG_ITM_MODELMATCHING))
        return 0; // though...no model
    if (!strcmp(item, XPMP_CFG_ITM_LOGLEVEL))
        return 0; // DEBUG logging level
#endif
    // Otherwise we just accept defaults
    return defaultVal;
}

/// This is the callback for the plane notifier function, which just logs some
/// info to Log.txt
/// @note Plane notifier functions are completely optional and actually rarely
/// used,
///       because you should know already by other means when your plugin
///       creates/modifies/deletes a plane. So this is for pure demonstration
///       (and testing) purposes.
void CBPlaneNotifier(XPMPPlaneID inPlaneID,
                     XPMPPlaneNotification inNotification,
                     void * /*inRefcon*/) {
    XPMP2::Aircraft *pAc = XPMP2::AcFindByID(inPlaneID);
    if (pAc) {
        LogMsg("XPMP2-Sample: Plane of type %s, airline %s, model %s, label "
               "'%s' %s",
               pAc->acIcaoType.c_str(), pAc->acIcaoAirline.c_str(),
               pAc->GetModelName().c_str(), pAc->label.c_str(),
               inNotification == xpmp_PlaneNotification_Created ? "created"
               : inNotification == xpmp_PlaneNotification_ModelChanged
                   ? "changed"
                   : "destroyed");
    }
}

#if defined(DEBUG) && (INCLUDE_FMOD_SOUND + 0 >= 1)
/// Just for purposes of testing this functionality, we list all loaded sounds
void DebugListLoadedSoundNames() {
    int i = 0;
    const char *filePath = nullptr, *sndName = nullptr;
    for (sndName = XPMPSoundEnumerate(nullptr, &filePath); sndName != nullptr;
         sndName = XPMPSoundEnumerate(sndName, &filePath)) {
        LogMsg("XPMP2-Sample: %2d. Sound: `%s`, loaded from `%s`", ++i, sndName,
               filePath ? filePath : "?");
    }
}
#endif

/// Returns a number between 0.0 and 1.0, increasing over the course of 10
/// seconds, then restarting
float GetTimeFragment() {
    static float lastVal = 0.0f;

    // Just keep returning last value while frozen
    if (gbFreeze)
        return lastVal;

    const float t = XPLMGetDataf(dr_time);
    return lastVal = std::fmod(t, PLANE_CIRCLE_TIME_S) / PLANE_CIRCLE_TIME_S;
}

/// Returns a number between 0.0 and 1.0, going up and down over the course of
/// 10 seconds
float GetTimeUpDown() {
    static float lastVal = 0.0f;

    // Just keep returning last value while frozen
    if (gbFreeze)
        return lastVal;

    return lastVal =
               std::abs(std::fmod(XPLMGetDataf(dr_time), PLANE_CIRCLE_TIME_S) /
                            (PLANE_CIRCLE_TIME_S / 2.0f) -
                        1.0f);
}

/// Finds a position 200m in front of the user's plane serving as the center for
/// further operations
positionTy FindCenterPos(float dist) {
    // Current user's plane's position and heading (relative to Z)
    positionTy pos = {XPLMGetDatad(dr_x), XPLMGetDatad(dr_y),
                      XPLMGetDatad(dr_z)};
    float heading = XPLMGetDataf(dr_heading);

    // Move point 200m away from aircraft, direction of its heading
    const double head_rad = deg2rad(heading);
    pos.x += sin(head_rad) * dist; // east axis
    pos.z -= cos(head_rad) * dist; // south axis

    return pos;
}

/// Put the position on a circle around itself
void CirclePos(positionTy &pos, float heading, float radius) {
    const double head_rad = deg2rad(heading);
    pos.x += radius * sin(head_rad); // east axis
    pos.z -= radius * cos(head_rad); // south axis
}

/// Convert local position to world coordinates
void ConvLocalToWorld(const positionTy &pos, double &lat, double &lon,
                      double &alt) {
    XPLMLocalToWorld(pos.x, pos.y, pos.z, &lat, &lon, &alt);
}

std::string GetTokenFromFile(const std::string fullPath) {
    std::ifstream infile(fullPath);
    if (!infile.is_open()) {
        std::cerr << "Unable to open file: " << fullPath << std::endl;
        return "";
    }

    std::string line;
    if (std::getline(infile, line)) {
        // Find the position of the '=' character.
        size_t pos = line.find('=');
        if (pos != std::string::npos && pos + 1 < line.size()) {
            // Return everything after '='.
            return line.substr(pos + 1);
        } else {
            std::cerr << "The line does not contain '=' or no value follows it."
                      << std::endl;
        }
    } else {
        std::cerr << "Failed to read a line from the file." << std::endl;
    }
    return "";
}
