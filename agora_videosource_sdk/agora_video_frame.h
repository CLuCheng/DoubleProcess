#pragma once
#include <memory>
#include "IAgoraMediaEngine.h"
struct image_frame_info {
  int stride;
  int stride0;
  int width;
  int height;
  int strideU;
  int strideV;
};

struct image_header_type {
  unsigned char format;
  unsigned char mirrored;
  unsigned short width;
  unsigned short height;
  unsigned short left;
  unsigned short top;
  unsigned short right;
  unsigned short bottom;
  unsigned short rotation;
  unsigned int timestamp;
};

class agora_video_frame : public agora::media::IVideoFrame {
 public:
  agora_video_frame() {}
  agora_video_frame(unsigned short width, unsigned short height,
                    unsigned char* stride_y, int stride_y_size,
                    unsigned char* stride_u, int stride_u_size,
                    unsigned char* stride_v, int sride_v_size,
                    unsigned int timestamp)
      : width_(width),
        height_(height),
        stride_y_size_(stride_y_size),
        stride_u_size_(stride_u_size),
        stride_v_size_(sride_v_size),
        timestamp_(timestamp)

  {
    int size = stride_y_size_ * 1.5 * height_;
    auto buf = new unsigned char[size];
    std::memcpy(buf, stride_y, size);
    buffer_.reset(buf);
  }

  agora_video_frame(const agora_video_frame& lrs)
      : width_(lrs.width_),
        height_(lrs.height_),
        stride_y_size_(lrs.stride_y_size_),
        stride_u_size_(lrs.stride_u_size_),
        stride_v_size_(lrs.stride_v_size_),
        timestamp_(lrs.timestamp_) {
    buffer_ = lrs.buffer_;
  }
  virtual VIDEO_TYPE GetVideoType() const override { return VIDEO_TYPE_YV12; }
  virtual void release() override { buffer_.reset(); }
  /*!
   * @brief
   * @author LuCheng
   * @date   2020/11/5
   * @param  PLANE_TYPE type
   * @return     const unsigned char*
   */
  virtual const unsigned char* buffer(PLANE_TYPE type) const override {
    switch (type) {
      case Y_PLANE:
        return buffer_.get();
      case U_PLANE:
        return buffer_.get() + stride_y_size_ * height_;
      case V_PLANE:
        return buffer_.get() + stride_y_size_ * height_ +
               stride_u_size_ / 2 * height_;
    }
    return nullptr;
  }

  // Copy frame: If required size is bigger than allocated one, new buffers of
  // adequate size will be allocated.
  // Return value: 0 on success ,-1 on error.
  virtual int copyFrame(IVideoFrame** dest_frame) const override {
    if (!dest_frame) return -1;
    if (!*dest_frame) {
      *dest_frame =
          new agora_video_frame(width_, height_, buffer_.get(), stride_y_size_,
                                buffer_.get() + stride_y_size_, stride_u_size_,
                                buffer_.get() + stride_y_size_ + stride_u_size_,
                                stride_v_size_, timestamp_);
      return 0;
    }

    agora_video_frame* video_frame =
        dynamic_cast<agora_video_frame*>(*dest_frame);
    if (video_frame->allocated_size(IVideoFrame::Y_PLANE) >
        this->allocated_size(IVideoFrame::Y_PLANE)) {
      video_frame->release();
      video_frame = new agora_video_frame(*this);
      return 0;
    }
    video_frame->stride_y_size_ = stride_y_size_;
    video_frame->stride_u_size_ = stride_u_size_;
    video_frame->stride_v_size_ = stride_v_size_;
    int size = stride_y_size_ * 1.5 * height_;
    auto buf = new unsigned char[size];
    std::memcpy(buf, buffer_.get(), size);
    video_frame->buffer_.reset(buf);
    return 0;
  }

  // Convert frame
  // Input:
  //   - src_frame        : Reference to a source frame.
  //   - dst_video_type   : Type of output video.
  //   - dst_sample_size  : Required only for the parsing of MJPG.
  //   - dst_frame        : Pointer to a destination frame.
  // Return value: 0 if OK, < 0 otherwise.
  // It is assumed that source and destination have equal height.
  virtual int convertFrame(VIDEO_TYPE dst_video_type, int dst_sample_size,
                           unsigned char* dst_frame) const override {
    return -1;
  }

  // Get allocated size per plane.
  virtual int allocated_size(PLANE_TYPE type) const override {
    switch (type) {
      case IVideoFrame::Y_PLANE:
        return stride_y_size_ * height_;
      case IVideoFrame::U_PLANE:
        return stride_u_size_ * height_ / 2;
      case IVideoFrame::V_PLANE:
        return stride_v_size_ * height_ / 2;
    }
    return height_ * width_ * 1.5;
  }

  // Get allocated stride per plane.
  virtual int stride(PLANE_TYPE type) const override {
    return allocated_size(type);
  }

  // Get frame width.
  virtual int width() const override { return width_; }

  // Get frame height.
  virtual int height() const override { return height_; }
  // Get frame timestamp (90kHz).
  virtual unsigned int timestamp() const override { return 0; }

  // Get render time in milliseconds.
  virtual int64_t render_time_ms() const override { return timestamp_; }

  // Return true if underlying plane buffers are of zero size, false if not.
  virtual bool IsZeroSize() const override { return !buffer_.get(); }

 private:
  int width_;
  int height_;
  std::shared_ptr<unsigned char> buffer_;
  int stride_y_size_;
  int stride_u_size_;
  int stride_v_size_;
  int timestamp_;
};

/*!
 * \class parser_video_frame
 *
 * \ingroup GroupName
 *
 * \brief
 *
 * TODO: long description
 *
 * \note
 *
 * \author LC
 *
 * \version 1.0
 *
 * \date Ê®Ò»ÔÂ 2020
 *
 * Contact: lucheng@agora.io
 *
 */
class parser_video_frame {
 public:
  std::shared_ptr<agora::media::IVideoFrame> gen_video_frame(unsigned char* buf,
                                                             int len) {
    if (!buf) return std::make_shared<agora_video_frame>();
    image_frame_info* info = reinterpret_cast<image_frame_info*>(buf);
    image_header_type* header =
        reinterpret_cast<image_header_type*>(buf + sizeof(image_frame_info));
    unsigned char* y_stride =
        buf + sizeof(image_frame_info) + sizeof(image_header_type);
    return std::make_shared<agora_video_frame>(
        header->width, header->height, y_stride, info->stride0,
        y_stride + info->stride0, info->strideU,
        y_stride + info->stride0 + info->strideU, info->strideV,
        header->timestamp);
  }
};