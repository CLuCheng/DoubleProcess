
// demoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "D3DRender.h"
//class VideoEvntHandler:public IVideoSourceEventHandler
//{
//	/**
//	* Video source joined channel success event.
//	* @param uid : video source's uid.
//	*/
//	virtual void onVideoSourceJoinedChannelSuccess(agora::rtc::uid_t uid)
//	{
//
//	}
//
//	/**
//	* Video source request new token event.
//	*/
//	virtual void onVideoSourceRequestNewToken() {
//
//	}
//
//	/**
//	* Video source leaved channel event.
//	*/
//	virtual void onVideoSourceLeaveChannel()
//	{
//
//	}
//
//	virtual void onVideoSourceExit()
//	{
//
//	}
//	virtual void onVideoSourceFirstVideoFrame(int width, int height, int elapsed)
//	{
//
//	}
//};


//class CDemoRtcEventHandler
//	: public IRtcEngineEventHandler
//{
//public:
//	//set the message notify window handler
//	void SetMsgReceiver(HWND hWnd) { m_hMsgHanlder = hWnd; }
//	virtual void onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed) override{
//
//	}
//	virtual void onUserJoined(uid_t uid, int elapsed) override
//	{
//
//	}
//	virtual void onUserOffline(uid_t uid, USER_OFFLINE_REASON_TYPE reason) override
//	{
//
//	}
//	virtual void onLeaveChannel(const RtcStats& stats) override
//	{
//	}
//private:
//	HWND m_hMsgHanlder;
//};

class AgoraDeviceEventHandler : public rtc::IAgoraDeviceEventHandler {
 public:
	 virtual void onAudioVolumeIndication(const AudioVolumeInfo_* speakers,
		 unsigned int speakerNumber,
		 int totalVolume)override
	 {
    CString str;
    str.Format(_T("%d\n"), totalVolume);
    OutputDebugString(str);
   }
};


// CdemoDlg dialog
class CdemoDlg : public CDialogEx
{
// Construction
public:
	CdemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_edtChannel;
private:
	IAgoraVideoSourceEngine* m_agoraVideoSourceEngine;
    IAgoraVideoSourceEngine* m_mainEngine;
    IAgoraDeviceEngine* m_deviceEngine;
    AgoraDeviceEventHandler m_deviceEventHnalder;
	D3DRender * m_render;
    D3DRender* m_remoteRender;

   public:
	//IRtcEngine* m_rtcEngine = nullptr;

	CStatic m_edtRemote1;
	CStatic m_localVideo;
	afx_msg void OnStnClickedStaticLocal();
	CButton m_btnLeave;
	afx_msg void OnBnClickedButtonLeave();
	CButton m_btnJoin;
	afx_msg void OnBnClickedButtonStartscreen();
	afx_msg void OnBnClickedButtonStopScreen();
	CStatic m_staLocalVideo;
};
