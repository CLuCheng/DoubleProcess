/*
 * Copyright (c) 2018 Agora.io
 * All rights reserved.
 * Proprietary and Confidential -- Agora.io
 */

/*
 *  Created by Wang Yongli, 2018
 */

/**
 * the file define classes used to deliver command and event between node ADDON
 * and video source process.
 */

#ifndef AGORA_VIDEO_SOURCE_IPC_H
#define AGORA_VIDEO_SOURCE_IPC_H
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "IAgoraRtcEngine.h"
#include "ipc_shm.h"

/**
 * AgoraIpcMsg define the message type transferred between node ADDON and video
 * source process
 */
enum AgoraIpcMsg {
  /**  obsolete*/
  AGORA_IPC_CONNECT = 1,
  /** obsolete  */
  AGORA_IPC_CONNECT_CONFIRM,
  /** obsolete  */
  AGORA_IPC_DISCONNECT,
  /** To notify video source to join channel */
  AGORA_IPC_JOIN,
  /** To notify Video Sink that Video Source is ready*/
  AGORA_IPC_SOURCE_READY,
  /** Video source ==> node ADDON join success event */
  AGORA_IPC_JOIN_SUCCESS,
  /** Node ADDON ==> video source to begin capture screen */
  AGORA_IPC_CAPTURE_SCREEN,
  /** Node ADDON ==> video source to update screen sharing area*/
  AGORA_IPC_UPDATE_CAPTURE_SCREEN,
  /** Node ADDON ==> video source to stop capture screen */
  AGORA_IPC_STOP_CAPTURE_SCREEN,
  /** Node ADDON ==> video source to start video */
  AGORA_IPC_START_CAMERA,
  /** Node ADDON ==> video source to update channelKey */
  AGORA_IPC_RENEW_TOKEN,
  /** Node ADDON ==> video source to set channel profile */
  AGORA_IPC_SET_CHANNEL_PROFILE,
  /** Node ADDON ==> video source to set video profile */
  AGORA_IPC_SET_VIDEO_RPOFILE,
  /** video source to set video encoder configuration*/
  AGORA_IPC_SET_VIDEO_ENCODER_CONFIGURATION,
  /** Node ADDON ==> video source to leave channel */
  AGORA_IPC_LEAVE_CHANNEL,
  /** video source render is ready */
  AGORA_IPC_RENDER_READY,
  /**  Node ADDON ==> video source, To start transfer local video of video
     source.*/
  AGORA_IPC_START_VS_PREVIEW,
  /** video source ==> Node ADDON, local video preview complete.*/
  AGORA_IPC_START_VS_PREVIEW_COMPLETE,
  /** Node ADDON ==> video source, to stop transfer local video of video source.
   */
  AGORA_IPC_STOP_VS_PREVIEW,
  /** video source ==> Node ADDON, local video preview stopped.*/
  AGORA_IPC_STOP_VS_PREVIEW_COMPLETE,
  /** Node ADDON ==> video source, to enable interoperability with the Agora Web
     SDK*/
  AGORA_IPC_ENABLE_WEB_SDK_INTEROPERABILITY,
  /** Node ADDON ==> video source, to enable dual stream with the Agora Web
     SDK*/
  AGORA_IPC_ENABLE_DUAL_STREAM_MODE,
  AGORA_IPC_SET_LOGFILE,
  /** Node ADDON ==> video source, to set rtc parameters*/
  AGORA_IPC_SET_PARAMETER,
  AGORA_IPC_SET_CUSTOM_RENDER,

  AGORA_IPC_SETUP_LOCAL_VIDEO,
  AGORA_IPC_FIRST_LOCAL_VIDEO,
  AGORA_UPC_SETUP_LOCAL_RENDER_MODE,
  AGORA_IPC_ENABLE_VIDEO,
  AGORA_IPC_ENABLE_AUDIO,
  AGORA_IPC_ENABLE_LOCAL_VIDEO,
  AGORA_IPC_ENABLE_LOCAL_AUDIO,
  AGORA_IPC_MUTE_REMOTE_AUDIO_STREAM,
  AGORA_IPC_MUTE_REMOTE_VIDEO_STREAM,
  AGORA_IPC_SET_REMOTE_VIDEO_STREAM_TYPE,
  AGORA_IPC_MUTE_LOCAL_AUDIO_STREAM,
  AGORA_IPC_MUTE_LOCAL_VIDEO_STREAM,
  AGORA_IPC_SETUP_REMOTE_VIDEO,
  AGORA_IPC_MUTE_ALL_REMOTE_AUDIO_STREAM,
  AGORA_IPC_SET_DEFAULT_REMOTE_MUTE_ALL_AUDIO_STREAMS,
  AGORA_IPC_ENABLED_HARDWARE_ENCODING

};

/**
 * Screen capture parameters when ADDON ask video source to start screen
 * sharing.
 */
