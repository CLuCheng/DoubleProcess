/*
 * Copyright (c) 2018 Agora.io
 * All rights reserved.
 * Proprietary and Confidential -- Agora.io
 */

/*
 *  Created by Wang Yongli, 2018
 */
#include "video_source.h"
#include <sstream>
#include "node_log.h"
#include "video_source_event_handler.h"
#include "video_source_ipc.h"
#include "video_source_param_parser.h"
#include "video_source_render.h"

#define PROCESS_RUN_EVENT_NAME "agora_video_source_process_ready_event_name"
#define DATA_IPC_NAME "avsipc"

using agora::rtc::RtcEngineContext;
using agora::rtc::uid_t;

AgoraVideoSource::AgoraVideoSource(const std::string& param)
    : m_initialized(false),
      m_params(param),
      m_videoProfile(agora::rtc::VIDEO_PROFILE_DEFAULT) {
  LOG_ENTER;
  LOG_LEAVE;
}

AgoraVideoSource::~AgoraVideoSource() {
  LOG_ENTER;
  m_rtcEngine.reset();
  m_eventHandler.reset();
  m_renderFactory.reset();
  m_ipc.reset();
  m_paramParser.reset();
  LOG_LEAVE;
}

std::string AgoraVideoSource::getId() {
  return m_paramParser->getParameter("id");
}

