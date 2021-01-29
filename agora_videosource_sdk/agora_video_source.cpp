/*
 * Copyright (c) 2018 Agora.io
 * All rights reserved.
 * Proprietary and Confidential -- Agora.io
 */

/*
 *  Created by Wang Yongli, 2018
 */

#include "agora_video_source.h"
#include "node_log.h"
//#include "node_async_queue.h"
//#include "node_video_stream_channel.h"
#include "node_event.h"
#include "node_process.h"
#include "video_source_ipc.h"
//#include "node_video_render.h"
//#include "node_napi_api.h"
#include <unordered_map>
#include "agora_video_frame.h"
#ifdef _WIN32
#include <Rpc.h>
#include <RpcDce.h>
#else
#include <uuid/uuid.h>
#endif
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define DATA_IPC_NAME "avsipc"
#define PROCESS_RUN_EVENT_NAME "agora_video_source_process_ready_event_name"
namespace agora {
namespace rtc {
/**
 * AgoraVideoSource implementation
 * agoraVideoSource starts two thread, one for message transform, and one for
 * video data transform.
 */

class AgoraVideoSourceSink : public AgoraVideoSource, public AgoraIpcListener {
 public:
  AgoraVideoSourceSink();
  ~AgoraVideoSourceSink();

  virtual bool initialize(IAgoraVideoSourceEventHandler* eventHandler,
                          const char* appid) override;
  virtual node_error join(const char* token, const char* cname,
                          const char* chan_info, uid_t uid) override;
  virtual node_error leave() override;
  virtual node_error release() override;
  virtual node_error renewVideoSourceToken(const char* token) override;
  virtual node_error setVideoSourceChannelProfile(
      agora::rtc::CHANNEL_PROFILE_TYPE profile) override;
  virtual node_error setVideoSourceVideoProfile(
      agora::rtc::VIDEO_PROFILE_TYPE profile, bool swapWidthAndHeight) override;
  virtual node_error setVideoSourceVideoEncoderConfiguration(
      VideoEncoderConfiguration config) override;
  virtual void onMessage(unsigned int msg, char* payload,
                         unsigned int len) override;
  virtual node_error captureScreen(agora::rtc::WindowIDType id, int captureFreq,
                                   agora::rtc::Rect* rect,
                                   int bitrate) override;
  virtual node_error updateScreenCapture(agora::rtc::Rect* rect) override;
  virtual node_error stopCaptureScreen() override;
  virtual node_error startPreview(const char* cameraId) override;
  virtual node_error setupLocalVideo(HWND hWnd, uid_t uid,
                                     agora::rtc::RENDER_MODE_TYPE renderMode,
                                     delegate_render render) override;
  virtual node_error stopPreview() override;
  virtual node_error enableWebSdkInteroperability(bool enabled) override;
  virtual node_error enableDualStreamMode(bool enabled) override;
  virtual node_error setLogFile(const char* file) override;
  virtual void setParameters(const char* parameters) override;
  virtual int setCustomRender(bool enable) override;
  virtual int setLocalRenderMode(RENDER_MODE_TYPE mode) override;
  virtual int enableVideo(bool enabled) override;
  virtual int enableAudio(bool enabled) override;
  virtual int enableLocalVideo(bool enabled) override;
  virtual int enableLocalAudio(bool enabled) override;
  virtual int muteRemoteAudioStream(uid_t uid, bool mute) override;
  virtual int muteRemoteVideoStream(uid_t userId, bool mute) override;
  virtual int setRemoteVideoStreamType(
      uid_t userId, REMOTE_VIDEO_STREAM_TYPE streamType) override;
  virtual int muteLocalAudioStream(bool mute) override;
  virtual int muteLocalVideoStream(bool mute) override;
  virtual int muteAllRemoteAudioStreams(bool mute) override;
  virtual int setDefaultMuteAllRemoteAudioStreams(bool mute) override;
  virtual int setupRemoteVideo(VideoCanvas canvas,
                               delegate_render render) override;
  virtual int enableHardWareVideoEncoding(bool enabled) override;
 private:
  void msgThread();
  void deliverFrame(const char* payload, int len);
  void clear();

  void onStartPreviewComplete();

 private:
  std::unordered_map<void*, delegate_render> m_mapViewRender;
  void* m_localView = nullptr;

