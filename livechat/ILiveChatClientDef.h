/*
 * author: Samson.Fan
 *   date: 2015-03-19
 *   file: ILiveChatClient.h
 *   desc: LiveChat客户端接口类
 */

#pragma once

#include <string>
#include <list>
#include <vector>
#include <ProtocolCommon/DeviceTypeDef.h>

using namespace std;

// 处理结果
typedef enum {
	LCC_ERR_SUCCESS					= 0,		// 成功
	LCC_ERR_FAIL					= -10000,	// 服务器返回失败结果
	
	// 服务器返回错误
	LCC_ERR_UNBINDINTER				= -1,		// 女士的翻译未将其绑定
	LCC_ERR_SIDEOFFLINE				= -2,		// 对方不在线（聊天）
	LCC_ERR_DUPLICATECHAT			= -3,		// 聊天状态已经建立
	LCC_ERR_NOMONEY					= -4,		// 帐号余额不足
	LCC_ERR_INVALIDUSER				= -5,		// 用户不存在（登录）
	LCC_ERR_TARGETNOTEXIST			= -6,		// 目标用户不存在
	LCC_ERR_INVAILDPWD				= -7,		// 密码错误（登录）
	LCC_ERR_NOTRANSORAGENTFOUND		= -8,		// 没有找到翻译或机构
	LCC_ERR_NOPERMISSION			= -9,		// 没有权限
	LCC_ERR_TRANSINFOCHANGE			= -10,		// 翻译信息改变
	LCC_ERR_CHATTARGETREJECT		= -11,		// 对方拒绝
	LCC_ERR_MAXBINDING				= -12,		// 超过最大连接数
	LCC_ERR_COMMANDREJECT			= -13,		// 请求被拒绝
	LCC_ERR_BLOCKUSER				= -14,		// 对方为黑名单用户（聊天）
	LCC_ERR_IDENTIFYCODE			= -15,		// 需要验证码
	LCC_ERR_SOCKETNOTEXIST			= -16,		// socket不存在
	LCC_ERR_EMOTIONERR				= -17,		// 高级表情异常（聊天）
	LCC_ERR_FREQUENCYEMOTION		= -18,		// 高级表情发送过快（聊天）
	LCC_ERR_WARNTIMES				= -19,		// 严重警告
	LCC_ERR_PHOTOERR				= -20,		// 图片异常（聊天）
	LCC_ERR_WOMANCHATLIMIT			= -21,		// 女士超过聊天上限
	LCC_ERR_FREQUENCYVOICE			= -22,		// 语音发送过快（聊天）
	LCC_ERR_MICVIDEO				= -23,		// 不允许发送视频
	LCC_ERR_VOICEERR				= -24,		// 语音异常
	LCC_ERR_NOSESSION				= -25,		// session错误
	LCC_ERR_FREQUENCYMAGICICON		= -26,		// 小高表发送过快
	LCC_ERR_MAGICICONERR			= -27,		// 小高表异常
	LCC_ERR_WOMANACTIVECHATLIMIT	= -28,		// 女士发送邀请过快
	LCC_ERR_SUBJECTEXCEPTION		= -29,		// 主题异常
	LCC_ERR_SUBJECTEXISTEXCEPTION	= -30,		// 主题存在异常
	LCC_ERR_SUBJECTNOTEXISTEXCEPTION= -31,		// 主题不存在异常
	
	// 客户端定义的错误
	LCC_ERR_PROTOCOLFAIL	= -10001,	// 协议解析失败（服务器返回的格式不正确）
	LCC_ERR_CONNECTFAIL		= -10002,	// 连接服务器失败/断开连接
	LCC_ERR_CHECKVERFAIL	= -10003,	// 检测版本号失败（可能由于版本过低导致）
	LCC_ERR_LOGINFAIL		= -10004,	// 登录失败
	LCC_ERR_SVRBREAK		= -10005,	// 服务器踢下线
	LCC_ERR_SETOFFLINE		= -10006,	// 不能把在线状态设为"离线"，"离线"请使用Logout()
} LCC_ERR_TYPE;

