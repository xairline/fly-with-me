//
//  menu.h
//  fly-with-me
//
//  Created by Di Zou on 2025-02-06.
//

#ifndef MENU_H
#define MENU_H

/// menu id of our plugin's menu
XPLMMenuID hMenu = nullptr;


/// List of all menu item indexes
enum MenuItemsTy {
    MENU_AI=0,                ///< Menu Item "Toggle AI control"
};

/// Planes currently visible?
bool gbVisible = true;

/// Labels currently shown in map view?
bool gbMapLabels = true;

/// for cycling CSL models: what is the index used for the first plane?
int gModelIdxBase = 0;

#endif MENU_H
