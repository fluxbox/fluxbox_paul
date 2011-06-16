// ScreenPlacement.cc
// Copyright (c) 2006 Fluxbox Team (fluxgen at fluxbox dot org)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "ScreenPlacement.hh"

#include "RowSmartPlacement.hh"
#include "MinOverlapPlacement.hh"
#include "UnderMousePlacement.hh"
#include "ColSmartPlacement.hh"
#include "CascadePlacement.hh"

#include "Screen.hh"
#include "Window.hh"

#include "FbTk/Menu.hh"
#include "FbTk/Luamm.hh"

#include <iostream>
#include <exception>
#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif
using std::cerr;
using std::endl;

ScreenPlacement::ScreenPlacement(BScreen &screen):
    m_row_direction(screen.resourceManager(), LEFTRIGHT,
                    screen.name()+".rowPlacementDirection"),
    m_col_direction(screen.resourceManager(), TOPBOTTOM,
                    screen.name()+".colPlacementDirection"),
    m_placement_policy(screen.resourceManager(), ROWMINOVERLAPPLACEMENT,
                       screen.name()+".windowPlacement"),
    m_old_policy(ROWSMARTPLACEMENT),
    m_strategy(0),
    m_screen(screen)
{
}

bool ScreenPlacement::placeWindow(const FluxboxWindow &win, int head,
                                  int &place_x, int &place_y) {


    // check the resource placement and see if has changed
    // and if so update the strategy
    if (m_old_policy != *m_placement_policy || !m_strategy.get()) {
        m_old_policy = *m_placement_policy;
        switch (*m_placement_policy) {
        case ROWSMARTPLACEMENT:
            m_strategy.reset(new RowSmartPlacement());
            break;
        case COLSMARTPLACEMENT:
            m_strategy.reset(new ColSmartPlacement());
            break;
        case ROWMINOVERLAPPLACEMENT:
        case COLMINOVERLAPPLACEMENT:
            m_strategy.reset(new MinOverlapPlacement());
            break;
        case CASCADEPLACEMENT:
            m_strategy.reset(new CascadePlacement(win.screen()));
            break;
        case UNDERMOUSEPLACEMENT:
            m_strategy.reset(new UnderMousePlacement());
            break;
        }
    }

    // view (screen + head) constraints
    int head_left = (signed) win.screen().maxLeft(head);
    int head_right = (signed) win.screen().maxRight(head);
    int head_top = (signed) win.screen().maxTop(head);
    int head_bot = (signed) win.screen().maxBottom(head);

    // start placement, top left corner
    place_x = head_left;
    place_y = head_top;

    bool placed = false;
    try {
        placed = m_strategy->placeWindow(win, head, place_x, place_y);
    } catch (std::bad_cast cast) {
        // This should not happen. 
        // If for some reason we change the PlacementStrategy in Screen
        // from ScreenPlacement to something else then we might get 
        // bad_cast from some placement strategies.
        cerr<<"Failed to place window: "<<cast.what()<<endl;
    }

    if (!placed) {
        // Create fallback strategy, when we need it the first time
        // This strategy must succeed!
        if (m_fallback_strategy.get() == 0)
            m_fallback_strategy.reset(new CascadePlacement(win.screen()));

        m_fallback_strategy->placeWindow(win, head, place_x, place_y);
    }



    int win_w = win.normalWidth() + win.fbWindow().borderWidth()*2 + win.widthOffset(),
        win_h = win.normalHeight() + win.fbWindow().borderWidth()*2 + win.heightOffset();

    // make sure the window is inside our screen(head) area
    if (place_x + win_w - win.xOffset() > head_right)
        place_x = head_left + (head_right - head_left - win_w) / 2 +
                  win.xOffset();
    if (place_y + win_h - win.yOffset() > head_bot)
        place_y = head_top + (head_bot - head_top - win_h) / 2 + win.yOffset();

    return true;
}

void ScreenPlacement::placeAndShowMenu(FbTk::Menu& menu, int x, int y, bool respect_struts) {

    int head = m_screen.getHead(x, y);

    menu.setScreen(m_screen.getHeadX(head),
        m_screen.getHeadY(head),
        m_screen.getHeadWidth(head),
        m_screen.getHeadHeight(head));

    menu.updateMenu(); // recalculate the size

    x = x - (menu.width() / 2);
    if (menu.isTitleVisible())
        y = y - (menu.titleWindow().height() / 2);

    // adjust (x, y) to fit on the screen
    if (!respect_struts) {

        int bw = 2 * menu.fbwindow().borderWidth();
        std::pair<int, int> pos = m_screen.clampToHead(head, x, y, menu.width() + bw, menu.height() + bw);
        x = pos.first;
        y = pos.second;

    } else { // do not cover toolbar if no title

        int top = static_cast<signed>(m_screen.maxTop(head));
        int bottom = static_cast<signed>(m_screen.maxBottom(head));
        int left = static_cast<signed>(m_screen.maxLeft(head));
        int right = static_cast<signed>(m_screen.maxRight(head));

        if (y < top)
            y = top;
        else if (y + static_cast<signed>(menu.height()) >= bottom)
            y = bottom - menu.height() - 1 - menu.fbwindow().borderWidth();

        if (x < left)
            x = left;
        else if (x + static_cast<signed>(menu.width()) >= right)
            x = right - static_cast<int>(menu.width()) - 1;
    }

    menu.move(x, y);
    menu.show();
    menu.grabInputFocus();
}

////////////////////// Placement Resources
namespace FbTk {

template <>
const EnumTraits<ScreenPlacement::PlacementPolicy>::Pair EnumTraits<ScreenPlacement::PlacementPolicy>::s_map[] = {
    { "RowSmartPlacement",      ScreenPlacement::ROWSMARTPLACEMENT },
    { "ColSmartPlacement",      ScreenPlacement::COLSMARTPLACEMENT },
    { "RowMinOverlapPlacement", ScreenPlacement::ROWMINOVERLAPPLACEMENT },
    { "ColMinOverlapPlacement", ScreenPlacement::COLMINOVERLAPPLACEMENT },
    { "UnderMousePlacement",    ScreenPlacement::UNDERMOUSEPLACEMENT },
    { "CascadePlacement",       ScreenPlacement::CASCADEPLACEMENT },
    { NULL,                     ScreenPlacement::CASCADEPLACEMENT }
};

template <>
const EnumTraits<ScreenPlacement::RowDirection>::Pair EnumTraits<ScreenPlacement::RowDirection>::s_map[] = {
    { "LeftToRight",            ScreenPlacement::LEFTRIGHT },
    { "RightToLeft",            ScreenPlacement::RIGHTLEFT },
    { NULL,                     ScreenPlacement::RIGHTLEFT },
};

template <>
const EnumTraits<ScreenPlacement::ColumnDirection>::Pair EnumTraits<ScreenPlacement::ColumnDirection>::s_map[] = {
    { "TopToBottom",            ScreenPlacement::TOPBOTTOM },
    { "BottomToTop",            ScreenPlacement::BOTTOMTOP },
    { NULL,                     ScreenPlacement::BOTTOMTOP },
};

} // end namespace FbTk
