
//
//  appState.h
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-09.
//
#ifndef MY_MENU_H
#define MY_MENU_H

// X-Plane SDK
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "util.h"

enum MenuItems {
    MENU_AI=0,                ///< Menu Item "Toggle AI control"
};

class Menu final {
public:
    static Menu* GetInstance();
    void Initialize();
    void Deinitialize();
    static void MenuHanlder(void * /*inMenuRef*/, void *inItemRef);
    
private:
    Menu();
    ~Menu();
    void menuUpdateCheckmarks();
    static Menu* instance;
    XPLMMenuID menuID = nullptr;
};
#endif // MY_MENU_H
