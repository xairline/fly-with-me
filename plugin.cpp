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
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

// Include XPMP2 headers
#include "XPCAircraft.h"
#include "XPMPAircraft.h"
#include "XPMPMultiplayer.h"

// Include other modules
#include "aircraft.h"
#include "appState.h"
#include "menu.h"
#include "util.h"

#if !XPLM300
#error This plugin requires version 300 of the SDK
#endif

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    std::strcpy(outName, "fly-with-me");
    std::strcpy(outSig, "org.xairline.fly-with-me");
    std::strcpy(outDesc, "Simple multiplayers plugin");
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    Menu::GetInstance()->Initialize();
    return 1;
}

PLUGIN_API int XPluginEnable(void) {

    AppState::GetInstance()->Initialize();
    LogMsg("XPMP2-Sample: Enabled");
    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    AppState::GetInstance()->Deinitialize();
    LogMsg("Disabled");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, long inMsg, void *) {
    // Some other plugin wants TCAS/AI control, so we (as an artificial
    // traffic plugin) give up
    if (inMsg == XPLM_MSG_RELEASE_PLANES) {
        XPMPMultiplayerDisable();
    }
}
