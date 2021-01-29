// agora_videosource_sdk.cpp : Defines the exported functions for the DLL
// application.
//
#include "agora_videosource_sdk.h"
#include "IAgoraMediaEngine.h"
#include "IAgoraRtcEngine.h"
namespace agora {
namespace rtc {
// CAgoraVideoSourceEventHandler
void CAgoraVideoSourceEventHandler::onVideoSourceJoinedChannel(
    agora::rtc::uid_t uid) {
  if (m_pEventHandler) m_pEventHandler->onVideoSourceJoinedChannelSuccess(uid);
}

void CAgoraVideoSourceEventHandler::onVideoSourceRequestNewToken() {
  if (m_pEventHandler) m_pEventHandler->onVideoSourceRequestNewToken();
}

void CAgoraVideoSourceEventHandler::onVideoSourceLeaveChannel() {
  if (m_pEventHandler) m_pEventHandler->onVideoSourceLeaveChannel();
}

void CAgoraVideoSourceEventHandler::onVideoSourceExit() {
  if (m_pEventHandler) m_pEventHandler->onVideoSourceExit();
}

void CAgoraVideoSourceEventHandler::onVideoSourceFirstVideoFrame(int width,
                                                                 int height,
                                                                 int elapsed) {
  if (m_pEventHandler)
    m_pEventHandler->onVideoSourceFirstVideoFrame(width, height, elapsed);
}

// AgoraVideoSourceEngine
AgoraVideoSourceEngine::AgoraVideoSourceEngine() {
  m_videoSourceSink.reset(createVideoSource());
}

AgoraVideoSourceEngine::~AgoraVideoSourceEngine() {}

bool AgoraVideoSourceEngine::initialize(
    const char* appid, IVideoSourceEventHandler* enventHandler) {
  m_videoSourceEventHandler.SetEventHandler(enventHandler);
  return m_videoSourceSink->initialize(&m_videoSourceEventHandler, appid);
}

int AgoraVideoSourceEngine::setChannelProfile(CHANNEL_PROFILE_TYPE_ profile) {
  return m_videoSourceSink->setVideoSourceChannelProfile(
      (CHANNEL_PROFILE_TYPE)profile);
}

int AgoraVideoSourceEngine::setVideoProfile(VIDEO_PROFILE_TYPE_ profile,
                                            bool swapWidthAndHeight) {
  return m_videoSourceSink->setVideoSourceVideoProfile(
      (VIDEO_PROFILE_TYPE)profile, swapWidthAndHeight);
}

int AgoraVideoSourceEngine::setVideoEncoderConfiguration(
    VideoEncoderConfiguration_ config) {
  agora::rtc::VideoEncoderConfiguration adaptee_config;
  adaptee_config.bitrate = config.bitrate;
  adaptee_config.degradationPreference =
      (DEGRADATION_PREFERENCE)config.degradationPreference;
  adaptee_config.dimensions.width = config.dimensions.width;
  adaptee_config.dimensions.height = config.dimensions.height;
  adaptee_config.frameRate = (FRAME_RATE)config.frameRate;
  adaptee_config.minBitrate = config.minBitrate;
  adaptee_config.minFrameRate = config.minFrameRate;
  adaptee_config.mirrorMode = (VIDEO_MIRROR_MODE_TYPE)config.mirrorMode;
  adaptee_config.orientationMode = (ORIENTATION_MODE)config.orientationMode;
  return m_videoSourceSink->setVideoSourceVideoEncoderConfiguration(
      adaptee_config);
}

int AgoraVideoSourceEngine::enableWebSdkInteroperability(bool enable) {
  return m_videoSourceSink->enableWebSdkInteroperability(enable);
}

int AgoraVideoSourceEngine::startScreenCapture(HWND windowId, int capFreq,
                                               Rect_ rect, int bitrate) {
  return m_videoSourceSink->captureScreen(
      windowId, capFreq, reinterpret_cast<agora::rtc::Rect*>(&rect), bitrate);
}

int AgoraVideoSourceEngine::stopScreenCapture() {
  return m_videoSourceSink->stopCaptureScreen();
}

int AgoraVideoSourceEngine::startPreview(const char* cameraId) {
  return m_videoSourceSink->startPreview(cameraId);
}

int AgoraVideoSourceEngine::setupLocalVideo(VideoCanvas_ canvas,
                                            delegate_render render) {
  return m_videoSourceSink->setupLocalVideo((HWND)canvas.view, canvas.uid,
                                            (RENDER_MODE_TYPE)canvas.renderMode,
                                            render);
}

int AgoraVideoSourceEngine::stopPreview() {
  return m_videoSourceSink->stopPreview();
}

bool AgoraVideoSourceEngine::enableDualStreamMode(bool enable) {
  return node_ok == m_videoSourceSink->enableDualStreamMode(enable);
}

int AgoraVideoSourceEngine::renewToken(const char* token) {
  return m_videoSourceSink->renewVideoSourceToken(token);
}

int AgoraVideoSourceEngine::joinChannel(const char* token,
                                        const char* channelId, const char* info,
                                        uid_t_ uid) {
  return m_videoSourceSink->join(token, channelId, info, uid);
}

int AgoraVideoSourceEngine::leaveChannel() {
  return m_videoSourceSink->leave();
}

void AgoraVideoSourceEngine::release() {
  m_videoSourceSink->release();
  m_videoSourceEventHandler.SetEventHandler(nullptr);
}

int AgoraVideoSourceEngine::setCustomRender(bool enable) {
  return m_videoSourceSink->setCustomRender(enable);
}

int AgoraVideoSourceEngine::setLocalRenderMode(RENDER_MODE_TYPE_ mode) {
  return m_videoSourceSink->setLocalRenderMode((RENDER_MODE_TYPE)mode);
}

int AgoraVideoSourceEngine::muteRemoteAudioStream(uid_t_ uid, bool mute) {
  return m_videoSourceSink->muteRemoteAudioStream(uid, mute);
}
int AgoraVideoSourceEngine::muteRemoteVideoStream(uid_t_ uid, bool mute) {
  return m_videoSourceSink->muteRemoteVideoStream(uid, mute);
}
int AgoraVideoSourceEngine::setRemoteVideoStreamType(
    uid_t_ userId, REMOTE_VIDEO_STREAM_TYPE_ streamType) {
  return m_videoSourceSink->setRemoteVideoStreamType(
      userId, (REMOTE_VIDEO_STREAM_TYPE)streamType);
}
int AgoraVideoSourceEngine::enableHardWareVideoEncoding(bool enabled) {
  return m_videoSourceSink->enableHardWareVideoEncoding(enabled);
}
int AgoraVideoSourceEngine::muteLocalAudioStream(bool mute) {
  return m_videoSourceSink->muteLocalAudioStream(mute);
}
int AgoraVideoSourceEngine::muteLocalVideoStream(bool mute) {
  return m_videoSourceSink->muteLocalAudioStream(mute);
}
int AgoraVideoSourceEngine::muteAllRemoteAudioStreams(bool mute) {
  return m_videoSourceSink->muteAllRemoteAudioStreams(mute);
}
int AgoraVideoSourceEngine::setDefaultMuteAllRemoteAudioStreams(bool mute) {
  return m_videoSourceSink->setDefaultMuteAllRemoteAudioStreams(mute);
}
int AgoraVideoSourceEngine::setupRemoteVideo(VideoCanvas_ canvas,
                                             delegate_render render) {
  VideoCanvas vc;
  strcpy(vc.channelId, canvas.channelId);
  vc.mirrorMode = (VIDEO_MIRROR_MODE_TYPE)canvas.mirrorMode;
  vc.priv = canvas.priv;
  vc.renderMode = canvas.renderMode;
  vc.uid = canvas.uid;
  vc.view = canvas.view;
  return m_videoSourceSink->setupRemoteVideo(vc, render);
}
int AgoraVideoSourceEngine::enableVideo(bool enabled) {
  return m_videoSourceSink->enableVideo(enabled);
}
int AgoraVideoSourceEngine::enableAudio(bool enabled) {
  return m_videoSourceSink->enableAudio(enabled);
}

int AgoraVideoSourceEngine::enableLocalVideo(bool enabled) {
  return m_videoSourceSink->enableLocalVideo(enabled);
}

int AgoraVideoSourceEngine::enableLocalAudio(bool enabled) {
  return m_videoSourceSink->enableLocalAudio(enabled);
}

AgoraDeviceEngine::AgoraDeviceEngine() { m_rtcEngine = createAgoraRtcEngine(); }

AgoraDeviceEngine::~AgoraDeviceEngine() { release(); }

void AgoraDeviceEngine::release() {
  m_audioDeviceManager.reset();
  m_videoDeviceManager.reset();
  m_rtcEngine->release(true);
  for (auto engine:m_vecEngine)
  {
    engine->enableVideo(false);
    engine->release();
  }
  m_vecEngine.clear();
}
bool AgoraDeviceEngine::initialize(const char* appid, size_t size) {
  RtcEngineContext context;
  context.appId = appid;
  context.eventHandler = &m_eventHandler;
  // initialize the Agora RTC engine context.
  int ret = m_rtcEngine->initialize(context);
  m_rtcEngine->enableAudio();
  m_rtcEngine->enableVideo();
  device_engine_count_ = size;
  if (device_engine_count_ > 1) {
    auto engine = createAgoraVideoSourceEngine();
    engine->initialize(appid, nullptr);
    engine->enableAudio(false);
    engine->enableVideo(true);
    m_vecEngine.push_back(engine);
  }
  // create audio and video device manager.
  m_audioDeviceManager.reset(new AAudioDeviceManager(m_rtcEngine));
  m_videoDeviceManager.reset(new AVideoDeviceManager(m_rtcEngine));
  return 0;
}

int AgoraDeviceEngine::getPlaybackDeviceVolume(int* volume) {
  return (*m_audioDeviceManager)->getPlaybackDeviceVolume(volume);
}
int AgoraDeviceEngine::getRecordingDeviceVolume(int* volume) {
  return (*m_audioDeviceManager)->getRecordingDeviceVolume(volume);
}
int AgoraDeviceEngine::enumerateVideoDevices(Device* device) {
  // get camera devices and add to combobox and insert map.
  char szDeviceName[MAX_DEVICE_ID_LENGTH];
  char szDeviceId[MAX_DEVICE_ID_LENGTH];
  if (device == nullptr) return -1;
  device->count = 0;
  auto cameraDevices = (*m_videoDeviceManager)->enumerateVideoDevices();
  if (cameraDevices)
  {
    device->count = cameraDevices->getCount();
    for (int i = 0; i < cameraDevices->getCount(); i++) {
      int nRet = cameraDevices->getDevice(i, szDeviceName, szDeviceId);
      strcpy_s(device->id[i], szDeviceId);
      strcpy_s(device->name[i], szDeviceName);
    }
    cameraDevices->release();
  }
  return 0;
}

int AgoraDeviceEngine::enumerateRecordingDevices(Device* device) {
  // get audio record devices and add to combobox and insert map.
  char szDeviceName[MAX_DEVICE_ID_LENGTH];
  char szDeviceId[MAX_DEVICE_ID_LENGTH];
  if (device == nullptr) return -1;
  IAudioDeviceCollection* audioRecordDevices =
      (*m_audioDeviceManager)->enumerateRecordingDevices();
  device->count = 0;
  if (audioRecordDevices)
  {
    device->count = audioRecordDevices->getCount();
    for (int i = 0; i < audioRecordDevices->getCount(); i++) {
      int nRet = audioRecordDevices->getDevice(i, szDeviceName, szDeviceId);
      strcpy_s(device->id[i], szDeviceId);
      strcpy_s(device->name[i], szDeviceName);
    }
    audioRecordDevices->release();
  }
  return 0;
}
int AgoraDeviceEngine::enumeratePlaybackDevices(Device* device) {
  // get audio record devices and add to combobox and insert map.
  char szDeviceName[MAX_DEVICE_ID_LENGTH];
  char szDeviceId[MAX_DEVICE_ID_LENGTH];
  if (device == nullptr) return -1;
  // get audio playback devices and add to combobox and insert map.
  IAudioDeviceCollection* audioPlaybackDevices =
      (*m_audioDeviceManager)->enumeratePlaybackDevices();
  device->count = 0;
  if (audioPlaybackDevices)
  {
    device->count = audioPlaybackDevices->getCount();
    for (int i = 0; i < audioPlaybackDevices->getCount(); i++) {
      int nRet = audioPlaybackDevices->getDevice(i, szDeviceName, szDeviceId);
      strcpy_s(device->id[i], szDeviceId);
      strcpy_s(device->name[i], szDeviceName);
    }
    audioPlaybackDevices->release();
  }
  return 0;
}
int AgoraDeviceEngine::setRecordingDevice(
    const char deviceId[DEVICE_MAX_NAME_LENGTH]) {
  return (*m_audioDeviceManager)->setRecordingDevice(deviceId);
}
int AgoraDeviceEngine::enableAudioVolumeIndication(int interval, int smooth,
                                                   bool report_vad) {
  return m_rtcEngine->enableAudioVolumeIndication(interval, smooth, report_vad);
}
int AgoraDeviceEngine::startRecordingDeviceTest(int indicationInterval) {
  return (*m_audioDeviceManager)->startRecordingDeviceTest(indicationInterval);
}
int AgoraDeviceEngine::stopRecordingDeviceTest() {
  return (*m_audioDeviceManager)->stopRecordingDeviceTest();
}
int AgoraDeviceEngine::setPlaybackDevice(
    const char deviceId[DEVICE_MAX_NAME_LENGTH]) {
  return (*m_audioDeviceManager)->setPlaybackDevice(deviceId);
}
int AgoraDeviceEngine::startPlaybackDeviceTest(const char* testAudioFilePath) {
  return (*m_audioDeviceManager)->startPlaybackDeviceTest(testAudioFilePath);
}
int AgoraDeviceEngine::stopPlaybackDeviceTest() {
  return (*m_audioDeviceManager)->stopPlaybackDeviceTest();
}
int AgoraDeviceEngine::setDevice(int idx,
                                 const char deviceId[MAX_DEVICE_ID_LENGTH]) {
  if (idx == 0) {
    return (*m_videoDeviceManager)->setDevice(deviceId);
  }
  return m_vecEngine[idx - 1]->startPreview(deviceId);
}
int AgoraDeviceEngine::startDeviceTest(int idx, void* hwnd) {
  VideoCanvas_ vc;
  vc.renderMode = RENDER_MODE_FIT;
  vc.uid = 0;
  vc.view = hwnd;
  if (idx == 0) {
    agora::rtc::BeautyOptions options;
    options.lighteningContrastLevel =
        agora::rtc::BeautyOptions::LIGHTENING_CONTRAST_HIGH;
    options.lighteningLevel = 0.5;
    options.rednessLevel = 1.0;
    options.smoothnessLevel = 0.5;
    m_rtcEngine->setBeautyEffectOptions(true, options);
    m_rtcEngine->startPreview();
    return m_rtcEngine->setupLocalVideo(*(VideoCanvas*)&vc);
  }
  return m_vecEngine[idx - 1]->setupLocalVideo(vc, nullptr);
}
int AgoraDeviceEngine::stopDeviceTest(int idx) {
  if (idx == 0) return m_rtcEngine->stopPreview();
  return m_vecEngine[idx - 1]->stopPreview();
}
int AgoraDeviceEngine::setRecordingDeviceVolume(int volume) {
  return (*m_audioDeviceManager)->setRecordingDeviceVolume(volume);
}
int AgoraDeviceEngine::setPlaybackDeviceVolume(int volume) {
  return (*m_audioDeviceManager)->setPlaybackDeviceVolume(volume);
}
int AgoraDeviceEngine::registerEventHandler(
    IAgoraDeviceEventHandler* eventHandler) {
  m_eventHandler.setEventRecevier(eventHandler);
  return 0;
}

}  // namespace rtc
}  // namespace agora

AGORA_API agora::rtc::IAgoraVideoSourceEngine* AGORA_CALL
createAgoraVideoSourceEngine() {
  return new agora::rtc::AgoraVideoSourceEngine();
}

AGORA_API agora::rtc::IAgoraDeviceEngine* AGORA_CALL createAgoraDeviceEngine() {
  return new agora::rtc::AgoraDeviceEngine();
}