// 认证类型
typedef enum {
	AUTH_TYPE_SID = 0,		// sid认证
	AUTH_TYPE_PWD = 1,		// 密码认证
} AUTH_TYPE;

// 用户性别
typedef enum {
	USER_SEX_FEMALE	= 0,	// 女士
	USER_SEX_MALE	= 1,	// 男士
	USER_SEX_UNKNOW,		// 未知
	USER_SEX_BEGIN	= USER_SEX_FEMALE,	// 有效范围起始值
	USER_SEX_END	= USER_SEX_UNKNOW,	// 有效范围结束值
} USER_SEX_TYPE;
// int 转换 USER_SEX_TYPE
inline USER_SEX_TYPE GetUserSexType(int value) {
	return USER_SEX_BEGIN <= value && value < USER_SEX_UNKNOW ? (USER_SEX_TYPE)value : USER_SEX_UNKNOW;
}

// 客户端类型
typedef enum {
	CLIENT_PC		= 0,	// PC端
	CLIENT_PC_JAVA	= 1,	// PC Java端
	CLIENT_PC_APP	= 2,	// PC app端
	CLIENT_PC_PAD	= 3,	// PC pad端
	CLIENT_ANDROID	= 5,	// Android客户端
	CLIENT_IPHONE	= 6,	// IPhone客户端
	CLIENT_IPAD		= 7,	// IPad客户端
	CLIENT_WEB		= 8,	// Web端
	CLIENT_UNKNOW,			// 未知
	CLIENT_BEGIN = CLIENT_PC,	// 有效范围起始值
	CLIENT_END = CLIENT_UNKNOW,	// 有效范围结束值
} CLIENT_TYPE;
// int 转换 CLIENT_TYPE
inline CLIENT_TYPE GetClientType(int value) {
	return CLIENT_BEGIN <= value && value < CLIENT_END ? (CLIENT_TYPE)value : CLIENT_UNKNOW;
}
// CLIENT_TYPE 转换 TDEVICE_TYPE
inline TDEVICE_TYPE GetDeviceTypeWithClientType(CLIENT_TYPE clientType) {
    TDEVICE_TYPE deviceType = DEVICE_TYPE_UNKNOW;
    switch (clientType) {
    case CLIENT_PC:
        deviceType = DEVICE_TYPE_APP_PC;
        break;
    case CLIENT_PC_JAVA:
        deviceType = DEVICE_TYPE_WAP;
        break;
    case CLIENT_PC_APP:
        deviceType = DEVICE_TYPE_WAP_ANDROID;
        break;
    case CLIENT_PC_PAD:
        deviceType = DEVICE_TYPE_APP_IPAD;
        break;
    case CLIENT_ANDROID:
        deviceType = DEVICE_TYPE_APP_ANDROID;
        break;
    case CLIENT_IPHONE:
        deviceType = DEVICE_TYPE_APP_IPHONE;
        break;
    case CLIENT_IPAD:
        deviceType = DEVICE_TYPE_APP_IPAD;
        break;
    case CLIENT_WEB:
        deviceType = DEVICE_TYPE_WEB;
        break;
    default:
        break;
    }
    return deviceType;
}

