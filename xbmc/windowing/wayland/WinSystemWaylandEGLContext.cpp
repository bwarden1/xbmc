/*
 *      Copyright (C) 2017 Team XBMC
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

#include "WinSystemWaylandEGLContext.h"

#include "Connection.h"
#include "cores/VideoPlayer/DVDCodecs/DVDFactoryCodec.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "utils/log.h"
#include "windowing/GraphicContext.h"

#include <EGL/eglext.h>

using namespace KODI::WINDOWING::WAYLAND;

CWinSystemWaylandEGLContext::CWinSystemWaylandEGLContext()
: m_eglContext{EGL_PLATFORM_WAYLAND_EXT, "EGL_EXT_platform_wayland"}
{}

bool CWinSystemWaylandEGLContext::InitWindowSystemEGL(EGLint renderableType, EGLint apiType)
{
  VIDEOPLAYER::CRendererFactory::ClearRenderer();
  CDVDFactoryCodec::ClearHWAccels();

  if (!CWinSystemWayland::InitWindowSystem())
  {
    return false;
  }

  if (!m_eglContext.CreatePlatformDisplay(GetConnection()->GetDisplay(), GetConnection()->GetDisplay(), renderableType, apiType))
  {
    return false;
  }

  return true;
}

bool CWinSystemWaylandEGLContext::CreateNewWindow(const std::string& name,
                                                  bool fullScreen,
                                                  RESOLUTION_INFO& res)
{
  if (!CWinSystemWayland::CreateNewWindow(name, fullScreen, res))
  {
    return false;
  }

  if (!CreateContext())
  {
    return false;
  }

  m_nativeWindow = wayland::egl_window_t{GetMainSurface(), GetBufferSize().Width(), GetBufferSize().Height()};

  // CWinSystemWayland::CreateNewWindow sets internal m_bufferSize
  // to the resolution that should be used for the initial surface size
  // - the compositor might want something other than the resolution given
  if (!m_eglContext.CreatePlatformSurface(m_nativeWindow.c_ptr(), m_nativeWindow.c_ptr()))
  {
    return false;
  }

  if (!m_eglContext.BindContext())
  {
    return false;
  }

  // Never enable the vsync of the EGL implementation, we handle that ourselves
  // in WinSystemWayland
  m_eglContext.SetVSync(false);

  return true;
}

bool CWinSystemWaylandEGLContext::DestroyWindow()
{
  m_eglContext.DestroySurface();
  m_nativeWindow = {};

  return CWinSystemWayland::DestroyWindow();
}

bool CWinSystemWaylandEGLContext::DestroyWindowSystem()
{
  m_eglContext.Destroy();

  return CWinSystemWayland::DestroyWindowSystem();
}

CSizeInt CWinSystemWaylandEGLContext::GetNativeWindowAttachedSize()
{
  int width, height;
  m_nativeWindow.get_attached_size(width, height);
  return {width, height};
}

void CWinSystemWaylandEGLContext::SetContextSize(CSizeInt size)
{
  // Change EGL surface size if necessary
  if (GetNativeWindowAttachedSize() != size)
  {
    CLog::LogF(LOGDEBUG, "Updating egl_window size to %dx%d", size.Width(), size.Height());
    m_nativeWindow.resize(size.Width(), size.Height(), 0, 0);
  }
}

void CWinSystemWaylandEGLContext::PresentFrame(bool rendered)
{
  PrepareFramePresentation();

  if (rendered)
  {
    m_eglContext.SwapBuffers();
    // eglSwapBuffers() (hopefully) calls commit on the surface and flushes
    // ... well mesa does anyway
  }
  else
  {
    // For presentation feedback: Get notification of the next vblank even
    // when contents did not change
    GetMainSurface().commit();
    // Make sure it reaches the compositor
    GetConnection()->GetDisplay().flush();
  }

  FinishFramePresentation();
}

EGLDisplay CWinSystemWaylandEGLContext::GetEGLDisplay() const
{
  return m_eglContext.GetEGLDisplay();
}
