//
//  menu.cpp
//  XPMP2-Sample
//
//  Created by Di Zou on 2025-02-09.
//

#include "menu.h"

Menu* Menu::instance = nullptr;

Menu::Menu(){
    
}

Menu *Menu::GetInstance() {
    if (instance == nullptr) {
        instance = new Menu();
    }

    return instance;
}

void Menu::Initialize() {
    int my_slot =
        XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Fly With Me", NULL, 0);
    menuID = XPLMCreateMenu("Fly With Me", XPLMFindPluginsMenu(), my_slot,
                           MenuHanlder, NULL);
    XPLMAppendMenuItem(menuID, "Toggle AI control", (void *)MENU_AI, 0);
    menuUpdateCheckmarks();
}

void Menu::MenuHanlder(void *, void *inItemRef) {
    switch (MenuItems(reinterpret_cast<unsigned long long>(inItemRef))) {

        case MENU_AI: // Toggle AI control?
            if (XPMPHasControlOfAIAircraft())
                XPMPMultiplayerDisable();
            else
                // When requested by menu we actually wait via callback to get
                // control
                XPMPMultiplayerEnable();
            break;
        }

        // Update menu items' checkmarks
    Menu::GetInstance()->menuUpdateCheckmarks();
}


void Menu::menuUpdateCheckmarks() {
    XPLMCheckMenuItem(menuID, MENU_AI,
                      XPMPHasControlOfAIAircraft() ? xplm_Menu_Checked
                                                   : xplm_Menu_Unchecked);
}