// 用户在线状态
typedef enum {
	USTATUSPRO_OFFLINE_OR_HIDDEN	= 0,	// 离线/隐身
	USTATUSPRO_ONLINE	= 1,				// 在线
	USTATUSPRO_HIDDEN	= 2,				// 隐身
	USTATUSPRO_BIND = 3,					// 绑定翻译
	USTATUSPRO_UNBIND = 4,					// 没有绑定翻译
	USTATUSPRO_VIDEOOPEN = 5,				// 视频开放
	USTATUSPRO_VIDEOCLOSE = 6,				// 视频关闭
	USTATUSPRO_UNKNOW,			// 未知
	USTATUSPRO_BEGIN = USTATUSPRO_OFFLINE_OR_HIDDEN,	// 有效范围起始值
	USTATUSPRO_END = USTATUSPRO_UNKNOW,	// 有效范围结束值
} USER_STATUS_PROTOCOL;
typedef enum {
	USTATUS_OFFLINE_OR_HIDDEN	= 0,	// 离线/隐身
	USTATUS_ONLINE	= 1,				// 在线
	USTATUS_HIDDEN	= 2,				// 隐身
	USTATUS_UNKNOW,			// 未知
	USTATUS_BEGIN = USTATUS_OFFLINE_OR_HIDDEN,	// 有效范围起始值
	USTATUS_END = USTATUS_UNKNOW,	// 有效范围结束值
} USER_STATUS_TYPE;
// int 转换 USER_STATUS_TYPE
inline USER_STATUS_TYPE GetUserStatusType(int value) {
	USER_STATUS_TYPE statusType = USTATUS_UNKNOW;
	switch (value)
	{
	case USTATUSPRO_OFFLINE_OR_HIDDEN:
	case USTATUSPRO_ONLINE:
	case USTATUSPRO_HIDDEN:
		statusType = (USER_STATUS_TYPE)value;
		break;
	case USTATUSPRO_BIND:
	case USTATUSPRO_VIDEOOPEN:
	case USTATUSPRO_VIDEOCLOSE:
		statusType = USTATUS_ONLINE;
		break;
	case USTATUSPRO_UNBIND:
		statusType = USTATUS_OFFLINE_OR_HIDDEN;
		break;
	}
	return statusType;
}

// 试聊事件
typedef enum {
	TTE_NORMAL		= 0,	// 正常使用
	TTE_USED		= 1,	// 已使用券
	TTE_PAID		= 2,	// 已付费
	TTE_NOTICKET	= 3,	// 没有券
	TTE_OFFLINE		= 4,	// 对方已离线
	TTE_UNKNOW,				// 未知
	TTE_BEGIN = TTE_NORMAL,	// 有效范围起始值
	TTE_END = TTE_UNKNOW,	// 有效范围结束值
} TRY_TICKET_EVENT;
// int 转换 TRY_TICKET_EVENT
inline TRY_TICKET_EVENT GetTryTicketEvent(int value) {
	return TTE_BEGIN <= value && value < TTE_END ? (TRY_TICKET_EVENT)value : TTE_UNKNOW;
}

// 聊天消息类型
typedef enum {
	TMT_FREE		= 0,		// 免费
	TMT_CHARGE		= 1,		// 收费
	TMT_FREE1		= 2,		// 免费1(已废弃)
	TMT_FREE2		= 3,		// 免费2(已废弃)
	TMT_FREE3		= 4,		// 免费3(已废弃)
	TMT_CHARGE_FREE	= 5,		// 试聊券
	TMT_UNKNOW,				// 未知
	TMT_BEGIN = TMT_FREE,	// 有效范围起始值
	TMT_END = TMT_UNKNOW,	// 有效范围结束值
} TALK_MSG_TYPE;
// int 转换 TALK_MSG_TYPE
inline TALK_MSG_TYPE GetTalkMsgType(int value) {
	TALK_MSG_TYPE msgType = TMT_BEGIN <= value && value < TMT_END ? (TALK_MSG_TYPE)value : TMT_UNKNOW;
	if (TMT_FREE1 == msgType
		|| TMT_FREE2 == msgType
		|| TMT_FREE3 == msgType)
	{
		// 把已废弃的免费类型转换为TMT_FREE
		msgType = TMT_FREE;
	}
	return msgType;
}

// 聊天事件类型
typedef enum {
	TET_ENDTALK			= 0,	// 结束聊天
	TET_STARTCHARGE		= 1,	// 开始收费
	TET_STOPCHARGE		= 2,	// 暂停收费
	TET_NOMONEY			= 3,	// 余额不足
	TET_VIDEONOMONEY	= 4,	// 视频余额不足
	TET_TARGETNOTFOUND	= 5,	// 目标不存在
	TET_UNKNOW,					// 未知
	TET_BEGIN = TET_ENDTALK,	// 有效范围起始值
	TET_END = TET_UNKNOW,		// 有效范围结束值
} TALK_EVENT_TYPE;
// int 转换 TALK_EVENT_TYPE
inline TALK_EVENT_TYPE GetTalkEventType(int value) {
	return TET_BEGIN <= value && value < TET_END ? (TALK_EVENT_TYPE)value : TET_UNKNOW;
}

