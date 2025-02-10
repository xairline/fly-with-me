//
//  appState.h
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-09.
//
#ifndef APP_STATE_H
#define APP_STATE_H

#define POS_LOOP_INTERVAL 0.05f // 1/20
// X-Plane SDK
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "util.h"
#include "menu.h"
#include "websocket.h"
#include "aircraft.h"
#include "interpolator.h"

struct NetworkAircraft {
    RemoteAircraft* remotePlane;
    Interpolator* interpolator;
};

class AppState final {
  public:
    static AppState *GetInstance();
    std::map<std::string,NetworkAircraft*> remotePlanes;
    void Initialize();
    void Deinitialize();
    static float PosReportLoopCallback(float inElapsedSinceLastCall,
                                float inElapsedTimeSinceLastFlightLoop,
                                int inCounter, void *inRefcon);
    void OnWebSocketMessage(const std::string &msg);

  private:
    AppState();
    // Destructor
    ~AppState();
    static AppState *instance;

    static XPLMDataRef planeLat;
    static XPLMDataRef planeLon;
    static XPLMDataRef planeEl;
    static XPLMDataRef planePitch;
    static XPLMDataRef planeHeading;
    static XPLMDataRef planeRoll;
    
    std::mutex m_mutex;
    std::vector<std::string> remoteAircraftInfo;
    
    
};
#endif // APP_STATE_H
