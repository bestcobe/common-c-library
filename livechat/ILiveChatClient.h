/*
 * author: Samson.Fan
 *   date: 2015-03-19
 *   file: ILiveChatClient.h
 *   desc: LiveChat客户端接口类
 */

#pragma once

#include "ILiveChatClientDef.h"

// LiveChat客户端监听接口类
class ILiveChatClientListener
{
public:
	ILiveChatClientListener() {};
	virtual ~ILiveChatClientListener() {}

public:
	// 客户端主动请求
	// 回调函数的参数在 err 之前的为请求参数，在 errmsg 之后为返回参数
	virtual void OnLogin(LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnLogout(LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnSetStatus(LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnEndTalk(const string& inUserId, LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnGetUserStatus(const UserIdList& inList, LCC_ERR_TYPE err, const string& errmsg, const UserStatusList& list) = 0;
	virtual void OnGetTalkInfo(const string& inUserId, LCC_ERR_TYPE err, const string& errmsg, const string& userId, const string& invitedId, bool charge, unsigned int chatTime) = 0;
	virtual void OnSendMessage(const string& inUserId, const string& inMessage, int inTicket, LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnSendEmotion(const string& inUserId, const string& inEmotionId, int inTicket, LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnSendVGift(const string& inUserId, const string& inGiftId, int inTicket, LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnGetVoiceCode(const string& inUserId, int ticket, LCC_ERR_TYPE err, const string& errmsg, const string& voiceCode) = 0;
	virtual void OnSendVoice(const string& inUserId, const string& inVoiceId, int inTicket, LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnUseTryTicket(const string& inUserId, LCC_ERR_TYPE err, const string& errmsg, const string& userId, TRY_TICKET_EVENT tickEvent) = 0;
	virtual void OnGetTalkList(int inListType, LCC_ERR_TYPE err, const string& errmsg, const TalkListInfo& talkListInfo) = 0;
	virtual void OnSendPhoto(LCC_ERR_TYPE err, const string& errmsg, int ticket) = 0;
	virtual void OnShowPhoto(LCC_ERR_TYPE err, const string& errmsg, int ticket) = 0;
	virtual void OnGetUserInfo(const string& inUserId, LCC_ERR_TYPE err, const string& errmsg, const UserInfoItem& userInfo) = 0;
	virtual void OnGetUsersInfo(LCC_ERR_TYPE err, const string& errmsg, const UserInfoList& userList) = 0;
	virtual void OnGetContactList(CONTACT_LIST_TYPE inListType, LCC_ERR_TYPE err, const string& errmsg, const TalkUserList& list) = 0;
	virtual void OnGetBlockUsers(LCC_ERR_TYPE err, const string& errmsg, const list<string>& users) = 0;
	virtual void OnGetRecentContact(LCC_ERR_TYPE err, const string& errmsg, const list<string>& userList) = 0;
	virtual void OnSearchOnlineMan(LCC_ERR_TYPE err, const string& errmsg, const list<string>& userList) = 0;
	virtual void OnReplyIdentifyCode(LCC_ERR_TYPE err, const string& errmsg) = 0;
	virtual void OnGetRecentContactList(LCC_ERR_TYPE err, const string& errmsg, const list<string>& userList) = 0;
	virtual void OnGetFeeRecentContactList(LCC_ERR_TYPE err, const string& errmsg, const list<string>& userList) = 0;
	virtual void OnGetLadyChatInfo(LCC_ERR_TYPE err, const string& errmsg, const list<string>& chattingList, const list<string>& chattingInviteIdList, const list<string>& missingList, const list<string>& missingInviteIdList) = 0;
	// 服务器主动请求
	virtual void OnRecvMessage(const string& toId, const string& fromId, const string& fromName, const string& inviteId, bool charge, int ticket, TALK_MSG_TYPE msgType, const string& message) = 0;
	virtual void OnRecvEmotion(const string& toId, const string& fromId, const string& fromName, const string& inviteId, bool charge, int ticket, TALK_MSG_TYPE msgType, const string& emotionId) = 0;
	virtual void OnRecvVoice(const string& toId, const string& fromId, const string& fromName, const string& inviteId, bool charge, TALK_MSG_TYPE msgType, const string& voiceId, const string& fileType, int timeLen) = 0;
	virtual void OnRecvWarning(const string& toId, const string& fromId, const string& fromName, const string& inviteId, bool charge, int ticket, TALK_MSG_TYPE msgType, const string& message) = 0;
	virtual void OnUpdateStatus(const string& userId, const string& server, CLIENT_TYPE clientType, USER_STATUS_TYPE statusType) = 0;
	virtual void OnUpdateTicket(const string& fromId, int ticket) = 0;
	virtual void OnRecvEditMsg(const string& fromId) = 0;
	virtual void OnRecvTalkEvent(const string& userId, TALK_EVENT_TYPE eventType) = 0;
	virtual void OnRecvTryTalkBegin(const string& toId, const string& fromId, int time) = 0;
	virtual void OnRecvTryTalkEnd(const string& userId) = 0;
	virtual void OnRecvEMFNotice(const string& fromId, TALK_EMF_NOTICE_TYPE noticeType) = 0;
	virtual void OnRecvKickOffline(KICK_OFFLINE_TYPE kickType) = 0;
	virtual void OnRecvPhoto(const string& toId, const string& fromId, const string& fromName, const string& inviteId, const string& photoId, const string& sendId, bool charge, const string& photoDesc, int ticket) = 0;
	virtual void OnRecvLadyVoiceCode(const string& voiceCode) = 0;
	virtual void OnRecvIdentifyCode(const unsigned char* data, long dataLen) = 0;
};

// LiveChat客户端接口类
class ILiveChatClient
{
public:
	static ILiveChatClient* CreateClient();
	static void ReleaseClient(ILiveChatClient* client);

public:
	ILiveChatClient(void) {};
	virtual ~ILiveChatClient(void) {};

public:
	// 调用所有接口函数前需要先调用Init
	virtual bool Init(const list<string>& svrIPs, unsigned int svrPort, ILiveChatClientListener* listener) = 0;
	// 登录
	virtual bool Login(const string& user, const string& password, const string& deviceId, CLIENT_TYPE clientType, USER_SEX_TYPE sexType) = 0;
	// 注销
	virtual bool Logout() = 0;
	// 设置在线状态
	virtual bool SetStatus(USER_STATUS_TYPE status) = 0;
	// 结束聊天
	virtual bool EndTalk(const string& userId) = 0;
	// 获取用户在线状态
	virtual bool GetUserStatus(const UserIdList& list) = 0;
	// 获取会话信息
	virtual bool GetTalkInfo(const string& userId) = 0;
	// 上传票根
	virtual bool UploadTicket(const string& userId, int ticket) = 0;
	// 通知对方女士正在编辑消息
	virtual bool SendLadyEditingMsg(const string& userId) = 0;
	// 发送聊天消息
	virtual bool SendMessage(const string& userId, const string& message, bool illegal, int ticket) = 0;
	// 发送高级表情
	virtual bool SendEmotion(const string& userId, const string& emotionId, int ticket) = 0;
	// 发送虚拟礼物
	virtual bool SendVGift(const string& userId, const string& giftId, int ticket) = 0;
	// 获取语音发送验证码
	virtual bool GetVoiceCode(const string& userId, int ticket) = 0;
	// 获取女士语音发送验证码
	virtual bool GetLadyVoiceCode(const string& userId) = 0;
	// 发送语音
	virtual bool SendVoice(const string& userId, const string& voiceId, int length, int ticket) = 0;
	// 使用试聊券
	virtual bool UseTryTicket(const string& userId) = 0;
	// 获取邀请列表或在聊列表
	virtual bool GetTalkList(int listType) = 0;
	// 发送图片
	virtual bool SendPhoto(const string& userId, const string& inviteId, const string& photoId, const string& sendId, bool charge, const string& photoDesc, int ticket) = 0;
	// 显示图片
	virtual bool ShowPhoto(const string& userId, const string& inviteId, const string& photoId, const string& sendId, bool charge, const string& photoDesc, int ticket) = 0;
	// 获取用户信息
	virtual bool GetUserInfo(const string& userId) = 0;
	// 获取多个用户信息
	virtual bool GetUsersInfo(const list<string>& userIdList) = 0;
	// 获取联系人/黑名单列表
	virtual bool GetContactList(CONTACT_LIST_TYPE listType) = 0;
	// 上传客户端版本号
	virtual bool UploadVer(const string& ver) = 0;
	// 获取被屏蔽女士列表
	virtual bool GetBlockUsers() = 0;
	// 获取最近联系人列表
	virtual bool GetRecentContactList() = 0;
	// 搜索在线男士
	virtual bool SearchOnlineMan(int beginAge, int endAge) = 0;
	// 回复验证码
	virtual bool ReplyIdentifyCode(string identifyCode) = 0;
	// 刷新验证码
	virtual bool RefreshIdentifyCode() = 0;
	// 刷新邀请模板
	virtual bool RefreshInviteTemplate() = 0;
    // 获取已扣费联系人列表
	virtual bool GetFeeRecentContactList() = 0;
	// 获取女士聊天信息（包括在聊及邀请的男士列表等）
	virtual bool GetLadyChatInfo() = 0;

public:
	// 获取用户账号
	virtual string GetUser() = 0;
};