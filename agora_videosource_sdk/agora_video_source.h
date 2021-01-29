/*
 * Copyright (c) 2018 Agora.io
 * All rights reserved.
 * Proprietary and Confidential -- Agora.io
 */

/*
 *  Created by Wang Yongli, 2018
 */

#ifndef AGORA_VIDEO_SOURCE_SINK_H
#define AGORA_VIDEO_SOURCE_SINK_H

#include <functional>
#include <string>
#include "IAgoraVideoSourceEngine.h"
#include "IAgoraMediaEngine.h"
#include "IAgoraRtcEngine.h"
#include "node_error.h"
namespace agora {
namespace rtc {

#if defined(__APPLE__)
typedef unsigned int WindowIDType;
#elif defined(_WIN32)
typedef HWND WindowIDType;
#endif
//using delegate_render = std::function<void(agora::media::IVideoFrame*)>;

/**
 * Video source need join the same channel, this class used to monitor video
 * source's event. More event maybe needed in future.
 */

class IAgoraVideoSourceEventHandler {
 public:
  virtual ~IAgoraVideoSourceEventHandler() {}
  /**
   * Video source joined channel success event.
   * @param uid : video source's uid.
   */
  virtual void onVideoSourceJoinedChannel(agora::rtc::uid_t uid) = 0;

  /**
   * Video source request new token event.
   */
  virtual void onVideoSourceRequestNewToken() = 0;

  /**
   * Video source leaved channel event.
   */
  virtual void onVideoSourceLeaveChannel() = 0;

  virtual void onVideoSourceExit() = 0;

  virtual void onVideoSourceFirstVideoFrame(int width, int height,
                                            int elapsed) = 0;
};

/**
 * This is video source stub interface.
 */
class AgoraVideoSource {
 public:
  virtual ~AgoraVideoSource() {}

  /**
   * To initialize Video source.
   * @param eventHandler : video source event handler.
   */
  virtual bool initialize(IAgoraVideoSourceEventHandler* eventHandler,
                          const char* appid) = 0;

  /**
   * To ask video source to join channel with specified parameters.
   * @param token : token if it is enabled.
   * @param cname : channel name.
   * @param chan_info : channel information
   * @param uid : uid of video source.
   */
  virtual node_error join(const char* token, const char* cname,
                          const char* chan_info, uid_t uid) = 0;

  /**
   * To ask video source to leave channel.
   */
  virtual node_error leave() = 0;

  /**
   * To start receive local video of video source.
   */
  virtual node_error startPreview(const char* cameraId) = 0;

  virtual node_error setupLocalVideo(HWND hWnd, uid_t uid,
                                     agora::rtc::RENDER_MODE_TYPE renderMode,
                                     delegate_render render) = 0;

  /**
   * To stop receive local video of video source.
   */
  virtual node_error stopPreview() = 0;

  /**
   * Release video source.
   */
  virtual node_error release() = 0;

  /**
   * To ask video source begin to share screen.
   * @param id : window id whose window will be shared. if the value is 0, then
   * desktop is shared.
   * @param captureFreq : video frequency, 0-15.
   * @param rect : the shared area
   * @param bitrate : bitrate of video
   */
  virtual node_error captureScreen(agora::rtc::WindowIDType id, int captureFreq,
                                   agora::rtc::Rect* rect, int bitrate) = 0;

  /**
   * To update shared window area
   * @param rect : updated area
   */
  virtual node_error updateScreenCapture(agora::rtc::Rect* rect) = 0;

  /**
   * To stop screen share
   */
  virtual node_error stopCaptureScreen() = 0;

  /**
   * To renew video source's token.
   * @param token : new token
   */
  virtual node_error renewVideoSourceToken(const char* token) = 0;

  /**
   * To set video source channel profile
   * @param profile : video source's channel profile
   */
  virtual node_error setVideoSourceChannelProfile(
      agora::rtc::CHANNEL_PROFILE_TYPE profile) = 0;

  /**
   * To set video source's video profile
   * @param profile : the video source's video profile
   * @param swapWidthAndHeight : whether adjust width and height
   */
  virtual node_error setVideoSourceVideoProfile(
      agora::rtc::VIDEO_PROFILE_TYPE profile, bool swapWidthAndHeight) = 0;

  /**
   */
  virtual node_error setVideoSourceVideoEncoderConfiguration(
      VideoEncoderConfiguration config) = 0;

  /**
   * Enable interoperability with the Agora Web SDK.
   * @param enabled : whether interoperability with the Agora Web SDK is enabled
   */
  virtual node_error enableWebSdkInteroperability(bool enabled) = 0;

  /**
   * Enable dual stream mode with the Agora Web SDK.
   * @param enabled : whether dual stream with the Agora Web SDK is enabled
   */
  virtual node_error enableDualStreamMode(bool enabled) = 0;

  /**
   * set log file path of video source
   * @param file : filepath of log
   */
  virtual node_error setLogFile(const char* file) = 0;

  /**
   * To set parameters for video source.
   */
  virtual void setParameters(const char* parameters) = 0;

  virtual int setCustomRender(bool enable) = 0;

  virtual int setLocalRenderMode(RENDER_MODE_TYPE mode) = 0;

  virtual int enableVideo(bool enabled) = 0;
  virtual int enableAudio(bool enabled) = 0;

  virtual int enableLocalVideo(bool enabled) = 0;
  virtual int enableLocalAudio(bool enabled) = 0;

  virtual int muteRemoteAudioStream(uid_t uid, bool mute) = 0;
  virtual int muteRemoteVideoStream(uid_t userId, bool mute) = 0;
  virtual int setRemoteVideoStreamType(
      uid_t userId, REMOTE_VIDEO_STREAM_TYPE streamType) = 0;
  virtual int muteLocalAudioStream(bool mute) = 0;
  virtual int muteLocalVideoStream(bool mute) = 0;
  virtual int muteAllRemoteAudioStreams(bool mute) = 0;
  virtual int setDefaultMuteAllRemoteAudioStreams(bool mute) = 0;
  virtual int setupRemoteVideo(VideoCanvas canvas, delegate_render render) = 0;
  virtual int enableHardWareVideoEncoding(bool enabled) = 0;
  
};

/**
 * Video source may be has different implementation on different platforms. The
 * API is used to generate video source.
 */
AgoraVideoSource* createVideoSource();
}  // namespace rtc
}  // namespace agora

#endif