  std::thread m_msgThread;
  std::mutex m_lock;
  std::unique_ptr<IAgoraIpc> m_ipcMsg;
  std::unique_ptr<AgoraIpcDataReceiver> m_ipcReceiver;
  std::unique_ptr<INodeProcess> m_sourceNodeProcess;
  // std::unique_ptr<D3DRender> m_d3dRender;
  // std::unique_ptr<NodeVideoRender> m_videoRender;
  std::string m_peerId;
  bool m_peerJoined;
  bool m_initialized;
  bool m_customRender;
  IAgoraVideoSourceEventHandler* m_eventHandler;
  uid_t m_peerUid;
  NodeEvent m_event;
  // buffer_list m_buffers;
  std::vector<char> m_backBuf;

  parser_video_frame m_parser;

 private:
  static const unsigned int s_bufLen;
};
const unsigned int AgoraVideoSourceSink::s_bufLen =
    (const unsigned int)(2560 * 1600 * 1.5 + 1000);

AgoraVideoSource* createVideoSource() { return new AgoraVideoSourceSink(); }

AgoraVideoSourceSink::AgoraVideoSourceSink()
    : m_msgThread(), m_initialized(false), m_event(false) {
  m_backBuf.resize(AgoraVideoSourceSink::s_bufLen);
  m_ipcReceiver.reset();
}

AgoraVideoSourceSink::~AgoraVideoSourceSink() { clear(); }

void AgoraVideoSourceSink::clear() {
  m_initialized = false;
  m_customRender = false;
  m_eventHandler = nullptr;
  // m_d3dRender.reset();
  // m_videoRender.reset();
  // m_ipcReceiver.reset();
  if (m_ipcMsg.get()) {
    m_ipcMsg->sendMessage(AGORA_IPC_DISCONNECT, nullptr, 0);
  }
  //if (m_msgThread.joinable()) m_msgThread.join();
}

node_error AgoraVideoSourceSink::release() {
  if (m_initialized) {
    clear();
  }
  return node_ok;
}

bool AgoraVideoSourceSink::initialize(
    IAgoraVideoSourceEventHandler* eventHandler, const char* appid) {
  if (m_initialized) return true;
  if (!appid) return false;
  clear();
  m_eventHandler = eventHandler;
#ifdef _WIN32
  UUID uuid = {0};
  RPC_CSTR struuid;

  if (UuidCreate(&uuid) != RPC_S_OK) return false;
  if (UuidToStringA(&uuid, &struuid) != RPC_S_OK) return false;
  m_peerId = (LPSTR)struuid;
  RpcStringFreeA(&struuid);
#else
  uuid_t uuid;
  uuid_generate(uuid);
  uuid_string_t uid = {0};
  uuid_unparse(uuid, uid);
  m_peerId = "/";
  m_peerId += uid;
  m_peerId = m_peerId.substr(0, 20);
#endif
  do {
    m_ipcMsg.reset(createAgoraIpc(this));
    if (!m_ipcMsg.get()) {
      break;
    }

    if (!m_ipcMsg->initialize(m_peerId)) break;

    if (!m_ipcMsg->listen()) break;

    std::string targetPath;
    if (!INodeProcess::getCurrentModuleFileName(targetPath)) {
      break;
    }

    size_t pos = targetPath.find_last_of("\\/");
    if (pos == targetPath.npos) {
      break;
    }

    std::stringstream ss;
    ss << INodeProcess::GetCurrentNodeProcessId();
    std::string path = targetPath.substr(0, pos + 1);
    std::string cmdname = "VideoSource";
    std::string idparam = "id:" + m_peerId;
    std::string pidparam = "pid:" + ss.str();
    std::string appidparam = "appid:" + std::string(appid);
    const char* params[] = {cmdname.c_str(), idparam.c_str(), pidparam.c_str(),
                            appidparam.c_str(), nullptr};
    m_sourceNodeProcess.reset(
        INodeProcess::CreateNodeProcess(path.c_str(), params));
    if (!m_sourceNodeProcess.get()) {
      break;
    }
    m_msgThread = std::thread(&AgoraVideoSourceSink::msgThread, this);
    NodeEvent::WaitResult result = m_event.WaitFor(5000);
    if (result != NodeEvent::WAIT_RESULT_SET) {
      break;
    }

    m_sourceNodeProcess->Monitor([eventHandler](INodeProcess* pProcess) {
      eventHandler->onVideoSourceExit();
    });
    /*auto nrc = new NodeRenderContext(NODE_RENDER_TYPE_VIDEO_SOURCE);
    ExternalVideoRenerContext context;
    m_videoRender.reset(new NodeVideoRender(nrc, context));*/
    // m_d3dRender.reset(new D3DRender);
    m_initialized = true;
    return true;
  } while (false);
  clear();
  return false;
}

node_error AgoraVideoSourceSink::startPreview(const char* cameraId) {
  std::unique_ptr<StartPreviewCmd> cmd(new StartPreviewCmd());
  if (cameraId) strncpy_s(cmd->cameraId, cameraId, 512);

  return m_ipcMsg->sendMessage(AGORA_IPC_START_VS_PREVIEW, (char*)cmd.get(),
                               sizeof(StartPreviewCmd))
             ? node_ok
             : node_generic_error;
}

node_error AgoraVideoSourceSink::setupLocalVideo(
    HWND hWnd, uid_t uid, agora::rtc::RENDER_MODE_TYPE renderMode,
    delegate_render render) {
  //-----------------------------------------------------
  /*if (m_d3dRender)
          m_d3dRender->Init(hWnd,2560, 1440, true);*/
  //-----------------------------------------------------
  // std::unique_ptr<LocalVideoCmd> cmd(new LocalVideoCmd());
  LocalVideoCmd cmd;
  cmd.renderMode = agora::rtc::RENDER_MODE_FIT;
  cmd.view = hWnd;
  cmd.uid = uid;
  m_mapViewRender[hWnd] = render;
  m_localView = hWnd;
  return m_ipcMsg->sendMessage(AGORA_IPC_SETUP_LOCAL_VIDEO, (char*)&cmd,
                               sizeof(cmd))
             ? node_ok
             : node_generic_error;
}

void AgoraVideoSourceSink::onStartPreviewComplete() {
  if (!m_initialized) return;
  if (!m_ipcReceiver)
  {
    m_ipcReceiver.reset(new AgoraIpcDataReceiver());
    if (!m_ipcReceiver->initialize(
            m_peerId + DATA_IPC_NAME,
            std::bind(&AgoraVideoSourceSink::deliverFrame, this,
                      std::placeholders::_1, std::placeholders::_2))) {
      m_ipcReceiver.reset();
      return;
    }
    m_ipcReceiver->run(true);
    Sleep(200);
  }
}

node_error AgoraVideoSourceSink::stopPreview() {
  if (!m_ipcReceiver) return node_status_error;

  return m_ipcMsg->sendMessage(AGORA_IPC_STOP_VS_PREVIEW, nullptr, 0)
             ? node_ok
             : node_generic_error;
}

node_error AgoraVideoSourceSink::enableWebSdkInteroperability(bool enabled) {
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_WEB_SDK_INTEROPERABILITY,
                                 (char*)&enabled, sizeof(enabled))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::enableDualStreamMode(bool enabled) {
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_DUAL_STREAM_MODE,
                                 (char*)&enabled, sizeof(enabled))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::setLogFile(const char* file) {
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_SET_LOGFILE, (char*)file,
                                 strlen(file))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

void AgoraVideoSourceSink::setParameters(const char* parameters) {
  if (!m_initialized) return;
  SetParameterCmd cmd;
  strncpy_s(cmd.parameters, parameters, MAX_PARAMETER_LEN);
  m_ipcMsg->sendMessage(AGORA_IPC_SET_PARAMETER, (char*)&cmd, sizeof(cmd));
}

int AgoraVideoSourceSink::setCustomRender(bool enable) {
  if (!m_initialized) return -1;
  CustomRenderCmd cmd;
  if (enable) {
    m_customRender = true;
    cmd.enable = true;
  } else {
    m_customRender = false;
    cmd.enable = false;
  }
  return m_ipcMsg->sendMessage(AGORA_IPC_SET_CUSTOM_RENDER, (char*)&cmd,
                               sizeof(cmd))
             ? 0
             : -1;
}

int AgoraVideoSourceSink::setLocalRenderMode(RENDER_MODE_TYPE mode) {
  if (!m_initialized) return -1;
  LocalRenderModeCmd cmd;
  cmd.renderMode = mode;
  m_ipcMsg->sendMessage(AGORA_UPC_SETUP_LOCAL_RENDER_MODE, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}

int AgoraVideoSourceSink::enableVideo(bool enabled) {
  if (!m_initialized) return -1;
  EnableVideoCmd cmd;
  cmd.enable = enabled;
  m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_VIDEO, (char*)&cmd, sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::enableAudio(bool enabled) {
  if (!m_initialized) return -1;
  EnableAudioCmd cmd;
  cmd.enable = enabled;
  m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_AUDIO, (char*)&cmd, sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::enableLocalVideo(bool enabled) {
  if (!m_initialized) return -1;
  EnableLocalVideoCmd cmd;
  cmd.enable = enabled;
  m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_LOCAL_VIDEO, (char*)&cmd, sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::enableLocalAudio(bool enabled) {
  if (!m_initialized) return -1;
  EnableLocalAudioCmd cmd;
  cmd.enable = enabled;
  m_ipcMsg->sendMessage(AGORA_IPC_ENABLE_LOCAL_AUDIO, (char*)&cmd, sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::muteRemoteAudioStream(uid_t uid, bool mute) {
  if (!m_initialized) return -1;
  RemoteAudioStreamCmd cmd;
  cmd.uid = uid;
  cmd.mute = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_MUTE_REMOTE_AUDIO_STREAM, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::muteRemoteVideoStream(uid_t uid, bool mute) {
  if (!m_initialized) return -1;
  RemoteVideoStreamCmd cmd;
  cmd.uid = uid;
  cmd.mute = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_MUTE_REMOTE_VIDEO_STREAM, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::setRemoteVideoStreamType(
    uid_t uid, REMOTE_VIDEO_STREAM_TYPE streamType) {
  if (!m_initialized) return -1;
  RemoteVideoStreamTypeCmd cmd;
  cmd.uid = uid;
  cmd.streamType = streamType;
  m_ipcMsg->sendMessage(AGORA_IPC_SET_REMOTE_VIDEO_STREAM_TYPE, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::muteLocalAudioStream(bool mute) {
  if (!m_initialized) return -1;
  LocalAudioStreamCmd cmd;
  cmd.mute = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_MUTE_LOCAL_AUDIO_STREAM, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::muteLocalVideoStream(bool mute) {
  if (!m_initialized) return -1;
  LocalVideoStreamCmd cmd;
  cmd.mute = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_MUTE_LOCAL_VIDEO_STREAM, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::muteAllRemoteAudioStreams(bool mute) {
  if (!m_initialized) return -1;
  MuteAllRemoteAudioStreamCmd cmd;
  cmd.enabled = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_MUTE_ALL_REMOTE_AUDIO_STREAM, (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::setDefaultMuteAllRemoteAudioStreams(bool mute) {
  if (!m_initialized) return -1;
  SetDefaultMuteRemoteAudioStreamCmd cmd;
  cmd.enabled = mute;
  m_ipcMsg->sendMessage(AGORA_IPC_SET_DEFAULT_REMOTE_MUTE_ALL_AUDIO_STREAMS,
                        (char*)&cmd,
                        sizeof(cmd));
  return 0;
}
int AgoraVideoSourceSink::setupRemoteVideo(VideoCanvas canvas,
                                           delegate_render render) {
  if (!m_initialized) return -1;
  RemoteVideoCmd cmd;
  cmd.uid = canvas.uid;
  cmd.view = (HWND)canvas.view;
  cmd.renderMode = (agora::rtc::RENDER_MODE_TYPE)canvas.renderMode;
  m_ipcMsg->sendMessage(AGORA_IPC_SETUP_REMOTE_VIDEO, (char*)&cmd, sizeof(cmd));
  m_mapViewRender[canvas.view] = render;
  return 0;
}

int AgoraVideoSourceSink::enableHardWareVideoEncoding(bool enabled) {
  if (m_initialized) {
    EnabledHWVideoEncodingCmd cmd;
    cmd.enabled = enabled;
    return m_ipcMsg->sendMessage(AGORA_IPC_ENABLED_HARDWARE_ENCODING, (char *)&cmd, 0)
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

void AgoraVideoSourceSink::msgThread() { m_ipcMsg->run(); }

node_error AgoraVideoSourceSink::join(const char* token, const char* cname,
                                      const char* chan_info, uid_t uid) {
  if (m_initialized) {
    m_peerUid = uid;
    std::unique_ptr<JoinChannelCmd> cmd(new JoinChannelCmd());
    cmd->uid = uid;
    if (token) strncpy_s(cmd->token, token, MAX_TOKEN_LEN);
    if (cname) {
      strncpy_s(cmd->cname, cname, MAX_CNAME_LEN);
    }
    if (chan_info) strncpy_s(cmd->chan_info, chan_info, MAX_CHAN_INFO);

    return m_ipcMsg->sendMessage(AGORA_IPC_JOIN, (char*)cmd.get(),
                                 sizeof(JoinChannelCmd))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::leave() {
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_LEAVE_CHANNEL, nullptr, 0)
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::renewVideoSourceToken(const char* token) {
  if (!token) return node_invalid_args;
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_RENEW_TOKEN, (char*)token,
                                 strlen(token))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::setVideoSourceChannelProfile(
    agora::rtc::CHANNEL_PROFILE_TYPE profile) {
  if (m_initialized) {
    // std::unique_ptr<ChannelProfileCmd> cmd(new ChannelProfileCmd());
    // cmd->profile = profile;
    ChannelProfileCmd cmd;
    cmd.profile = profile;
    return m_ipcMsg->sendMessage(AGORA_IPC_SET_CHANNEL_PROFILE, (char*)&cmd,
                                 sizeof(cmd))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::setVideoSourceVideoProfile(
    agora::rtc::VIDEO_PROFILE_TYPE profile, bool swapWidthAndHeight) {
  if (m_initialized) {
    VideoProfileCmd cmd(profile, swapWidthAndHeight);
    return m_ipcMsg->sendMessage(AGORA_IPC_SET_VIDEO_RPOFILE, (char*)&cmd,
                                 sizeof(cmd))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::setVideoSourceVideoEncoderConfiguration(
    VideoEncoderConfiguration config) {
  if (m_initialized) {
    VideoEncoderConfigurationCmd cmd(config);
    return m_ipcMsg->sendMessage(AGORA_IPC_SET_VIDEO_ENCODER_CONFIGURATION,
                                 (char*)&cmd, sizeof(cmd))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

void AgoraVideoSourceSink::onMessage(unsigned int msg, char* payload,
                                     unsigned int len) {
  LOG_INFO("Receive msg : %d\n", msg);
  if (msg == AGORA_IPC_SOURCE_READY) {
    m_event.notifyAll();
  }
  if (!m_initialized) return;
  if (msg == AGORA_IPC_JOIN_SUCCESS) {
    m_peerJoined = true;
    if (m_eventHandler) {
      m_eventHandler->onVideoSourceJoinedChannel(
          *((agora::rtc::uid_t*)payload));
    }
  } else if (msg == AGORA_IPC_FIRST_LOCAL_VIDEO) {
    if (m_eventHandler) {
      FirtLocalVideoFrame* first = (FirtLocalVideoFrame*)&payload;
      m_eventHandler->onVideoSourceFirstVideoFrame(first->width, first->height,
                                                   first->elapsed);
    }
  } else if (msg == AGORA_IPC_LEAVE_CHANNEL) {
    if (m_eventHandler) {
      m_eventHandler->onVideoSourceLeaveChannel();
    }
  } else if (msg == AGORA_IPC_RENEW_TOKEN) {
    if (m_eventHandler) {
      m_eventHandler->onVideoSourceRequestNewToken();
    }
  } else if (msg == AGORA_IPC_RENDER_READY) {
    /* TBD */
  } else if (msg == AGORA_IPC_START_VS_PREVIEW_COMPLETE) {
    onStartPreviewComplete();
  } else if (msg == AGORA_IPC_STOP_VS_PREVIEW_COMPLETE) {
    //m_ipcReceiver.reset();
  } else if (msg == AGORA_IPC_DISCONNECT) {
    m_ipcMsg->disconnect();
  }
}

node_error AgoraVideoSourceSink::captureScreen(agora::rtc::WindowIDType id,
                                               int captureFreq,
                                               agora::rtc::Rect* rect,
                                               int bitrate) {
  if (m_initialized) {
    CaptureScreenCmd cmd;
    cmd.windowid = id;
    cmd.captureFreq = captureFreq;
    if (rect) {
      cmd.rect = *rect;
    }
    cmd.bitrate = bitrate;
    return m_ipcMsg->sendMessage(AGORA_IPC_CAPTURE_SCREEN, (char*)&cmd,
                                 sizeof(cmd))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::updateScreenCapture(agora::rtc::Rect* rect) {
  if (!rect) return node_invalid_args;
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_UPDATE_CAPTURE_SCREEN, (char*)rect,
                                 sizeof(*rect))
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

node_error AgoraVideoSourceSink::stopCaptureScreen() {
  if (m_initialized) {
    return m_ipcMsg->sendMessage(AGORA_IPC_STOP_CAPTURE_SCREEN, nullptr, 0)
               ? node_ok
               : node_generic_error;
  }
  return node_status_error;
}

void AgoraVideoSourceSink::deliverFrame(const char* payload, int len) {
  if (len > (int)m_backBuf.size()) return;
  char* pBack = m_backBuf.data();
  memcpy(pBack, payload, len);
  /*if (m_d3dRender)
  {
          m_d3dRender->Render(pBack + 44);
  }*/
  void* view;
  auto video_frame = m_parser.gen_video_frame(
      reinterpret_cast<unsigned char*>(pBack), len, &view);

  if (m_customRender) {
    auto iter = m_mapViewRender.find(view);
    if (iter != m_mapViewRender.end() && iter->second)
      iter->second(video_frame.get());
  }
  // auto *p = getNodeVideoFrameTransporter();
  // p->deliverVideoSourceFrame(pBack, len);
}
}  // namespace rtc
}  // namespace agora
