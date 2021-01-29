/*
 * Copyright (c) 2018 Agora.io
 * All rights reserved.
 * Proprietary and Confidential -- Agora.io
 */

/*
 *  Created by Wang Yongli, 2018
 */

#include "video_source_render.h"
#include "video_source.h"

AgoraVideoSourceRenderFactory::AgoraVideoSourceRenderFactory(
    AgoraVideoSource& videoSource)
    : m_videoSource(videoSource) {}

AgoraVideoSourceRenderFactory::~AgoraVideoSourceRenderFactory() {}

IExternalVideoRender* AgoraVideoSourceRenderFactory::createRenderInstance(
    const ExternalVideoRenerContext& context) {
  return new AgoraVideoSourceRender(m_videoSource, context.view,
                                    m_videoSource.isLocalview(context.view));
}

AgoraVideoSourceRender::AgoraVideoSourceRender(AgoraVideoSource& videoSource,void * view,bool is_local_view)
    : m_videoSource(videoSource) {
  m_view = view;
  m_transporter.reset(
      createAgoraVideoSourceTransporter(videoSource, view, is_local_view));
}

AgoraVideoSourceRender::~AgoraVideoSourceRender() {}

void AgoraVideoSourceRender::release() {
  m_transporter.reset();
  delete this;
}

int AgoraVideoSourceRender::initialize() {
  if (m_transporter->initialize()) {
    return 0;
  }
  return -1;
}

int AgoraVideoSourceRender::deliverFrame(const IVideoFrame& videoFrame,
                                         int rotation, bool mirrored) {
  return m_transporter->deliverFrame(videoFrame, rotation, mirrored);
}