bool AgoraVideoSource::initialize() {
  LOG_ENTER;
  m_paramParser.reset(new VideoSourceParamParser());
  m_paramParser->initialize(m_params);

  std::string appid = m_paramParser->getParameter("appid");
  if (appid.empty()) {
    LOG_ERROR("%s, appid is null\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }

  std::string id = m_paramParser->getParameter("id");
  if (id.empty()) {
    LOG_ERROR("%s, id is null\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }

  m_ipc.reset(createAgoraIpc(this));
  if (!m_ipc->initialize(id)) {
    LOG_ERROR("%s, ipc init fail\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }
  if (!m_ipc->connect()) {
    LOG_ERROR("%s, ipc connect fail.\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }

  m_rtcEngine.reset(createAgoraRtcEngine());
  if (!m_rtcEngine.get()) {
    LOG_ERROR("%s, rtcengine create fail.\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }
  m_eventHandler.reset(new AgoraVideoSourceEventHandler(*this));
  m_renderFactory.reset(new AgoraVideoSourceRenderFactory(*this));
  RtcEngineContext context;
  context.eventHandler = m_eventHandler.get();
  context.appId = appid.c_str();
  context.areaCode = agora::rtc::AREA_CODE_GLOB;
  LOG_ERROR("initialized: %s, appid\n", __FUNCTION__);
  if (m_rtcEngine->initialize(context) != 0) {
    LOG_ERROR("%s, AgoraVideoSource initialize failed.\n", __FUNCTION__);
    LOG_LEAVE;
    return false;
  }
  std::stringstream ss;
  ss << "\n"
     << "initialize:appid:" << appid.c_str() << "\n";
  OutputDebugStringA(ss.str().c_str());

  std::string currentPath;
  INodeProcess::getCurrentModuleFileName(currentPath);
  std::string logFile =
      currentPath + "SDK" + std::to_string(GetCurrentProcessId()) + ".txt";
  m_rtcEngine->setLogFile(logFile.c_str());
  /* agora::util::AutoPtr<agora::media::IMediaEngine> pMediaEngine;
    pMediaEngine.queryInterface(m_rtcEngine.get(),
    agora::AGORA_IID_MEDIA_ENGINE);

    if (pMediaEngine.get()) {
      pMediaEngine->registerVideoRenderFactory(m_renderFactory.get());
    } else {
      LOG_ERROR("%s, Get media engine failed.\n", __FUNCTION__);
      LOG_LEAVE;
      return false;
    }*/
  m_videoDeviceManager = new agora::rtc::AVideoDeviceManager(m_rtcEngine.get());
  // m_rtcEngine->disableAudio();
  // m_rtcEngine->enableVideo();
  ////  m_rtcEngine->enableLocalVideo(false);
  // m_rtcEngine->muteAllRemoteVideoStreams(true);
  // m_rtcEngine->muteAllRemoteAudioStreams(true);
  m_ipc->sendMessage(AGORA_IPC_SOURCE_READY, nullptr, 0);
  m_initialized = true;
  LOG_LEAVE;
  return true;
}

node_error AgoraVideoSource::startPreview(const char* cameraId) {
  LOG_ENTER;
  int status = 0;
  if (!m_ipcSender) {
    std::string id = m_paramParser->getParameter("id");
    m_ipcSender.reset(new AgoraIpcDataSender());
    if (!m_ipcSender->initialize(id + DATA_IPC_NAME)) {
      LOG_ERROR("%s, ipc sender init fail.", __FUNCTION__);
      status = -1;
      return node_generic_error;
    }
  }

  if (cameraId == nullptr || *cameraId == '\0') {
  } else {
    (*m_videoDeviceManager)->setDevice(cameraId);
  }
  agora::rtc::BeautyOptions options;
  options.lighteningContrastLevel =
      agora::rtc::BeautyOptions::LIGHTENING_CONTRAST_HIGH;
  options.lighteningLevel = 0.5;
  options.rednessLevel = 1.0;
  options.smoothnessLevel = 0.5;
  m_rtcEngine->setBeautyEffectOptions(true, options);
  m_rtcEngine->startPreview();
  m_ipc->sendMessage(AGORA_IPC_START_VS_PREVIEW_COMPLETE, (char*)&status,
                     sizeof(status));
  return status == 0 ? node_ok : node_generic_error;
}

node_error AgoraVideoSource::stopPreview() {
  LOG_ENTER;
  agora::rtc::VideoCanvas canvas;
  m_rtcEngine->setupLocalVideo(canvas);
  m_rtcEngine->stopPreview();
  {
    std::lock_guard<std::mutex> lock(m_ipcSenderMutex);
    // m_ipcSender.reset();
  }
  m_rtcEngine->setParameters("");
  int status =
      m_ipc->sendMessage(AGORA_IPC_STOP_VS_PREVIEW_COMPLETE, nullptr, 0)
          ? node_ok
          : node_generic_error;

  LOG_LEAVE;
  return status == 0 ? node_ok : node_generic_error;
}

void AgoraVideoSource::notifyJoinedChannel(uid_t uid) {
  m_ipc->sendMessage(AGORA_IPC_JOIN_SUCCESS, (char*)&uid, sizeof(uid));
}

void AgoraVideoSource::notifyRenderReady() {
  m_ipc->sendMessage(AGORA_IPC_RENDER_READY, nullptr, 0);
}

void AgoraVideoSource::notifyLeaveChannel() {
  m_ipc->sendMessage(AGORA_IPC_LEAVE_CHANNEL, nullptr, 0);
}

void AgoraVideoSource::notifyRequestNewToken() {
  m_ipc->sendMessage(AGORA_IPC_RENEW_TOKEN, nullptr, 0);
}

void AgoraVideoSource::notifyFirstLocalVideoFrame(int width, int height,
                                                  int elapsed) {
  std::unique_ptr<FirtLocalVideoFrame> firstLocalVideo(new FirtLocalVideoFrame);
  firstLocalVideo->width = width;
  firstLocalVideo->height = height;
  firstLocalVideo->elapsed = elapsed;
  m_ipc->sendMessage(AGORA_IPC_FIRST_LOCAL_VIDEO, (char*)&firstLocalVideo,
                     sizeof(FirtLocalVideoFrame));
}

int AgoraVideoSource::notifySetCustomRender(bool enable) {
  agora::util::AutoPtr<agora::media::IMediaEngine> pMediaEngine;
  pMediaEngine.queryInterface(m_rtcEngine.get(), agora::AGORA_IID_MEDIA_ENGINE);
  if (enable) {
    if (pMediaEngine.get()) {
      pMediaEngine->registerVideoRenderFactory(m_renderFactory.get());
    } else {
      LOG_ERROR("%s, Get media engine failed.\n", __FUNCTION__);
      LOG_LEAVE;
      return -1;
    }
  } else {
    pMediaEngine->registerVideoRenderFactory(NULL);
  }
  return 0;
}

void AgoraVideoSource::notifyDisconnect() {
  m_ipc->sendMessage(AGORA_IPC_DISCONNECT, NULL, 0);
}

void AgoraVideoSource::release() { delete this; }

void AgoraVideoSource::onMessage(unsigned int msg, char* payload,
                                 unsigned int len) {
  LOG_ENTER;
  if (!m_initialized) {
    LOG_ERROR("%s, no init.\n", __FUNCTION__);
    LOG_LEAVE;
    return;
  }
  LOG_INFO("%s, msg : %d\n", __FUNCTION__, msg);
  if (msg == AGORA_IPC_JOIN) {
    if (payload) {
      JoinChannelCmd* cmd = (JoinChannelCmd*)payload;
      LOG_INFO("%s, AGORA_IPC_JOIN.\n", __FUNCTION__);
      joinChannel(cmd->token, cmd->cname, cmd->chan_info, cmd->uid);
      std::stringstream ss;
      ss << "\n"
         << "join channel:appid:" << cmd->token << "channel name:" << cmd->cname
         << "\n";
      OutputDebugStringA(ss.str().c_str());
    }
  } else if (msg == AGORA_IPC_CAPTURE_SCREEN) {
    if (len != sizeof(CaptureScreenCmd)) {
      LOG_ERROR("%s, Size not equal with capture screen cmd.\n", __FUNCTION__);
      LOG_LEAVE;
      return;
    }

    // m_rtcEngine->enableLocalVideo(true);
    CaptureScreenCmd* cmd = (CaptureScreenCmd*)payload;
    LOG_INFO("Start screen share, top : %d, left : %d, bottom :%d, right :%d\n",
             cmd->rect.top, cmd->rect.left, cmd->rect.bottom, cmd->rect.right);

    //	if (m_rtcEngine->startScreenCapture(cmd->windowid, cmd->captureFreq,
    //&cmd->rect, cmd->bitrate) != 0) {
    if (m_rtcEngine->startScreenCapture(cmd->windowid, cmd->captureFreq,
                                        &cmd->rect, cmd->bitrate) != 0) {
      LOG_ERROR("start screen capture failed.");
      // m_rtcEngine->enableLocalVideo(false);
    }
  } else if (msg == AGORA_IPC_STOP_CAPTURE_SCREEN) {
    m_rtcEngine->stopScreenCapture();
    // m_rtcEngine->enableLocalVideo(false);
  } else if (msg == AGORA_IPC_START_VS_PREVIEW) {
    StartPreviewCmd* cmd = (StartPreviewCmd*)payload;
    this->startPreview(cmd->cameraId);
    std::stringstream ss;
    ss << "\n"
       << "startPreview:appid:" << cmd->cameraId << "\n";
    OutputDebugStringA(ss.str().c_str());

  } else if (msg == AGORA_IPC_SET_CUSTOM_RENDER) {
    CustomRenderCmd* cmd = (CustomRenderCmd*)payload;
    this->notifySetCustomRender(cmd->enable);
    std::stringstream ss;
    ss << "\n"
       << "custom render:enable:" << cmd->enable << "\n";
    OutputDebugStringA(ss.str().c_str());
  } else if (msg == AGORA_IPC_SETUP_LOCAL_VIDEO) {
    LocalVideoCmd* cmd = (LocalVideoCmd*)payload;
    agora::rtc::VideoCanvas canvas;
    canvas.uid = cmd->uid;
    canvas.renderMode = cmd->renderMode;
    canvas.view = cmd->view;
    m_localView = cmd->view;
    m_rtcEngine->setupLocalVideo(canvas);
    std::stringstream ss;
    ss << "\n"
       << "setupLocalVideo:view:" << cmd->view << "\n";
    OutputDebugStringA(ss.str().c_str());
  } else if (msg == AGORA_IPC_STOP_VS_PREVIEW) {
    this->stopPreview();
    // m_rtcEngine->enableLocalVideo(false);
  } else if (msg == AGORA_IPC_RENEW_TOKEN) {
    m_rtcEngine->renewToken(payload);
  } else if (msg == AGORA_IPC_SET_CHANNEL_PROFILE) {
    if (payload) {
      ChannelProfileCmd* cmd = (ChannelProfileCmd*)payload;
      m_rtcEngine->setChannelProfile(cmd->profile);
      if (cmd->profile == agora::rtc::CHANNEL_PROFILE_LIVE_BROADCASTING) {
        m_rtcEngine->setClientRole(agora::rtc::CLIENT_ROLE_BROADCASTER);
      }
    }
  } else if (msg == AGORA_IPC_SET_VIDEO_RPOFILE) {
    if (len != sizeof(VideoProfileCmd)) {
      LOG_ERROR("%s, size not equal with video profile size.\n", __FUNCTION__);
      LOG_LEAVE;
      return;
    }
    VideoProfileCmd* cmd = (VideoProfileCmd*)payload;
    if (cmd->profile > agora::rtc::VIDEO_PROFILE_LANDSCAPE_4K_3) {
      LOG_ERROR("%s, set video profile with invalid value : %d", __FUNCTION__,
                cmd->profile);
    } else {
      this->m_videoProfile = (agora::rtc::VIDEO_PROFILE_TYPE)cmd->profile;
      m_rtcEngine->setVideoProfile(cmd->profile, cmd->swapWidthAndHeight);
    }
  } else if (msg == AGORA_IPC_SET_VIDEO_ENCODER_CONFIGURATION) {
    if (len != sizeof(VideoEncoderConfigurationCmd)) {
      LOG_ERROR("%s, size not equal with video encoder configuration size.\n",
                __FUNCTION__);
      LOG_LEAVE;
      return;
    }

    VideoEncoderConfigurationCmd* cmd = (VideoEncoderConfigurationCmd*)payload;
    agora::rtc::VideoEncoderConfiguration config(cmd->width, cmd->height,
                                                 cmd->frameRate, cmd->bitrate,
                                                 cmd->orientationMode);
    config.minBitrate = cmd->minBitrate;
    m_rtcEngine->setVideoEncoderConfiguration(config);
  } else if (msg == AGORA_IPC_LEAVE_CHANNEL) {
    m_rtcEngine->leaveChannel();
  } else if (msg == AGORA_IPC_DISCONNECT) {
    this->exit(false);
  } else if (msg == AGORA_IPC_ENABLE_WEB_SDK_INTEROPERABILITY) {
    m_rtcEngine->enableWebSdkInteroperability((bool)*payload);
  } else if (msg == AGORA_IPC_ENABLE_DUAL_STREAM_MODE) {
    m_rtcEngine->enableDualStreamMode((bool)*payload);
  } else if (msg == AGORA_IPC_SET_LOGFILE) {
    m_rtcEngine->setLogFile((char*)payload);
  } else if (msg == AGORA_IPC_SET_PARAMETER) {
    if (len != sizeof(SetParameterCmd)) return;
    SetParameterCmd* cmd = (SetParameterCmd*)payload;
    agora::rtc::AParameter rep(m_rtcEngine.get());
    rep->setParameters(cmd->parameters);
  } else if (msg == AGORA_UPC_SETUP_LOCAL_RENDER_MODE) {
    LocalRenderModeCmd* cmd = (LocalRenderModeCmd*)payload;
    agora::rtc::RENDER_MODE_TYPE mode =
        (agora::rtc::RENDER_MODE_TYPE)cmd->renderMode;
    m_rtcEngine->setLocalRenderMode(mode);
  } else if (msg == AGORA_IPC_ENABLE_VIDEO) {
    EnableVideoCmd* cmd = (EnableVideoCmd*)payload;
    if (cmd->enable)
      m_rtcEngine->enableVideo();
    else
      m_rtcEngine->disableVideo();
    std::stringstream ss;
    ss << "\n"
          "enablevideo:"
       << cmd->enable << "\n";
    OutputDebugStringA(ss.str().c_str());
  } else if (msg == AGORA_IPC_ENABLE_AUDIO) {
    EnableAudioCmd* cmd = (EnableAudioCmd*)payload;
    if (cmd->enable)
      m_rtcEngine->enableAudio();
    else
      m_rtcEngine->disableAudio();
    std::stringstream ss;
    ss << "\n"
       << "enableAudio:" << cmd->enable << "\n";
    OutputDebugStringA(ss.str().c_str());
  } else if (msg == AGORA_IPC_ENABLE_LOCAL_VIDEO) {
    EnableLocalVideoCmd* cmd = (EnableLocalVideoCmd*)payload;
    m_rtcEngine->enableLocalVideo(cmd->enable);
  } else if (msg == AGORA_IPC_ENABLE_LOCAL_AUDIO) {
    EnableLocalAudioCmd* cmd = (EnableLocalAudioCmd*)payload;
    m_rtcEngine->enableLocalAudio(cmd->enable);
  } else if (msg == AGORA_IPC_MUTE_REMOTE_AUDIO_STREAM) {
    RemoteAudioStreamCmd* cmd = (RemoteAudioStreamCmd*)payload;
    m_rtcEngine->muteRemoteAudioStream(cmd->uid, cmd->mute);
    m_mapMuteRemoteAudioStreams[cmd->uid] = cmd->mute;
  } else if (msg == AGORA_IPC_MUTE_REMOTE_VIDEO_STREAM) {
    RemoteVideoStreamCmd* cmd = (RemoteVideoStreamCmd*)payload;
    m_rtcEngine->muteRemoteVideoStream(cmd->uid, cmd->mute);
  } else if (msg == AGORA_IPC_SET_REMOTE_VIDEO_STREAM_TYPE) {
    RemoteVideoStreamTypeCmd* cmd = (RemoteVideoStreamTypeCmd*)payload;
    m_rtcEngine->setRemoteVideoStreamType(cmd->uid, cmd->streamType);
  } else if (msg == AGORA_IPC_MUTE_LOCAL_AUDIO_STREAM) {
    LocalAudioStreamCmd* cmd = (LocalAudioStreamCmd*)payload;
    m_rtcEngine->muteLocalAudioStream(cmd->mute);
  } else if (msg == AGORA_IPC_MUTE_LOCAL_VIDEO_STREAM) {
    LocalVideoStreamCmd* cmd = (LocalVideoStreamCmd*)payload;
    m_rtcEngine->muteLocalVideoStream(cmd->mute);
  } else if (msg == AGORA_IPC_SETUP_REMOTE_VIDEO) {
    RemoteVideoCmd* cmd = (RemoteVideoCmd*)payload;
    agora::rtc::VideoCanvas cv;
    cv.uid = cmd->uid;
    cv.renderMode = cmd->renderMode;
    cv.view = cmd->view;
    cv.priv = 0;
    m_rtcEngine->setupRemoteVideo(cv);
  } else if (msg == AGORA_IPC_MUTE_ALL_REMOTE_AUDIO_STREAM) {
    MuteAllRemoteAudioStreamCmd* cmd = (MuteAllRemoteAudioStreamCmd*)payload;
    m_rtcEngine->muteAllRemoteAudioStreams(cmd->enabled);
    mute_all_remote_audio = cmd->enabled;
  }
  else if (msg == AGORA_IPC_SET_DEFAULT_REMOTE_MUTE_ALL_AUDIO_STREAMS){
    SetDefaultMuteRemoteAudioStreamCmd* cmd = (SetDefaultMuteRemoteAudioStreamCmd*)payload;
    m_rtcEngine->setDefaultMuteAllRemoteAudioStreams(cmd->enabled);
    mute_remote_audio = cmd->enabled;
  }
  else if (msg == AGORA_IPC_ENABLED_HARDWARE_ENCODING){
    EnabledHWVideoEncodingCmd* cmd = (EnabledHWVideoEncodingCmd*)payload;
    agora::rtc::AParameter apm(*m_rtcEngine);
    int ret = apm->setInt("che.hardware_encoding", cmd->enabled);
  }
  LOG_LEAVE;
}
bool AgoraVideoSource::joinChannel(const char* key, const char* name,
                                   const char* chanInfo,
                                   agora::rtc::uid_t uid) {
  return m_rtcEngine->joinChannel(key ? key : "", name ? name : "",
                                  chanInfo ? chanInfo : "", uid);
}

void AgoraVideoSource::exit(bool notifySink) {
  notifyDisconnect();
  m_rtcEngine->stopPreview();
  m_rtcEngine->disableVideo();
  m_rtcEngine->disableAudio();
  m_rtcEngine->enableLocalVideo(false);
  m_rtcEngine->enableLocalAudio(false);
  m_rtcEngine->muteAllRemoteVideoStreams(true);
  notifySetCustomRender(false);

  TerminateProcess(GetCurrentProcess(), 1);
  // why release locked?
  auto func = [this] { m_rtcEngine->release(true); };
  func();
  std::thread(func).detach();
  {
    std::lock_guard<std::mutex> lock(m_ipcSenderMutex);
    m_ipcSender.reset();
  }
  m_ipc->disconnect();
}

void AgoraVideoSource::run() {
  LOG_ENTER;
#ifdef _WIN32
  std::string idstr = m_paramParser->getParameter("pid");
#else
  std::string idstr = m_paramParser->getParameter("fd");
#endif
  if (idstr.empty()) {
    LOG_ERROR("%s, pid is null\n", __FUNCTION__);
    LOG_LEAVE;
    return;
  }
  m_process.reset(INodeProcess::OpenNodeProcess(std::atoi(idstr.c_str())));
  if (!m_process.get()) {
    LOG_ERROR("Process open fail.\n");
    LOG_LEAVE;
    return;
  }
  m_process->Monitor([this](INodeProcess*) {
    LOG_WARNING("%s, remote process ext.\n", __FUNCTION__);
    this->exit(false);
  });
  m_ipc->run();
  LOG_LEAVE;
}

bool AgoraVideoSource::sendData(char* payload, int len) {
  if (!payload || len == 0) return false;

  std::lock_guard<std::mutex> lock(m_ipcSenderMutex);

  if (m_ipcSender) {
    m_ipcSender->sendData(payload, len);
    LOG_INFO("Send data success\n");
    return true;
  } else {
    LOG_WARNING("IPC Sender not initialized before send data.");
    return false;
  }
}

bool AgoraVideoSource::isLocalview(void* view) { return m_localView == view; }

agora::rtc::VIDEO_PROFILE_TYPE AgoraVideoSource::getVideoProfile() {
  return m_videoProfile;
}

void initLogService() {
  std::string currentPath;
  INodeProcess::getCurrentModuleFileName(currentPath);
  std::string logFile =
      currentPath + std::to_string(GetCurrentProcessId()) + ".txt";
  startLogService(logFile.c_str());
}

int main(int argc, char* argv[]) {
  // OSVERSIONINFO info;
  // memset(&info, 0, sizeof(OSVERSIONINFO));
  // GetVersionEx(&info);
  // info.dwMajorVersion
  // SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
  SetProcessDPIAware();
  initLogService();
  if (argc < 3) {
    LOG_ERROR("Need at least 3 parameter. Current parameter num : %d\n", argc);
    return 0;
  }
  std::string param;
  for (int i = 1; i < argc; i++) {
    param.append(argv[i]);
    param.append(" ");
  }
  LOG_INFO("Args : %s\n", param.c_str());
  auto videoSource = new AgoraVideoSource(param);
  videoSource->initialize();
  videoSource->run();
  videoSource->release();
  LOG_INFO("end ");
  stopLogService();
  return 0;
}
