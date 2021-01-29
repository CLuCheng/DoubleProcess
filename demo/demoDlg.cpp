
// demoDlg.cpp : implementation file
//
#include <sstream>

#include "stdafx.h"

#include "demo.h"

#include "afxdialogex.h"

#include <thread>
#include "demoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx {
 public:
  CAboutDlg();

  // Dialog Data
  enum { IDD = IDD_ABOUTBOX };

 protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

  // Implementation
 protected:
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) {}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CdemoDlg dialog

CdemoDlg::CdemoDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CdemoDlg::IDD, pParent) {
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  m_render = new D3DRender;
  m_remoteRender = new D3DRender;
}

void CdemoDlg::DoDataExchange(CDataExchange* pDX) {
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT_CHANNEL_NAME, m_edtChannel);

  DDX_Control(pDX, IDC_STATIC_REMOTE1, m_edtRemote1);
  DDX_Control(pDX, IDC_STATIC_LOCAL, m_localVideo);
  DDX_Control(pDX, IDC_BUTTON_LEAVE, m_btnLeave);
  DDX_Control(pDX, IDOK, m_btnJoin);
  DDX_Control(pDX, IDC_STATIC_LOCAL2, m_staLocalVideo);
}

BEGIN_MESSAGE_MAP(CdemoDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDOK, &CdemoDlg::OnBnClickedOk)

ON_STN_CLICKED(IDC_STATIC_LOCAL, &CdemoDlg::OnStnClickedStaticLocal)
ON_BN_CLICKED(IDC_BUTTON_LEAVE, &CdemoDlg::OnBnClickedButtonLeave)
ON_BN_CLICKED(IDC_BUTTON_STARTSCREEN, &CdemoDlg::OnBnClickedButtonStartscreen)
ON_BN_CLICKED(IDC_BUTTON_STOP_SCREEN, &CdemoDlg::OnBnClickedButtonStopScreen)
END_MESSAGE_MAP()

// CdemoDlg message handlers

