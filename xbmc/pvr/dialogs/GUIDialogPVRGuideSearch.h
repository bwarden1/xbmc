/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <map>

#include "XBDateTime.h"
#include "guilib/GUIDialog.h"

#include "pvr/channels/PVRChannelNumber.h"

namespace PVR
{
  class CPVREpgSearchFilter;

  class CGUIDialogPVRGuideSearch : public CGUIDialog
  {
  public:
    CGUIDialogPVRGuideSearch(void);
    ~CGUIDialogPVRGuideSearch(void) override = default;
    bool OnMessage(CGUIMessage& message) override;
    void OnWindowLoaded() override;

    void SetFilterData(CPVREpgSearchFilter *searchFilter) { m_searchFilter = searchFilter; }
    bool IsConfirmed() const { return m_bConfirmed; }
    bool IsCanceled() const { return m_bCanceled; }

  protected:
    void OnInitWindow() override;

  private:
    void OnSearch();
    void UpdateChannelSpin(void);
    void UpdateGroupsSpin(void);
    void UpdateGenreSpin(void);
    void UpdateDurationSpin(void);
    CDateTime ReadDateTime(const std::string &strDate, const std::string &strTime) const;
    void Update();

    bool IsRadioSelected(int controlID);
    int  GetSpinValue(int controlID);
    std::string GetEditValue(int controlID);

    bool m_bConfirmed = false;
    bool m_bCanceled = false;
    CPVREpgSearchFilter *m_searchFilter;
    std::map<int, CPVRChannelNumber> m_channelNumbersMap;
  };
}