struct CaptureScreenCmd {
#if defined(__APPLE__)
  unsigned int windowid;
#elif defined(_WIN32)
  HWND windowid;
#endif
  //   agora::rtc::IRtcEngine::WindowIDType windowid;
  int captureFreq;
  agora::rtc::Rect rect;
  int bitrate;
  CaptureScreenCmd() : windowid(0), captureFreq(0), rect(), bitrate() {}
};

#define MAX_TOKEN_LEN 320
#define MAX_CNAME_LEN 256
#define MAX_CHAN_INFO 512
#define MAX_PERMISSION_KEY 128
/**
 * Join channel parameters when ADDON ask video source to join channel
 */
struct JoinChannelCmd {
  agora::rtc::uid_t uid;
  char token[MAX_TOKEN_LEN];
  char cname[MAX_CNAME_LEN];
  char chan_info[MAX_CHAN_INFO];
  JoinChannelCmd() : uid{0} {
    memset(token, 0, MAX_TOKEN_LEN);
    memset(cname, 0, MAX_CNAME_LEN);
    memset(chan_info, 0, MAX_CHAN_INFO);
  }
};

/**
 * video profile parameters when ADDON ask video source to set video profile
 */
struct VideoProfileCmd {
  agora::rtc::VIDEO_PROFILE_TYPE profile;
  bool swapWidthAndHeight;
  VideoProfileCmd()
      : profile(agora::rtc::VIDEO_PROFILE_DEFAULT), swapWidthAndHeight(false) {}
  VideoProfileCmd(agora::rtc::VIDEO_PROFILE_TYPE type, bool swap)
      : profile(type), swapWidthAndHeight(swap) {}
};

/**
 * video encoder configuration
 */
struct VideoEncoderConfigurationCmd {
  int width;
  int height;
  agora::rtc::FRAME_RATE frameRate;
  int bitrate;
  int minBitrate;
  agora::rtc::ORIENTATION_MODE orientationMode;
  VideoEncoderConfigurationCmd()
      : width(640),
        height(480),
        frameRate(agora::rtc::FRAME_RATE_FPS_15),
        bitrate(agora::rtc::STANDARD_BITRATE),
        minBitrate(agora::rtc::DEFAULT_MIN_BITRATE),
        orientationMode(agora::rtc::ORIENTATION_MODE_ADAPTIVE) {}
  VideoEncoderConfigurationCmd(agora::rtc::VideoEncoderConfiguration config)
      : width(config.dimensions.width),
        height(config.dimensions.height),
        frameRate(config.frameRate),
        bitrate(config.bitrate),
        minBitrate(config.minBitrate),
        orientationMode(config.orientationMode) {}
};

/**
 * channel profile parameters when ADDON ask video source to set channel profile
 */
struct ChannelProfileCmd {
  agora::rtc::CHANNEL_PROFILE_TYPE profile;
  ChannelProfileCmd()
      : profile(agora::rtc::CHANNEL_PROFILE_LIVE_BROADCASTING) {}
};

#define MAX_PARAMETER_LEN 512
struct SetParameterCmd {
  char parameters[MAX_PARAMETER_LEN];

 public:
  SetParameterCmd() {}
};

struct LocalVideoCmd {
  HWND view;
  agora::rtc::uid_t uid;
  agora::rtc::RENDER_MODE_TYPE renderMode;
  char cameraId[MAX_PARAMETER_LEN];  // MAX_DEVICE_ID_LENGTH 512
 public:
  LocalVideoCmd() : view(NULL), renderMode(agora::rtc::RENDER_MODE_FIT) {}
};

struct FirtLocalVideoFrame {
  int width;
  int height;
  int elapsed;

 public:
  FirtLocalVideoFrame() : width(0), height(0), elapsed(0) {}
};

struct CustomRenderCmd {
  bool enable;

 public:
  CustomRenderCmd() : enable(true) {}
};

struct LocalRenderModeCmd {
  int renderMode;

 public:
  LocalRenderModeCmd() : renderMode(0) {}
};

struct StartPreviewCmd {
  char cameraId[512];

 public:
  StartPreviewCmd() { memset(cameraId, 0, 512); }
};

struct EnableAudioCmd {
  bool enable;

 public:
  EnableAudioCmd() : enable(false) {}
};

struct EnableLocalVideoCmd {
  bool enable;

 public:
  EnableLocalVideoCmd() : enable(false) {}
};

struct EnableLocalAudioCmd {
  bool enable;

 public:
  EnableLocalAudioCmd() : enable(false) {}
};

struct EnableVideoCmd {
  bool enable;

 public:
  EnableVideoCmd() : enable(false) {}
};

struct RemoteAudioStreamCmd {
  agora::rtc::uid_t uid;
  bool mute;

 public:
  RemoteAudioStreamCmd() : uid(0), mute(false) {}
};

struct RemoteVideoStreamCmd {
  agora::rtc::uid_t uid;
  bool mute;

 public:
  RemoteVideoStreamCmd() : uid(0), mute(false) {}
};