// 邮件更新通知类型
typedef enum {
	TENT_EMF		= 0,	// EMF邮件
	TENT_ADMIRER	= 1,	// Admirer邮件
	TENT_UNKNOW,			// 未知
	TENT_BEGIN = TENT_EMF,	// 有效范围起始值
	TENT_END = TENT_UNKNOW,	// 有效范围结束值
} TALK_EMF_NOTICE_TYPE;
// int 转换 TALK_EMF_NOTICE_TYPE
inline TALK_EMF_NOTICE_TYPE GetTalkEmfNoticeType(int value) {
	return TENT_BEGIN <= value && value < TENT_END ? (TALK_EMF_NOTICE_TYPE)value : TENT_UNKNOW;
}

// 服务器踢下线类型
typedef enum {
	KOT_UNKNOW		= -1,	// 未知
	KOT_MAINTAIN	= 1,	// 服务器维护
	KOT_TIMEOUT		= 2,	// 心跳包超时
	KOT_OTHER_LOGIN	= 4,	// 用户在其它地方登录
} KICK_OFFLINE_TYPE;

typedef list<string> UserIdList;	// 用户 id list

// 用户在线状态item
typedef struct _tagUserStatusItem
{
	USER_STATUS_TYPE	statusType;	// 用户在线状态
	string				userId;		// 用户Id
	
	_tagUserStatusItem()  {
		Reset();
	}
	_tagUserStatusItem(const _tagUserStatusItem& item) {
		statusType = item.statusType;
		userId = item.userId;
	}
	void Reset() {
		statusType = USTATUS_OFFLINE_OR_HIDDEN;
		userId = "";
	}
} UserStatusItem;
typedef list<UserStatusItem> UserStatusList;

// 婚姻类型
typedef enum {
	MT_UNKNOW = 0,			// 未知
	MT_NEVERMARRIED = 1,	// 从来未婚
	MT_DIVORCED = 2,		// 离婚
	MT_WIDOWED = 3,			// 丧偶
	MT_SEPARATED = 4,		// 分居
	MT_MARRIED = 5,			// 已婚
	MT_BEGIN = MT_UNKNOW,	// 有效范围起始值
	MT_END = MT_MARRIED,	// 有效范围结束值
} MARRY_TYPE;
// int 转换 MARRY_TYPE
inline MARRY_TYPE GetMarryType(int value) {
	return MT_BEGIN <= value && value < MT_END ? (MARRY_TYPE)value : MT_UNKNOW;
}

// 子女类型
typedef enum {
	CT_NO = 0,		// 没有子女
	CT_YES = 1,		// 有子女
	CT_UNKNOW,		// 未知
} CHILDREN_TYPE;

// 用户类型
typedef enum {
	UT_WOMAN = 0,		// 女士
	UT_MAN = 1,			// 男士
	UT_TRANS = 2,		// 翻译
	UT_UNKNOW,			// 未知
	UT_BEGIN = UT_WOMAN,	// 有效范围起始值
	UT_END = UT_UNKNOW,		// 有效范围结束值
} USER_TYPE;
// int 转换 USER_TYPE
inline USER_TYPE GetUserType(int value) {
	return UT_BEGIN <= value && value < UT_END ? (USER_TYPE)value : UT_UNKNOW;
}

