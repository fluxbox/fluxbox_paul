-- click on the desktop to get menus
default_keymode['OnDesktop Mouse1'] = 'HideMenus'
default_keymode['OnDesktop Mouse2'] = 'WorkspaceMenu'
default_keymode['OnDesktop Mouse3'] = 'RootMenu'

-- scroll on the desktop to change workspaces
default_keymode['OnDesktop Mouse4'] = 'PrevWorkspace'
default_keymode['OnDesktop Mouse5'] = 'NextWorkspace'

-- scroll on the toolbar to change current window
default_keymode['OnToolbar Mouse4'] = 'PrevWindow {static groups} (iconhidden=no)'
default_keymode['OnToolbar Mouse5'] = 'NextWindow {static groups} (iconhidden=no)'

-- alt + left/right click to move/resize a window
default_keymode['OnWindow Mod1 Mouse1'] = 'MacroCmd {Raise} {Focus} {StartMoving}'
default_keymode['OnWindowBorder Move1'] = 'StartMoving'

default_keymode['OnWindow Mod1 Mouse3'] = 'MacroCmd {Raise} {Focus} {StartResizing NearestCorner}'
default_keymode['OnLeftGrip Move1'] = 'StartResizing bottomleft'
default_keymode['OnRightGrip Move1'] = 'StartResizing bottomright'

-- alt + middle click to lower the window
default_keymode['OnWindow Mod1 Mouse2'] = 'Lower'

-- control-click a window's titlebar and drag to attach windows
default_keymode['OnTitlebar Control Mouse1'] = 'StartTabbing'

-- double click on the titlebar to shade
default_keymode['OnTitlebar Double Mouse1'] = 'Shade'

-- left click on the titlebar to move the window
default_keymode['OnTitlebar Mouse1'] = 'MacroCmd {Raise} {Focus} {ActivateTab}'
default_keymode['OnTitlebar Move1 '] = 'StartMoving'

-- middle click on the titlebar to lower
default_keymode['OnTitlebar Mouse2'] = 'Lower'

-- right click on the titlebar for a menu of options
default_keymode['OnTitlebar Mouse3'] = 'WindowMenu'

-- alt-tab
default_keymode['Mod1 Tab'] = 'NextWindow {groups} (workspace=[current])'
default_keymode['Mod1 Shift Tab'] = 'PrevWindow {groups} (workspace=[current])'

-- cycle through tabs in the current window
default_keymode['Mod4 Tab'] = 'NextTab'
default_keymode['Mod4 Shift Tab'] = 'PrevTab'

-- go to a specific tab in the current window
default_keymode['Mod4 1'] = 'Tab 1'
default_keymode['Mod4 2'] = 'Tab 2'
default_keymode['Mod4 3'] = 'Tab 3'
default_keymode['Mod4 4'] = 'Tab 4'
default_keymode['Mod4 5'] = 'Tab 5'
default_keymode['Mod4 6'] = 'Tab 6'
default_keymode['Mod4 7'] = 'Tab 7'
default_keymode['Mod4 8'] = 'Tab 8'
default_keymode['Mod4 9'] = 'Tab 9'

-- open a terminal
default_keymode['Mod1 F1'] = 'Exec xterm'

-- open a dialog to run programs
default_keymode['Mod1 F2'] = 'Exec fbrun'

-- volume settings, using common keycodes
-- if these don't work, use xev to find out your real keycodes
default_keymode['176'] = 'Exec amixer sset Master,0 1+'
default_keymode['174'] = 'Exec amixer sset Master,0 1-'
default_keymode['160'] = 'Exec amixer sset Master,0 toggle'

-- current window commands
default_keymode['Mod1 F4'] = 'Close'
default_keymode['Mod1 F5'] = 'Kill'
default_keymode['Mod1 F9'] = 'Minimize'
default_keymode['Mod1 F10'] = 'Maximize'
default_keymode['Mod1 F11'] = 'Fullscreen'

-- open the window menu
default_keymode['Mod1 space'] = 'WindowMenu'

-- exit fluxbox
default_keymode['Control Mod1 Delete'] = 'Exit'

-- change to previous/next workspace
default_keymode['Control Mod1 Left'] = 'PrevWorkspace'
default_keymode['Control Mod1 Right'] = 'NextWorkspace'

-- send the current window to previous/next workspace
default_keymode['Mod4 Left'] = 'SendToPrevWorkspace'
default_keymode['Mod4 Right'] = 'SendToNextWorkspace'

-- send the current window and follow it to previous/next workspace
default_keymode['Control Mod4 Left'] = 'TakeToPrevWorkspace'
default_keymode['Control Mod4 Right'] = 'TakeToNextWorkspace'

-- change to a specific workspace
default_keymode['Control F1'] = 'Workspace 1'
default_keymode['Control F2'] = 'Workspace 2'
default_keymode['Control F3'] = 'Workspace 3'
default_keymode['Control F4'] = 'Workspace 4'
default_keymode['Control F5'] = 'Workspace 5'
default_keymode['Control F6'] = 'Workspace 6'
default_keymode['Control F7'] = 'Workspace 7'
default_keymode['Control F8'] = 'Workspace 8'
default_keymode['Control F9'] = 'Workspace 9'
default_keymode['Control F10'] = 'Workspace 10'
default_keymode['Control F11'] = 'Workspace 11'
default_keymode['Control F12'] = 'Workspace 12'

-- send the current window to a specific workspace
default_keymode['Mod4 F1'] = 'SendToWorkspace 1'
default_keymode['Mod4 F2'] = 'SendToWorkspace 2'
default_keymode['Mod4 F3'] = 'SendToWorkspace 3'
default_keymode['Mod4 F4'] = 'SendToWorkspace 4'
default_keymode['Mod4 F5'] = 'SendToWorkspace 5'
default_keymode['Mod4 F6'] = 'SendToWorkspace 6'
default_keymode['Mod4 F7'] = 'SendToWorkspace 7'
default_keymode['Mod4 F8'] = 'SendToWorkspace 8'
default_keymode['Mod4 F9'] = 'SendToWorkspace 9'
default_keymode['Mod4 F10'] = 'SendToWorkspace 10'
default_keymode['Mod4 F11'] = 'SendToWorkspace 11'
default_keymode['Mod4 F12'] = 'SendToWorkspace 12'

-- send the current window and change to a specific workspace
default_keymode['Control Mod4 F1'] = 'TakeToWorkspace 1'
default_keymode['Control Mod4 F2'] = 'TakeToWorkspace 2'
default_keymode['Control Mod4 F3'] = 'TakeToWorkspace 3'
default_keymode['Control Mod4 F4'] = 'TakeToWorkspace 4'
default_keymode['Control Mod4 F5'] = 'TakeToWorkspace 5'
default_keymode['Control Mod4 F6'] = 'TakeToWorkspace 6'
default_keymode['Control Mod4 F7'] = 'TakeToWorkspace 7'
default_keymode['Control Mod4 F8'] = 'TakeToWorkspace 8'
default_keymode['Control Mod4 F9'] = 'TakeToWorkspace 9'
default_keymode['Control Mod4 F10'] = 'TakeToWorkspace 10'
default_keymode['Control Mod4 F11'] = 'TakeToWorkspace 11'
default_keymode['Control Mod4 F12'] = 'TakeToWorkspace 12'