struct RemoteVideoStreamTypeCmd {
  agora::rtc::uid_t uid;
  agora::rtc::REMOTE_VIDEO_STREAM_TYPE streamType;

 public:
  RemoteVideoStreamTypeCmd()
      : uid(0), streamType(agora::rtc::REMOTE_VIDEO_STREAM_HIGH) {}
};

struct LocalAudioStreamCmd {
  bool mute;

 public:
  LocalAudioStreamCmd() : mute(false) {}
};
struct LocalVideoStreamCmd {
  bool mute;

 public:
  LocalVideoStreamCmd() : mute(false) {}
};

struct RemoteVideoCmd {
  HWND view;
  agora::rtc::RENDER_MODE_TYPE renderMode;
  agora::rtc::uid_t uid;

 public:
  RemoteVideoCmd() : view(NULL), renderMode(agora::rtc::RENDER_MODE_FIT) {}
};

struct MuteAllRemoteAudioStreamCmd {
  bool enabled;

 public:
  MuteAllRemoteAudioStreamCmd() : enabled(false) {}
};

struct SetDefaultMuteRemoteAudioStreamCmd {
  bool enabled;

 public:
  SetDefaultMuteRemoteAudioStreamCmd() : enabled(false) {}
};

struct EnabledHWVideoEncodingCmd {
  bool enabled;

 public:
  EnabledHWVideoEncodingCmd() : enabled(false) {}

};

/**
 * AgoraIpcListener is used to monitor IPC message
 */
class AgoraIpcListener {
 public:
  virtual ~AgoraIpcListener() {}
  virtual void onMessage(unsigned int msg, char* payload, unsigned int len) {
    (void)payload;
    (void)len;
  }
};

/**
 * IAgoraIpc is used to facilitate communications between processes. This is one
 * virtual class, may have different implementation on different platforms.
 */
class IAgoraIpc {
 public:
  IAgoraIpc(AgoraIpcListener* listener) : m_listener(listener) {}
  virtual ~IAgoraIpc() {}
  virtual const std::string& getId() { return m_id; }
  /**
   * To initialize IPC.
   * @param id : the id used to identify the IPC endpoint.
   */
  virtual bool initialize(const std::string& id) = 0;
  /**
   * To put IPC endpoint in listen state, then other endpoint can connect the
   * endpoint.
   */
  virtual bool listen() = 0;
  /**
   * To connect to other IPC endpoint.
   */
  virtual bool connect() = 0;
  /**
   * To disconnect the IPC
   */
  virtual bool disconnect() = 0;
  /**
   * To start IPC.
   */
  virtual void run() = 0;
  /**
   * To send message.
   * @param msg : the message id.
   * @param payload : the pointer to the transferred data.
   * @param len : the length of the data payload.
   */
  virtual bool sendMessage(AgoraIpcMsg msg, char* payload,
                           unsigned int len) = 0;

 protected:
  std::string m_id;
  AgoraIpcListener* m_listener;
};

/**
 * The class is used for IPC with large throughput.
 * AgoraIpcDataSender provide send-only facilities.
 */
#define DATA_DELIVER_BLOCK_SIZE 6145000
// 3110450
class AgoraIpcDataSender {
 public:
  AgoraIpcDataSender();
  ~AgoraIpcDataSender();

  /**
   * To initialize the AgoraIpcDataSender.
   * @param id : sender identifier.
   */
  bool initialize(const std::string& id);
  /**
   * To send data.
   * @param payload : the pointer to data to be sent.
   * @param len : the length of the data.
   */
  void sendData(char* payload, unsigned int len);
  /**
   * To send multiple data in one time.
   * @payloads : vector of data to be sent.
   */
  void sendMultiData(const std::vector<std::pair<char*, int32_t>>& payloads);

  /** Disconnect the sender and IPC */
  void Disconnect();

 private:
  shm_ipc<1, DATA_DELIVER_BLOCK_SIZE> m_ipcData;
  bool m_initialized;
  std::string m_id;
};

/**
 * The class is used to IPC with large throughput.
 * The AgoraIpcDataReceiver provide receive-only facilities.
 */
class AgoraIpcDataReceiver {
 public:
  AgoraIpcDataReceiver();
  ~AgoraIpcDataReceiver();

  /**
   * To initialize AgoraIpcDataReciever
   * @param id : IPC identifier
   * @param handler : the receiver event handler.
   */
  bool initialize(const std::string& id,
                  const std::function<void(const char*, int)>& handler);
  /**
   * To start the IPC.
   */
  void run(bool async = false);

  /**
   * To stop the IPC
   */
  void stop();

 private:
  std::function<void(const char*, int)> m_handler;
  std::string m_id;
  bool m_initialized;
  shm_ipc<1, DATA_DELIVER_BLOCK_SIZE> m_ipcData;
  std::unique_ptr<std::thread> m_thread;
};

/**
 * To create IAgroaIpc instance.
 */
IAgoraIpc* createAgoraIpc(AgoraIpcListener* listner);

#endif