// 邀请/在聊user列表item
typedef struct _tagUserInfoItem {
	string		userId;			// 用户ID
	string		userName;		// 用户名
	string		server;			// 服务器名
	string		imgUrl;			// 头像URL
	USER_SEX_TYPE	sexType;	// 性别
	int			age;			// 年龄
	string		weight;			// 体重
	string		height;			// 身高
	string		country;		// 国家
	string		province;		// 省份
	bool		videoChat;		// 是否能视频聊天
	int			videoCount;		// 视频数量
	MARRY_TYPE	marryType;		// 婚姻状况
	CHILDREN_TYPE childrenType;	// 子女状况
	USER_STATUS_TYPE	status;	// 用户在线状态
	USER_TYPE	userType;		// 用户类型
	int			orderValue;		// 排序分值
	TDEVICE_TYPE	deviceType;		// 设备类型
	CLIENT_TYPE	clientType;		// 客户端类型
	string		clientVersion;	// 客户端版本号
	// 以下仅女士端获取自己信息才有
	bool		needTrans;		// 是否需要翻译
	string		transUserId;	// 翻译ID
	string		transUserName;	// 翻译名称
	bool		transBind;		// 是否绑定翻译
	USER_STATUS_TYPE	transStatus;	// 翻译在线状态

	_tagUserInfoItem() {
		userId = "";
		userName = "";
		server = "";
		imgUrl = "";
		sexType = USER_SEX_UNKNOW;
		age = 0;
		weight = "";
		height = "";
		country = "";
		province = "";
		videoChat = false;
		videoCount = 0;
		marryType = MT_UNKNOW;
		childrenType = CT_UNKNOW;
		status = USTATUS_OFFLINE_OR_HIDDEN;
		userType = UT_MAN;
		orderValue = 0;
		deviceType = DEVICE_TYPE_UNKNOW;
		clientType = CLIENT_UNKNOW;
		clientVersion = "";
		// --- 以下仅女士端获取自己信息才有 ---
		needTrans = false;
		transUserId = "";
		transUserName = "";
		transBind = false;
		transStatus = USTATUS_OFFLINE_OR_HIDDEN;
	}
	_tagUserInfoItem(const _tagUserInfoItem& item) {
		userId = item.userId;
		userName = item.userName;
		server = item.server;
		imgUrl = item.imgUrl;
		sexType = item.sexType;
		age = item.age;
		weight = item.weight;
		height = item.height;
		country = item.country;
		province = item.province;
		videoChat = item.videoChat;
		videoCount = item.videoCount;
		marryType = item.marryType;
		childrenType = item.childrenType;
		status = item.status;
		userType = item.userType;
		orderValue = item.orderValue;
		deviceType = item.deviceType;
		clientType = item.clientType;
		clientVersion = item.clientVersion;
		// --- 以下仅女士端获取自己信息才有 ---
		needTrans = item.needTrans;
		transUserId = item.transUserId;
		transUserName = item.transUserName;
		transBind = item.transBind;
		transStatus = item.transStatus;
	}
} UserInfoItem, TalkUserListItem;
// user列表
typedef list<UserInfoItem> UserInfoList;
// 邀请/在聊user列表
typedef list<TalkUserListItem> TalkUserList;

// 邀请/在聊session列表item
typedef struct _tagTalkSessionListItem {
	string	targetId;	// 聊天对象ID
	string	invitedId;	// 邀请ID
	bool	charget;	// 是否已付费
	int		chatTime;	// 聊天时长
	_tagTalkSessionListItem() {
		targetId = "";
		invitedId = "";
		charget = false;
		chatTime = 0;
	}
	_tagTalkSessionListItem(const _tagTalkSessionListItem& item) {
		targetId = item.targetId;
		invitedId = item.invitedId;
		charget = item.charget;
		chatTime = item.chatTime;
	}
} TalkSessionListItem;
// 邀请/在聊session列表
typedef list<TalkSessionListItem> TalkSessionList;

// 获取邀请/在聊列表接口返回信息
typedef struct _tagTalkListInfo {
	TalkUserList	invite;			// 邀请列表
	TalkSessionList	inviteSession;	// 邀请的session列表
	TalkUserList	invited;		// 被邀请列表
	TalkSessionList	invitedSession;	// 被邀请的session列表
	TalkUserList	chating;		// 在聊天列表
	TalkSessionList	chatingSession;	// 在聊天的session列表
	TalkUserList	pause;			// 在聊天已暂停列表
	TalkSessionList	pauseSession;	// 在聊天已暂停的session列表
} TalkListInfo;

// 请求邀请/在聊列表类型
typedef enum {
	TALK_LIST_CHATING = 1,		// 在聊列表
	TALK_LIST_PAUSE = 2,		// 在聊已暂停列表
	TALK_LIST_WOMAN_INVITE = 4,	// 女士邀请列表
	TALK_LIST_MAN_INVITE = 8,	// 男士邀请列表
} TALK_LIST_TYPE;

