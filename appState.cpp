//
//  appState.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-09.
//

#include "appState.h"

AppState *AppState::instance = nullptr;
XPLMDataRef AppState::planeLat = nullptr;
XPLMDataRef AppState::planeLon = nullptr;
XPLMDataRef AppState::planeEl = nullptr;

AppState::AppState() {
    /* Find the data refs we want to record. */
    planeLat = XPLMFindDataRef("sim/flightmodel/position/latitude");
    planeLon = XPLMFindDataRef("sim/flightmodel/position/longitude");
    planeEl = XPLMFindDataRef("sim/flightmodel/position/elevation");
}

AppState *AppState::GetInstance() {
    if (instance == nullptr) {
        instance = new AppState();
    }
    return instance;
}

void AppState::Initialize() {

    // Resister Pos report flight loop
    XPLMRegisterFlightLoopCallback(PosReportLoopCallback, -1.0f, NULL);
    
    // The path separation character, one out of /\:
    char pathSep = XPLMGetDirectorySeparator()[0];
    // The plugin's path, results in something like
    // ".../Resources/plugins/XPMP2-Sample/64/XPMP2-Sample.xpl"
    char szPath[256];
    XPLMGetPluginInfo(XPLMGetMyID(), nullptr, szPath, nullptr, nullptr);
    *(std::strrchr(szPath, pathSep)) = 0; // Cut off the plugin's file name
    *(std::strrchr(szPath, pathSep) + 1) =
        0; // Cut off the "64" directory name, but leave the dir separation
           // character
    // We search in a subdirectory named "Resources" for all we need
    std::string resourcePath = szPath;
    resourcePath +=
        "Resources"; // should now be something like
                     // ".../Resources/plugins/XPMP2-Sample/Resources"

    // Try initializing XPMP2:
    const char *res =
        XPMPMultiplayerInit("XPMP2-Sample",       // plugin name,
                            resourcePath.c_str(), // path to supplemental files
                            CBIntPrefsFunc, // configuration callback function
                            "C172");        // default ICAO type
    if (res[0]) {
        LogMsg("XPMP2-Sample: Initialization of XPMP2 failed: %s", res);
        return;
    }

    // Load our CSL models
    res = XPMPLoadCSLPackage(resourcePath.c_str()); // CSL folder root path
    if (res[0]) {
        LogMsg("XPMP2-Sample: Error while loading CSL packages: %s", res);
    }
    
    // Now we also try to get control of AI planes. That's optional, though,
    // other plugins (like LiveTraffic, XSquawkBox, X-IvAp...)
    // could have control already
    res = XPMPMultiplayerEnable();
    if (res[0]) {
        LogMsg("XPMP2-Sample: Could not enable AI planes: %s", res);
    }
    
    LogMsg("Plugin Path: %s", szPath);
    const std::string token = GetTokenFromFile(std::strcat(szPath, "config"));
    if (token != "") {
        // Launch the WebSocket connection on a new thread
        WebSocketClient &wsClient = WebSocketClient::getInstance();
        const std::string uri = "wss://app.xairline.org/apis/mp?auth=";
        wsClient.connect(uri + token);
    } else {
        LogMsg("Failed to get Token: check %s", szPath);
    }
    
    // Register the plane notifer function
    // (this is rarely used in actual applications, but used here for
    //  demonstration and testing purposes)
    XPMPRegisterPlaneNotifierFunc(CBPlaneNotifier, NULL);

}

void AppState::Deinitialize() {
    // Stop pos reporting flight loop
    XPLMUnregisterFlightLoopCallback(PosReportLoopCallback, NULL);

    // Remove the planes
    //    PlanesRemove();

    // Give up AI plane control
    XPMPMultiplayerDisable();

    // Unregister plane notifier (must match function _and_ refcon)
    XPMPUnregisterPlaneNotifierFunc(CBPlaneNotifier, NULL);

    // Properly cleanup the XPMP2 library
    XPMPMultiplayerCleanup();
}

// Flightloop, targetting 25 fps
float AppState::PosReportLoopCallback(float inElapsedSinceLastCall,
                                      float inElapsedTimeSinceLastFlightLoop,
                                      int inCounter, void *inRefcon) {
    if (inElapsedSinceLastCall > 0.05f) {
        LogMsg("FPS is too low: %f", inElapsedSinceLastCall);
    } else {
        LogMsg("FPS is OK: %f", inElapsedSinceLastCall);
    }
    float lat = XPLMGetDataf(planeLat);
    float lon = XPLMGetDataf(planeLon);
    float el = XPLMGetDataf(planeEl);

    // Create a comma-separated string from the values.
    std::string message = std::to_string(lat) + "," + std::to_string(lon) +
                          "," + std::to_string(el);
    // Send the binary message.
    try {
        WebSocketClient::getInstance().send(message);
    } catch (const websocketpp::lib::error_code &e) {
        LogMsg("Send error: %s", e.message().c_str());
    }

    return POS_LOOP_INTERVAL;
}
