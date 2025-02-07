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

// Include other modules
#include "aircraft.h"
#include "menu.h"
#include "util.h"
#include "websocket.h"

#if !XPLM300
#error This plugin requires version 300 of the SDK
#endif

/// Initial type / airline / livery to be used to create our 3 planes
/// @see https://www.icao.int/publications/DOC8643/Pages/Search.aspx for ICAO
/// aircraft types
/// @see
/// https://forums.x-plane.org/index.php?/files/file/37041-bluebell-obj8-csl-packages/
/// for the Bluebell package, which includes the models named here
std::string PLANE_MODEL[3][3] = {
    {"A321", "EUK", ""}, // Don't have A321_EUK, shall find A320_EIN via
                         // related.txt and relOp.txt
    {"B06", "TXB", ""},
    {"DH8D", "BER", ""},
};

//
// MARK: Using XPMP2 - New XPMP2::Aircraft class
//       This is the new and recommended way of using the library:
//       Deriving a class from XPMP2::Aircraft and providing
//       a custom implementation for UpdatePosition(),
//       which provides all current values in one go directly into
//       the member variables, which are later on used for
//       controlling the plane objects. This avoids any unnecessary copying
//       within the library

using namespace XPMP2;

/// The one aircraft of this type that we manage
RemoteAircraft *pSamplePlane = nullptr;

//
// MARK: Menu functionality
//

/// Is any plane object created?
inline bool ArePlanesCreated() { return pSamplePlane; }

/// Create our 3 planes (if they don't exist already)
void PlanesCreate() {
    // 1. New interface of XPMP2::Aircraft class
    if (!pSamplePlane)
        try {
            pSamplePlane =
                new RemoteAircraft(PLANE_MODEL[gModelIdxBase][0],  // type
                                   PLANE_MODEL[gModelIdxBase][1],  // airline
                                   PLANE_MODEL[gModelIdxBase][2]); // livery
        } catch (const XPMP2::XPMP2Error &e) {
            LogMsg("Could not create object of type SampleAircraft: %s",
                   e.what());
            pSamplePlane = nullptr;
        }

    // Put a checkmark in front of menu item if planes are visible
    XPLMCheckMenuItem(
        hMenu, 0, ArePlanesCreated() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    XPLMCheckMenuItem(hMenu, 1,
                      gbVisible ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

void MenuUpdateCheckmarks() {
    XPLMCheckMenuItem(hMenu, MENU_AI,
                      XPMPHasControlOfAIAircraft() ? xplm_Menu_Checked
                                                   : xplm_Menu_Unchecked);
}

/// Remove all planes
void PlanesRemove() {
    if (pSamplePlane) {
        delete pSamplePlane;
        pSamplePlane = nullptr;
    }

    // Remove the checkmark in front of menu item
    XPLMCheckMenuItem(hMenu, 0, xplm_Menu_Unchecked);
    XPLMCheckMenuItem(hMenu, 1, xplm_Menu_Unchecked);
}

/// Callback function for the case that we might get AI access later
void CPRequestAIAgain(void *) {
    // Well...we just try again ;-)
    XPMPMultiplayerEnable(CPRequestAIAgain);
    MenuUpdateCheckmarks();
}

/// Callback function for menu
void CBMenu(void * /*inMenuRef*/, void *inItemRef) {
    switch (MenuItemsTy(reinterpret_cast<unsigned long long>(inItemRef))) {

    case MENU_AI: // Toggle AI control?
        if (XPMPHasControlOfAIAircraft())
            XPMPMultiplayerDisable();
        else
            // When requested by menu we actually wait via callback to get
            // control
            XPMPMultiplayerEnable(CPRequestAIAgain);
        break;
    }

    // Update menu items' checkmarks
    MenuUpdateCheckmarks();
}

//
// MARK: Standard Plugin Callbacks
//

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    std::strcpy(outName, "XPMP2-Sample");
    std::strcpy(outSig, "TwinFan.plugin.XPMP2-Sample");
    std::strcpy(outDesc, "Sample plugin demonstrating using XPMP2 library");

    // use native paths, i.e. Posix style (as opposed to HFS style)
    // https://developer.x-plane.com/2014/12/mac-plugin-developers-you-should-be-using-native-paths/

    /* Disable next line only for testing purposes: Does XPMP2 also handle HFS
     * well? */
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    // Create the menu for the plugin
    int my_slot =
        XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XPMP2 Sample", NULL, 0);
    hMenu = XPLMCreateMenu("XPMP2 Sample", XPLMFindPluginsMenu(), my_slot,
                           CBMenu, NULL);
    XPLMAppendMenuItem(hMenu, "Toggle AI control", (void *)MENU_AI, 0);
    MenuUpdateCheckmarks();
    return 1;
}

PLUGIN_API void XPluginStop(void) {}

PLUGIN_API int XPluginEnable(void) {
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
        return 0;
    }

    // Load our CSL models
    res = XPMPLoadCSLPackage(resourcePath.c_str()); // CSL folder root path
    if (res[0]) {
        LogMsg("XPMP2-Sample: Error while loading CSL packages: %s", res);
    }

#if defined(DEBUG) && (INCLUDE_FMOD_SOUND + 0 >= 1)
    // Just for purposes of testing this functionality, we list all loaded
    // sounds (This is likely not required in your plugin)
    DebugListLoadedSoundNames();
#endif

    // Now we also try to get control of AI planes. That's optional, though,
    // other plugins (like LiveTraffic, XSquawkBox, X-IvAp...)
    // could have control already
    res = XPMPMultiplayerEnable(CPRequestAIAgain);
    if (res[0]) {
        LogMsg("XPMP2-Sample: Could not enable AI planes: %s", res);
    }

    // Register the plane notifer function
    // (this is rarely used in actual applications, but used here for
    //  demonstration and testing purposes)
    XPMPRegisterPlaneNotifierFunc(CBPlaneNotifier, NULL);

    // *** Create the planes ***
    PlanesCreate();

    // Success
    MenuUpdateCheckmarks();
    LogMsg("XPMP2-Sample: Enabled");
    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    // Remove the planes
    PlanesRemove();

    // Give up AI plane control
    XPMPMultiplayerDisable();

    // Unregister plane notifier (must match function _and_ refcon)
    XPMPUnregisterPlaneNotifierFunc(CBPlaneNotifier, NULL);

    // Properly cleanup the XPMP2 library
    XPMPMultiplayerCleanup();

    LogMsg("XPMP2-Sample: Disabled");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, long inMsg, void *) {
    // Some other plugin wants TCAS/AI control, so we (as an artificial
    // traffic plugin) give up
    if (inMsg == XPLM_MSG_RELEASE_PLANES) {
        XPMPMultiplayerDisable();
        MenuUpdateCheckmarks();
    }
}