// 请求联系人/黑名单列表类型
typedef enum {
	CONTACT_LIST_BLOCK = 1,		// 黑名单列表
	CONTACT_LIST_CONTACT = 2,	// 联系人列表
} CONTACT_LIST_TYPE;

// 女士择偶条件item
typedef struct _tagLadyConditionItem {
	string womanId;				// 女士ID
	bool marriageCondition;		// 是否判断婚姻状况
	bool neverMarried;			// 是否从未结婚
	bool divorced;					// 是否离婚
	bool widowed;					// 是否丧偶
	bool separated;					// 是否分居
	bool married;					// 是否已婚
	bool childCondition;		// 是否判断子女状况
	bool noChild;					// 是否没有子女
	bool countryCondition;		// 是否判断国籍
	bool unitedstates;				// 是否美国国籍
	bool canada;					// 是否加拿大国籍
	bool newzealand;				// 是否新西兰国籍
	bool australia;					// 是否澳大利亚国籍
	bool unitedkingdom;				// 是否英国国籍
	bool germany;					// 是否德国国籍
	bool othercountries;			// 是否其它国籍
	int startAge;				// 起始年龄
	int endAge;					// 结束年龄

	_tagLadyConditionItem() {
		womanId = "";
		marriageCondition = false;
		neverMarried = false;
		divorced = false;
		widowed = false;
		separated = false;
		married = false;
		childCondition = false;
		noChild = false;
		countryCondition = false;
		unitedstates = false;
		canada = false;
		newzealand = false;
		australia = false;
		unitedkingdom = false;
		germany = false;
		othercountries = false;
		startAge = 0;
		endAge = 0;
	}
	_tagLadyConditionItem(const _tagLadyConditionItem& item) {
		womanId = item.womanId;
		marriageCondition = item.marriageCondition;
		neverMarried = item.neverMarried;
		divorced = item.divorced;
		widowed = item.widowed;
		separated = item.separated;
		married = item.married;
		childCondition = item.childCondition;
		noChild = item.noChild;
		countryCondition = item.countryCondition;
		unitedstates = item.unitedstates;
		canada = item.canada;
		newzealand = item.newzealand;
		australia = item.australia;
		unitedkingdom = item.unitedkingdom;
		germany = item.germany;
		othercountries = item.othercountries;
		startAge = 0;
		endAge = 0;
	}
} LadyConditionItem;

// 设备类型
typedef enum {
	AUTO_CHARGE_START = 0,			// 开始充值
	AUTO_CHARGE_END = 1,			// 完成充值
} TAUTO_CHARGE_TYPE;
// int 转换 TALK_MSG_TYPE
inline TAUTO_CHARGE_TYPE GetAutoChargeType(int value) {
	TAUTO_CHARGE_TYPE type = (AUTO_CHARGE_START <= value && value <= AUTO_CHARGE_END ? (TAUTO_CHARGE_TYPE)value : AUTO_CHARGE_START);
	return type;
}

// 主题包item
typedef struct _tagThemeInfoItem {
	string themeId;		// 主题包ID
	string manId;		// 男士ID
	string womanId;		// 女士ID
	int startTime;		// 起始时间
	int endTime;		// 结束时间
	int nowTime;		// 服务器当前时间
	int updateTime;		// 最后购买/应用时间（值最大的为当前应用主题包）

	_tagThemeInfoItem() 
	{
		themeId = "";
		manId = "";
		womanId = "";
		startTime = 0;
		endTime = 0;
		nowTime = 0;
		updateTime = 0;
	}

	_tagThemeInfoItem(const _tagThemeInfoItem& item)
	{
		themeId = item.themeId;
		manId = item.manId;
		womanId = item.womanId;
		startTime = item.startTime;
		endTime = item.endTime;
		nowTime = item.nowTime;
		updateTime = item.updateTime;
	}
} ThemeInfoItem;
typedef list<ThemeInfoItem> ThemeInfoList;
