
#include "GUIPlexDefaultActionHandler.h"
#include "PlexExtraDataLoader.h"
#include "Application.h"
#include "PlexApplication.h"
#include "Playlists/PlexPlayQueueManager.h"
#include "guilib/GUIWindowManager.h"
#include <boost/foreach.hpp>
#include "dialogs/GUIDialogYesNo.h"
#include "GUIBaseContainer.h"
#include "Client/PlexServerManager.h"
#include "ApplicationMessenger.h"
#include "VideoInfoTag.h"
#include "GUIMessage.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
CGUIPlexDefaultActionHandler::CGUIPlexDefaultActionHandler()
{
  ACTION_SETTING* action;
  
  action = new ACTION_SETTING(ACTION_PLAYER_PLAY);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = false;
  action->WindowSettings[WINDOW_PLEX_PLAY_QUEUE].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_PLEX_PLAY_ALL);
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = false;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_PLEX_SHUFFLE_ALL);
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);

  action = new ACTION_SETTING(ACTION_PLEX_NOW_PLAYING);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_PLEX_PLAY_QUEUE].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_PLEX_PLAY_TRAILER);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_PLEX_PLAY_QUEUE].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);

  action = new ACTION_SETTING(ACTION_QUEUE_ITEM);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);

  action = new ACTION_SETTING(ACTION_PLEX_PQ_ADDUPTONEXT);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_MARK_AS_WATCHED);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_MARK_AS_UNWATCHED);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
  
  action = new ACTION_SETTING(ACTION_TOGGLE_WATCHED);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = false;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = false;
  m_ActionSettings.push_back(*action);

  action = new ACTION_SETTING(ACTION_PLEX_PQ_CLEAR);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_PLEX_PLAY_QUEUE].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);

  action = new ACTION_SETTING(ACTION_DELETE_ITEM);
  action->WindowSettings[WINDOW_HOME].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_PLEX_PLAY_QUEUE].contextMenuVisisble = true;
  action->WindowSettings[WINDOW_VIDEO_NAV].contextMenuVisisble = true;
  m_ActionSettings.push_back(*action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CGUIPlexDefaultActionHandler::GetContextButtons(int windowID, CFileItemPtr item, CFileItemListPtr container, CContextButtons& buttons)
{
  // check if the action is supported
  for (ActionsSettingListIterator it = m_ActionSettings.begin(); it != m_ActionSettings.end(); ++it)
  {
    ActionWindowSettingsMapIterator itwin = it->WindowSettings.find(windowID);
    if ((itwin != it->WindowSettings.end()) && (itwin->second.contextMenuVisisble))
    {
      GetContextButtonsForAction(it->actionID, item, container, buttons);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::OnAction(int windowID, CAction action, CFileItemPtr item, CFileItemListPtr container)
{
  CGUIWindow* window = g_windowManager.GetWindow(windowID);
  int actionID = action.GetID();

  // if the action is not known, then just exit
  ACTION_SETTING* setting = NULL;
  for (ActionsSettingListIterator it = m_ActionSettings.begin(); it != m_ActionSettings.end(); ++it)
  {
    if (it->actionID == action.GetID())
    {
      setting = &(*it);
      break;
    }
  }

  if (!setting)
    return false;

  // if the action is known, but not available for the window, then exit
  if (setting->WindowSettings.find(windowID) == setting->WindowSettings.end())
    return false;

  if (item)
  {
    // actions that require an item
    switch (actionID)
    {
      case ACTION_PLAYER_PLAY:
        PlayMedia(item, container);
        return true;
        break;
        
      case ACTION_PLEX_PLAY_TRAILER:

        if (item->GetPlexDirectoryType() == PLEX_DIR_TYPE_MOVIE)
        {
          CPlexExtraDataLoader loader;
          if (loader.getDataForItem(item) && loader.getItems()->Size())
          {
            /// we dont want quality selection menu for trailers
            CFileItem trailerItem(*loader.getItems()->Get(0));
            trailerItem.SetProperty("avoidPrompts", true);
            g_application.PlayFile(trailerItem, true);
            
          }
        }
        return true;
        break;

      case ACTION_MARK_AS_WATCHED:
        if (item->IsVideo() && item->IsPlexMediaServerLibrary())
        {
          item->MarkAsWatched(true);
          g_windowManager.SendMessage(GUI_MSG_PLEX_ITEM_WATCHEDSTATE_CHANGED, 0, windowID, actionID, 0);
          return true;
        }
        break;
        
      case ACTION_MARK_AS_UNWATCHED:
        if (item->IsVideo() && item->IsPlexMediaServerLibrary())
        {
          item->MarkAsUnWatched(true);
          g_windowManager.SendMessage(GUI_MSG_PLEX_ITEM_WATCHEDSTATE_CHANGED, 0, windowID, actionID, 0);
          return true;
        }
        break;
        
      case ACTION_TOGGLE_WATCHED:
        if (item->IsVideo() && item->IsPlexMediaServerLibrary())
        {
          if (item->GetVideoInfoTag()->m_playCount == 0)
            return OnAction(windowID, ACTION_MARK_AS_WATCHED, item, container);
          if (item->GetVideoInfoTag()->m_playCount > 0)
            return OnAction(windowID, ACTION_MARK_AS_UNWATCHED, item, container);
          break;
        }
        
      case ACTION_PLEX_PQ_CLEAR:
        if (IsPlayQueueContainer(container))
        {
          g_plexApplication.playQueueManager->clear();
          return true;
        }
        break;

      case ACTION_DELETE_ITEM:
        // if we are on a PQ item, remove it from PQ
        if (IsPlayQueueContainer(container))
        {
          g_plexApplication.playQueueManager->removeItem(item);
          return true;
        }
        else
        {
          // we're one a regular item, try to delete it
          // Confirm.
          if (!CGUIDialogYesNo::ShowAndGetInput(750, 125, 0, 0))
            return false;

          g_plexApplication.mediaServerClient->deleteItem(item);

          /* marking as watched and is on the on deck list, we need to remove it then */
          CGUIBaseContainer* container = (CGUIBaseContainer*)window->GetFocusedControl();
          if (container)
          {
            std::vector<CGUIListItemPtr> items = container->GetItems();
            int idx = std::distance(items.begin(), std::find(items.begin(), items.end(), item));
            CGUIMessage msg(GUI_MSG_LIST_REMOVE_ITEM, windowID, window->GetFocusedControlID(),
                            idx + 1, 0);
            window->OnMessage(msg);
            return true;
          }
        }
        break;

      case ACTION_QUEUE_ITEM:
        g_plexApplication.playQueueManager->QueueItem(item, true);
        return true;
        break;

      case ACTION_PLEX_PQ_ADDUPTONEXT:
        g_plexApplication.playQueueManager->QueueItem(item, false);
        return true;
        break;
    }
  }

  // other actions that dont need an itemp.
  switch (actionID)
  {
    case ACTION_PLEX_NOW_PLAYING:
      m_navHelper.navigateToNowPlaying();
      return true;
      break;
      
    case ACTION_PLEX_PLAY_ALL:
      PlayAll(container, false);
      return true;
      break;
      
    case ACTION_PLEX_SHUFFLE_ALL:
      PlayAll(container, true);
      return true;
      break;
      
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CGUIPlexDefaultActionHandler::GetContextButtonsForAction(int actionID, CFileItemPtr item,
                                                              CFileItemListPtr container, CContextButtons& buttons)
{
  switch (actionID)
  {
    case ACTION_PLAYER_PLAY:
      buttons.Add(actionID, 208);
      break;
      
    case ACTION_PLEX_PLAY_TRAILER:
      if (item->GetPlexDirectoryType() == PLEX_DIR_TYPE_MOVIE)
      {
        CPlexExtraDataLoader loader;
        if (loader.getDataForItem(item) && loader.getItems()->Size())
          buttons.Add(actionID, 44550);
      }
      break;
      
    case ACTION_PLEX_NOW_PLAYING:
      if (g_application.IsPlaying())
        buttons.Add(actionID, 13350);
      break;
      
    case ACTION_PLEX_SHUFFLE_ALL:
      if (container->Size())
        buttons.Add(actionID, 52600);
      break;

    case ACTION_MARK_AS_WATCHED:
    {
      if (item->IsVideo() && item->IsPlexMediaServerLibrary())
      {
        CStdString viewOffset = item->GetProperty("viewOffset").asString();
        
        if (item->GetVideoInfoTag()->m_playCount == 0 || viewOffset.size() > 0)
          buttons.Add(actionID, 16103);
        break;
      }
    }
      
    case ACTION_MARK_AS_UNWATCHED:
    {
      if (item->IsVideo() && item->IsPlexMediaServerLibrary())
      {
        CStdString viewOffset = item->GetProperty("viewOffset").asString();
        
        if (item->GetVideoInfoTag()->m_playCount > 0 || viewOffset.size() > 0)
          buttons.Add(actionID, 16104);
        break;
      }
    }

    case ACTION_PLEX_PQ_CLEAR:
      if (IsPlayQueueContainer(container))
        buttons.Add(actionID, 192);
      break;

    case ACTION_DELETE_ITEM:
      if (IsPlayQueueContainer(container))
      {
        buttons.Add(actionID, 1210);
      }
      else
      {
        EPlexDirectoryType dirType = item->GetPlexDirectoryType();

        if (item->IsPlexMediaServerLibrary() &&
            (item->IsRemoteSharedPlexMediaServerLibrary() == false) &&
            (dirType == PLEX_DIR_TYPE_EPISODE || dirType == PLEX_DIR_TYPE_MOVIE ||
             dirType == PLEX_DIR_TYPE_VIDEO || dirType == PLEX_DIR_TYPE_TRACK))
        {
          CPlexServerPtr server =
          g_plexApplication.serverManager->FindByUUID(item->GetProperty("plexserver").asString());
          if (server && server->SupportsDeletion())
            buttons.Add(actionID, 117);
        }
      }
      break;

    case ACTION_QUEUE_ITEM:
    {
      CFileItemList pqlist;
      g_plexApplication.playQueueManager->getCurrentPlayQueue(pqlist);

      if (pqlist.Size())
        buttons.Add(actionID, 52602);
      else
        buttons.Add(actionID, 52607);
      break;
    }

    case ACTION_PLEX_PQ_ADDUPTONEXT:
    {
      ePlexMediaType itemType = PlexUtils::GetMediaTypeFromItem(item);
      if (g_plexApplication.playQueueManager->getCurrentPlayQueueType() == itemType)
        buttons.Add(actionID, 52603);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::PlayMedia(CFileItemPtr item, CFileItemListPtr container)
{
  if (!item)
    return false;

  if (IsPhotoContainer(container))
  {
    if (item->m_bIsFolder)
      CApplicationMessenger::Get().PictureSlideShow(item->GetPath(), false);
    else
      CApplicationMessenger::Get().PictureSlideShow(container->GetPath(), false, item->GetPath());
  }
  else if (IsMusicContainer(container) && !item->m_bIsFolder)
  {
    PlayAll(container, false, item);
  }
  else
  {
    std::string uri = GetFilteredURI(*item);

    g_plexApplication.playQueueManager->create(*item, uri);
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CGUIPlexDefaultActionHandler::PlayAll(CFileItemListPtr container, bool shuffle,
                                           const CFileItemPtr& fromHere)
{
  if (IsPhotoContainer(container))
  {
    // Photos are handled a bit different
    CApplicationMessenger::Get().PictureSlideShow(container->GetPath(), false,
                                                  fromHere ? fromHere->GetPath() : "", shuffle);
    return;
  }

  CPlexServerPtr server;
  if (container->HasProperty("plexServer"))
    server =
    g_plexApplication.serverManager->FindByUUID(container->GetProperty("plexServer").asString());

  CStdString fromHereKey;
  if (fromHere)
    fromHereKey = fromHere->GetProperty("key").asString();

  // take out the plexserver://plex part from above when passing it down
  CStdString uri = GetFilteredURI(*container);

  CPlexPlayQueueOptions options;
  options.startItemKey = fromHereKey;
  options.startPlaying = true;
  options.shuffle = shuffle;
  options.showPrompts = true;

  g_plexApplication.playQueueManager->create(*container, uri, options);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::string CGUIPlexDefaultActionHandler::GetFilteredURI(const CFileItem& item) const
{
  CURL itemUrl(item.GetPath());
  
  itemUrl.SetProtocol("plexserver");
  itemUrl.SetHostName("plex");
  
  if (itemUrl.HasOption("unwatchedLeaves"))
  {
    itemUrl.SetOption("unwatched", itemUrl.GetOption("unwatchedLeaves"));
    itemUrl.RemoveOption("unwatchedLeaves");
  }

  if (item.GetPlexDirectoryType() == PLEX_DIR_TYPE_SHOW ||
      (item.GetPlexDirectoryType() == PLEX_DIR_TYPE_SEASON && item.HasProperty("size")))
  {
    std::string fname = itemUrl.GetFileName();
    boost::replace_last(fname, "/children", "/allLeaves");
    itemUrl.SetFileName(fname);
  }

  // set sourceType
  if (item.m_bIsFolder)
  {
    CStdString sourceType = boost::lexical_cast<CStdString>(PlexUtils::GetFilterType(item));
    itemUrl.SetOption("sourceType", sourceType);
  }

  return CPlexPlayQueueManager::getURIFromItem(item,itemUrl.Get().substr(17, std::string::npos));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::IsPhotoContainer(CFileItemListPtr container) const
{
  if (!container)
    return false;
  
  EPlexDirectoryType dirType = container->GetPlexDirectoryType();

  if (dirType == PLEX_DIR_TYPE_CHANNEL && container->Get(0))
    dirType = container->Get(0)->GetPlexDirectoryType();

  return (dirType == PLEX_DIR_TYPE_PHOTOALBUM | dirType == PLEX_DIR_TYPE_PHOTO);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::IsMusicContainer(CFileItemListPtr container) const
{
  if (!container)
    return false;

  EPlexDirectoryType dirType = container->GetPlexDirectoryType();
  if (dirType == PLEX_DIR_TYPE_CHANNEL && container->Get(0))
    dirType = container->Get(0)->GetPlexDirectoryType();
  return (dirType == PLEX_DIR_TYPE_ALBUM || dirType == PLEX_DIR_TYPE_ARTIST ||
          dirType == PLEX_DIR_TYPE_TRACK);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::IsVideoContainer(CFileItemListPtr container) const
{
  if (!container)
    return false;
  
  EPlexDirectoryType dirType = container->GetPlexDirectoryType();
  
  if (dirType == PLEX_DIR_TYPE_CHANNEL && container->Get(0))
    dirType = container->Get(0)->GetPlexDirectoryType();
  
  return (dirType == PLEX_DIR_TYPE_MOVIE    ||
          dirType == PLEX_DIR_TYPE_SHOW     ||
          dirType == PLEX_DIR_TYPE_SEASON   ||
          dirType == PLEX_DIR_TYPE_PLAYLIST ||
          dirType == PLEX_DIR_TYPE_EPISODE  ||
          dirType == PLEX_DIR_TYPE_VIDEO    ||
          dirType == PLEX_DIR_TYPE_CLIP);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::IsPlayListContainer(CFileItemListPtr container) const
{
  if (!container)
    return false;
  
  CURL u(container->GetPath());
  
  if (boost::algorithm::starts_with(u.GetFileName(),"playlists"))
    return true;
  
  return false;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIPlexDefaultActionHandler::IsPlayQueueContainer(CFileItemListPtr container) const
{
  if (!container)
    return false;
  
  CURL u(container->GetPath());
  
  if (u.GetHostName() == "playqueue")
    return true;
  
  return false;
};