BOOL CdemoDlg::OnInitDialog() {
  CDialogEx::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL) {
    BOOL bNameValid;
    CString strAboutMenu;
    bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
    ASSERT(bNameValid);
    if (!strAboutMenu.IsEmpty()) {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);   // Set big icon
  SetIcon(m_hIcon, FALSE);  // Set small icon

  m_btnLeave.EnableWindow(FALSE);
  // TODO: Add extra initialization here
  m_agoraVideoSourceEngine = createAgoraVideoSourceEngine();
  m_mainEngine = createAgoraVideoSourceEngine();
  m_deviceEngine = createAgoraDeviceEngine();
  m_deviceEngine->initialize(APP_ID,2);
  Device dc = {0};
  m_deviceEngine->enumeratePlaybackDevices(&dc);
  {
    std::stringstream ss;
    ss << "device test:" << dc.name[dc.count - 1] << "\n";
  }
  m_deviceEngine->enumerateRecordingDevices(&dc);
  {
    std::stringstream ss;
    ss << "device test:" << dc.name[dc.count - 1] << "\n";
    OutputDebugStringA(ss.str().c_str());
  }
  m_deviceEngine->enumerateVideoDevices(&dc);
  {
    std::stringstream ss;
    ss << "device test:" << dc.name[dc.count - 1] << "\n";
    OutputDebugStringA(ss.str().c_str());
  }
  //auto func =
  //    [=] {
  //      m_deviceEngine->setDevice(0, dc.id[dc.count - 1]);
  //      m_deviceEngine->startDeviceTest(0, m_localVideo.GetSafeHwnd());
  //      int i = 0;
		//while (i++<3)
		//{
  //        Sleep(5000);
  //        m_deviceEngine->stopDeviceTest(0);
  //        m_deviceEngine->setDevice(1, dc.id[dc.count - 1]);
  //        m_deviceEngine->startDeviceTest(1, m_localVideo.GetSafeHwnd());
  //        Sleep(5000);
  //        m_deviceEngine->stopDeviceTest(1);
  //        m_deviceEngine->setDevice(0, dc.id[dc.count - 1]);
  //        m_deviceEngine->startDeviceTest(0, m_localVideo.GetSafeHwnd());

		//}
  //  m_deviceEngine->release();
  //};

  //auto thread_ = new std::thread(func);
  //thread_->detach();
  VideoEncoderConfiguration_ config;

  //m_agoraVideoSourceEngine->initialize(APP_ID, NULL);
  //m_agoraVideoSourceEngine->setCustomRender(true);
  //m_agoraVideoSourceEngine->setChannelProfile(
  //    CHANNEL_PROFILE_LIVE_BROADCASTING_);  //Ö÷²¥
  //m_agoraVideoSourceEngine->joinChannel(APP_ID, "test1234", "", 12345);
  //m_agoraVideoSourceEngine->setVideoEncoderConfiguration(config);
  //m_agoraVideoSourceEngine->enableVideo(true);
  //m_agoraVideoSourceEngine->enableAudio(false);
  //m_agoraVideoSourceEngine->enableLocalVideo(false);
  //m_agoraVideoSourceEngine->enableLocalVideo(true);
  //m_agoraVideoSourceEngine->startPreview("");

  m_mainEngine->initialize(APP_ID, NULL);
  m_mainEngine->setCustomRender(true);
  m_mainEngine->enableHardWareVideoEncoding(true);

  m_mainEngine->setChannelProfile(CHANNEL_PROFILE_LIVE_BROADCASTING_);
  m_mainEngine->joinChannel(APP_ID, "test1234", "", 1234);
  m_mainEngine->enableVideo(true);
  m_mainEngine->enableAudio(true);
  m_mainEngine->enableLocalVideo(true);
  m_mainEngine->muteRemoteAudioStream(12345, false);
  m_mainEngine->muteRemoteVideoStream(12345, false);
  m_mainEngine->setDefaultMuteAllRemoteAudioStreams(true);
  m_mainEngine->setRemoteVideoStreamType(12345, REMOTE_VIDEO_STREAM_HIGH_);
  m_mainEngine->startPreview("");
  m_mainEngine->setVideoProfile(VIDEO_PROFILE_LANDSCAPE_1080P_, false);
  m_mainEngine->setVideoEncoderConfiguration(config);
  Rect_ rc = {0, 0, 1920, 1080};
  m_mainEngine->enableLocalVideo(true);
  m_mainEngine->startPreview("");
  m_mainEngine->startScreenCapture(::GetDesktopWindow(), 10, rc, 0);
  VideoCanvas_ cv;
  cv.uid = 1234;
  cv.renderMode = RENDER_MODE_FIT_;
  cv.view = m_localVideo.GetSafeHwnd();
  m_mainEngine->setVideoProfile(VIDEO_PROFILE_LANDSCAPE_1080P_, false);
  m_mainEngine->setVideoEncoderConfiguration(config);
  m_remoteRender->Init(m_localVideo.GetSafeHwnd(), 1920, 1080, true);

  m_mainEngine->setupLocalVideo(cv, [this](agora_video_frame* video_frame) {
    std::stringstream ss;
    ss << "xxx2: " << video_frame->width() << " x " << video_frame->height()
       << "\n";
    OutputDebugStringA(ss.str().c_str());
    agora_video_frame* c = nullptr;
    video_frame->copyFrame(&c);
    m_remoteRender->Render((char*)c->buffer(agora_video_frame::Y_PLANE));
    c->release();
  });
  // 000A0934
  // auto ptr = new std::thread([=] {
  //  Sleep(3000);
  //  m_mainEngine->muteLocalAudioStream(true);
  //  m_mainEngine->muteLocalVideoStream(true);
  //  m_mainEngine->setupLocalVideo(cv, nullptr);
  //  m_mainEngine->muteLocalVideoStream(true);
  //  m_mainEngine->startPreview("");
  //  m_mainEngine->setVideoProfile(VIDEO_PROFILE_LANDSCAPE_1080P_, false);
  //  m_mainEngine->setVideoEncoderConfiguration(config);
  //  m_mainEngine->startScreenCapture(::GetDesktopWindow(), 10, rc, 0);
  //  m_mainEngine->setupLocalVideo(cv, [this](agora_video_frame* video_frame) {
  //    std::stringstream ss;
  //    ss << "xxx2: " << video_frame->width() << " x " << video_frame->height()
  //       << "\n";
  //    OutputDebugStringA(ss.str().c_str());
  //    agora_video_frame* c = nullptr;
  //    video_frame->copyFrame(&c);
  //    m_remoteRender->Render((char*)c->buffer(agora_video_frame::Y_PLANE));
  //    c->release();
  //  });
  //  Sleep(3000);
  //  m_mainEngine->stopScreenCapture();
  //  ::ShowWindow((HWND)0x000307C0, SW_SHOW);
  //  OutputDebugStringA(
  //      "\n---------------------startScreenCapture------------------------\n");
  //  m_mainEngine->startScreenCapture((HWND)0x000307C0, 10, rc, 0);
  //});
  // ptr->detach();

  //bool init = false;
  //VideoCanvas_ cv_;
  //cv_.renderMode = RENDER_MODE_FIT_;
  //cv_.uid = 1234;
  //cv_.view = m_staLocalVideo.GetSafeHwnd();
  //m_agoraVideoSourceEngine->setupRemoteVideo(
  //    cv_, [=](agora_video_frame* video_frame) mutable {
  //      if (init == false) {
  //        m_render->Init(m_staLocalVideo.GetSafeHwnd(), video_frame->width(),
  //                       video_frame->height(), true);
  //        init = true;
  //      }
  //      std::stringstream ss;
  //      ss << "Remote: " << video_frame->width() << " x "
  //         << video_frame->height();
  //      OutputDebugStringA(ss.str().c_str());
  //      agora_video_frame* c = nullptr;
  //      video_frame->copyFrame(&c);
  //      m_render->Render((char*)c->buffer(agora_video_frame::Y_PLANE));
  //      c->release();
  //    });

  // main process rtc engine.
  // m_rtcEngine = createAgoraRtcEngine();
  // RtcEngineContext ctx;
  // ctx.appId = APP_ID;
  // ctx.eventHandler = new CDemoRtcEventHandler;
  // nRet = m_rtcEngine->initialize(ctx);
  // nRet = m_rtcEngine->enableVideo();
  // nRet = m_rtcEngine->disableAudio();
  ////set channel profile in the engine to the
  /// CHANNEL_PROFILE_LIVE_BROADCASTING.
  // nRet = m_rtcEngine->setChannelProfile(CHANNEL_PROFILE_LIVE_BROADCASTING);
  ////set client role in the engine to the CLIENT_ROLE_BROADCASTER.
  // nRet = m_rtcEngine->setClientRole(CLIENT_ROLE_BROADCASTER);
  // VideoCanvas canvas;
  // canvas.renderMode = RENDER_MODE_FIT;
  // canvas.uid = 0;
  // auto p = new CStatic();
  // RECT rc;
  // m_staLocalVideo.GetWindowRect(&rc);
  //       p->Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, rc,
  //                 (CWnd*)this);
  // p->ShowWindow(SW_SHOW);
  // canvas.view = p->GetSafeHwnd();
  ////setup local video in the engine to the canvas.
  // nRet = m_rtcEngine->setupLocalVideo(canvas);
  // nRet = m_rtcEngine->startPreview();
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CdemoDlg::OnSysCommand(UINT nID, LPARAM lParam) {
  if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  } else {
    CDialogEx::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CdemoDlg::OnPaint() {
  if (IsIconic()) {
    CPaintDC dc(this);  // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()),
                0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  } else {
    CDialogEx::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user
// drags
//  the minimized window.
HCURSOR CdemoDlg::OnQueryDragIcon() { return static_cast<HCURSOR>(m_hIcon); }

void CdemoDlg::OnBnClickedOk() {
  CString channel;
  m_edtChannel.GetWindowText(channel);
  channel.Trim();

  if (channel.IsEmpty()) {
    AfxMessageBox(_T("input channel name"));
    return;
  }

  VideoCanvas_ canvas;
  canvas.renderMode = RENDER_MODE_FIT_;
  canvas.view = m_localVideo.GetSafeHwnd();
  canvas.uid = 0;
  bool init = false;
  m_agoraVideoSourceEngine->setupLocalVideo(
      canvas, [=](agora_video_frame* video_frame) mutable {
        std::stringstream ss;
        ss << "width:" << video_frame->width()
           << "height:" << video_frame->height() << "\n";
        OutputDebugStringA(ss.str().c_str());
        agora_video_frame* c = nullptr;
        video_frame->copyFrame(&c);
        m_render->Render((char*)c->buffer(agora_video_frame::Y_PLANE));
        c->release();
      });
  m_agoraVideoSourceEngine->startPreview("0");

  char szChannel[MAX_PATH] = {0};
  WideCharToMultiByte(CP_UTF8, 0, channel.GetBuffer(0), -1, szChannel, MAX_PATH,
                      NULL, NULL);

  m_btnLeave.EnableWindow(TRUE);
  m_btnJoin.EnableWindow(FALSE);
  int nRet = m_agoraVideoSourceEngine->joinChannel(NULL, szChannel, NULL, 123);
  m_mainEngine->joinChannel(NULL, szChannel, NULL, 111);

  // TODO: Add your control notification handler code here
  // CDialogEx::OnOK();
}

void CdemoDlg::OnStnClickedStaticLocal() {
  // TODO: Add your control notification handler code here
}

void CdemoDlg::OnBnClickedButtonLeave() {
  VideoCanvas_ canvas;
  canvas.uid = 0;
  canvas.view = NULL;
  m_agoraVideoSourceEngine->setupLocalVideo(canvas, nullptr);

  m_agoraVideoSourceEngine->leaveChannel();
  m_agoraVideoSourceEngine->stopPreview();

  m_btnJoin.EnableWindow(TRUE);
  m_btnLeave.EnableWindow(FALSE);

  m_localVideo.Invalidate();
}

void CdemoDlg::OnBnClickedButtonStartscreen() {
  Rect_ rc = {0, 0, 1920, 1080};
  m_agoraVideoSourceEngine->startScreenCapture(::GetDesktopWindow(), 10, rc, 0);
  m_render->Init(m_localVideo.GetSafeHwnd(), 1920, 1080, true);
  m_agoraVideoSourceEngine->setVideoProfile(VIDEO_PROFILE_LANDSCAPE_1080P_,
                                            false);

  VideoCanvas_ cv;
  cv.renderMode = RENDER_MODE_FIT_;
  cv.uid = 111;
  auto p = new CStatic();
  RECT rc_;
  m_staLocalVideo.GetWindowRect(&rc_);
  p->Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, rc_, (CWnd*)this);
  p->ShowWindow(SW_SHOW);
  cv.view = p->GetSafeHwnd();
  m_remoteRender->Init(p->GetSafeHwnd(), 640, 360, true);
  m_agoraVideoSourceEngine->setupRemoteVideo(
      cv, [this](agora_video_frame* video_frame) {
        OutputDebugString(_T("RemoteVideo ... \n"));
        std::stringstream ss;
        ss << "xxx2: " << video_frame->width() << " x "
           << video_frame->height();
        OutputDebugStringA(ss.str().c_str());
        agora_video_frame* c = nullptr;
        video_frame->copyFrame(&c);
        m_remoteRender->Render((char*)c->buffer(agora_video_frame::Y_PLANE));
        c->release();
      });
}

void CdemoDlg::OnBnClickedButtonStopScreen() {
  // TODO: Add your control notification handler code here
  m_mainEngine->leaveChannel();
  m_mainEngine->release();
  m_agoraVideoSourceEngine->leaveChannel();
  m_agoraVideoSourceEngine->release();
}
