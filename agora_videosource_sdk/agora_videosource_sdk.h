
#pragma once
#include <memory>
#include <unordered_map>
#include "IAgoraVideoSourceEngine.h"
#include "agora_video_source.h"
namespace agora {
namespace rtc {

class CAgoraVideoSourceEventHandler : public IAgoraVideoSourceEventHandler {
  // callback
  virtual void onVideoSourceJoinedChannel(agora::rtc::uid_t uid) override;
  virtual void onVideoSourceRequestNewToken() override;
  virtual void onVideoSourceLeaveChannel() override;
  virtual void onVideoSourceExit() override;
  virtual void onVideoSourceFirstVideoFrame(int width, int height,
                                            int elapsed) override;

 public:
  void SetEventHandler(IVideoSourceEventHandler* enventHandler) {
    m_pEventHandler = enventHandler;
  }

 private:
  IVideoSourceEventHandler* m_pEventHandler;
};

class AgoraVideoSourceEngine : public IAgoraVideoSourceEngine {
 public:
  AgoraVideoSourceEngine();
  ~AgoraVideoSourceEngine();

  virtual void release() override;
  virtual bool initialize(const char* appidd,
                          IVideoSourceEventHandler* enventHandler) override;
  virtual int setChannelProfile(CHANNEL_PROFILE_TYPE_ profile) override;
  virtual int setVideoProfile(VIDEO_PROFILE_TYPE_ profile,
                              bool swapWidthAndHeight) override;
  virtual int setVideoEncoderConfiguration(
      VideoEncoderConfiguration_ config) override;
  virtual int enableWebSdkInteroperability(bool enable) override;
  virtual int startScreenCapture(HWND windowId, int capFreq, Rect_ rect,
                                 int bitrate) override;
  virtual int stopScreenCapture() override;
  virtual int setupLocalVideo(VideoCanvas_ canvas, 
                              delegate_render render) override;
  virtual int startPreview(const char* cameraId) override;
  virtual int stopPreview() override;
  virtual bool enableDualStreamMode(bool enable) override;
  virtual int renewToken(const char* token) override;
  virtual int joinChannel(const char* token, const char* channelId,
                          const char* info, uid_t_ uid) override;
  virtual int leaveChannel() override;
  virtual int setCustomRender(bool enable) override;
  virtual int setLocalRenderMode(RENDER_MODE_TYPE_ mode) override;
  virtual int enableVideo(bool enabled) override;
  virtual int enableAudio(bool enabled) override;
  
  virtual int enableLocalVideo(bool enabled) override;
  virtual int enableLocalAudio(bool enabled) override;

  virtual int muteRemoteAudioStream(uid_t_ uid, bool mute) override;
  virtual int muteRemoteVideoStream(uid_t_ userId, bool mute) override;
  virtual int setRemoteVideoStreamType(
      uid_t_ userId, REMOTE_VIDEO_STREAM_TYPE_ streamType) override;
  virtual int enableHardWareVideoEncoding(bool enabled) override;
  virtual int muteLocalAudioStream(bool mute) override;
  virtual int muteLocalVideoStream(bool mute) override;
  virtual int muteAllRemoteAudioStreams(bool mute) override;
  virtual int setDefaultMuteAllRemoteAudioStreams(bool mute) override;
  virtual int setupRemoteVideo(VideoCanvas_ canvas,
                               delegate_render render = nullptr) override;

 private:
  std::unique_ptr<AgoraVideoSource> m_videoSourceSink;
  CAgoraVideoSourceEventHandler m_videoSourceEventHandler;
  bool m_initialized;
};

class IAgoraEventHandler : public IRtcEngineEventHandler {
 public:
  virtual void onAudioVolumeIndication(const AudioVolumeInfo* speakers,
                                       unsigned int speakerNumber,
                                       int totalVolume) {
    if (m_recevicer) {
      m_recevicer->onAudioVolumeIndication((AudioVolumeInfo_*)speakers,
                                           speakerNumber, totalVolume);
    }
  }

  virtual void setEventRecevier(IAgoraDeviceEventHandler* eventHandler) {
    m_recevicer = eventHandler;
  }

 private:
  IAgoraDeviceEventHandler* m_recevicer;
};

class AgoraDeviceEngine : public IAgoraDeviceEngine {
 public:
  agora::rtc::IRtcEngine* m_rtcEngine;
  std::vector<IAgoraVideoSourceEngine*> m_vecEngine;
  IAgoraEventHandler m_eventHandler;
  std::unique_ptr<AAudioDeviceManager> m_audioDeviceManager;
  std::unique_ptr<AVideoDeviceManager> m_videoDeviceManager;
  size_t device_engine_count_;
 public:
  AgoraDeviceEngine();
  ~AgoraDeviceEngine();
  void release() override;
  bool initialize(const char* appid, size_t size) override;
  int getPlaybackDeviceVolume(int* volume) override;
  int getRecordingDeviceVolume(int* volume) override;
  int enumerateVideoDevices(Device* device) override;
  int enumerateRecordingDevices(Device* device) override;
  int enumeratePlaybackDevices(Device* device) override;
  int setRecordingDevice(const char deviceId[DEVICE_MAX_NAME_LENGTH]) override;
  int enableAudioVolumeIndication(int interval, int smooth,
                                  bool report_vad) override;
  int startRecordingDeviceTest(int indicationInterval) override;
  int stopRecordingDeviceTest() override;
  int setPlaybackDevice(const char deviceId[DEVICE_MAX_NAME_LENGTH]) override;
  int startPlaybackDeviceTest(const char* testAudioFilePath) override;
  int stopPlaybackDeviceTest() override;
  int setDevice(int idx,const char deviceId[MAX_DEVICE_ID_LENGTH]) override;
  int startDeviceTest(int idx, void* hwnd) override;
  int stopDeviceTest(int idx) override;
  int setRecordingDeviceVolume(int volume) override;
  int setPlaybackDeviceVolume(int volume) override;
  int registerEventHandler(IAgoraDeviceEventHandler* eventHandler) override;
};

}  // namespace rtc
}  // namespace agora
