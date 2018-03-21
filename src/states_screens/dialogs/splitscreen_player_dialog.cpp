//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "states_screens/dialogs/splitscreen_player_dialog.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "network/network_config.hpp"
#include "states_screens/networking_lobby.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;

// ----------------------------------------------------------------------------
void SplitscreenPlayerDialog::beforeAddingWidgets()
{
    m_profiles = getWidget<SpinnerWidget>("name-spinner");
    for (unsigned i = 0; i < PlayerManager::get()->getNumPlayers(); i++)
        m_profiles->addLabel(PlayerManager::get()->getPlayer(i)->getName());

    m_message = getWidget<LabelWidget>("message-label");

    m_handicap = getWidget<CheckBoxWidget>("handicap");
    m_handicap->setState(false);
    m_handicap->setActive(UserConfigParams::m_per_player_difficulty);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);

    m_add = getWidget<IconButtonWidget>("add");
    assert(m_add != NULL);
    m_connect = getWidget<IconButtonWidget>("connect");
    assert(m_connect != NULL);
    m_cancel = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel != NULL);
    m_reset = getWidget<IconButtonWidget>("reset");
    assert(m_reset != NULL);

    input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
    input_manager->getDeviceManager()->mapFireToSelect(false);

}   // beforeAddingWidgets

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    SplitscreenPlayerDialog::processEvent(const std::string& source)
{
    if (source == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget
            ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_add->m_properties[PROP_ID])
        {
            const unsigned pid = m_profiles->getValue();
            assert(pid < PlayerManager::get()->getNumPlayers());
            PlayerProfile* p = PlayerManager::get()->getPlayer(pid);
            const bool handicap = m_handicap->getState();
            if (NetworkConfig::get()->addNetworkPlayer(m_device, p, handicap))
            {
                NetworkingLobby::getInstance()
                    ->addSplitscreenPlayer(p->getName());
                m_self_destroy = true;
                return GUIEngine::EVENT_BLOCK;
            }
            else
            {
                //I18N: in splitscreen player dialog for network game
                m_message->setErrorColor();
                m_message->setText(_("Player or input device already exists."),
                    false);
            }
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_connect->m_properties[PROP_ID])
        {
            if (!NetworkConfig::get()->getNetworkPlayers().empty())
            {
                NetworkConfig::get()->doneAddingNetworkPlayers();
                NetworkingLobby::getInstance()->finishAddingPlayers();
                m_self_destroy = true;
                return GUIEngine::EVENT_BLOCK;
            }
            //I18N: in splitscreen player dialog for network game
            m_message->setErrorColor();
            m_message->setText(
                _("No player available for connecting to server."), false);
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_cancel->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_reset->m_properties[PROP_ID])
        {
            NetworkConfig::get()->cleanNetworkPlayers();
            NetworkingLobby::getInstance()->cleanAddedPlayers();
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent