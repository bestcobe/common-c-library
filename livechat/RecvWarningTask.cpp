/*
 * author: Samson.Fan
 *   date: 2015-04-01
 *   file: RecvWarningTask.cpp
 *   desc: 接收警告消息Task实现类
 */

#include "RecvWarningTask.h"
#include "ITaskManager.h"
#include "ILiveChatClient.h"
#include "AmfPublicParse.h"
#include <json/json/json.h>
#include <common/CheckMemoryLeak.h>

// 请求参数定义
#define TOID_PARAM			"toId"			// 接收用户Id
#define FROMID_PARAM		"fromUserId"	// 发送用户Id
#define FROMNAME_PARAM		"fromUserName"	// 发送用户名称
#define INVITEID_PARAM		"inviteId"		// 邀请Id
#define CHARGE_PARAM		"charge"		// 是否已付费
#define TICKET_PARAM		"ticket"		// 票根
#define MSGTYPE_PARAM		"msgType"		// 消息类型
#define MESSAGE_PARAM		"msg"			// 消息内容

RecvWarningTask::RecvWarningTask(void)
{
	m_listener = NULL;

	m_seq = 0;
	m_errType = LCC_ERR_FAIL;
	m_errMsg = "";

	m_toId = "";
	m_fromId = "";
	m_fromName = "";
	m_inviteId = "";
	m_charge = false;
	m_ticket = 0;
	m_msgType = TMT_UNKNOW;
	m_message = "";
}

RecvWarningTask::~RecvWarningTask(void)
{
}

// 初始化
bool RecvWarningTask::Init(ILiveChatClientListener* listener)
{
	bool result = false;
	if (NULL != listener)
	{
		m_listener = listener;
		result = true;
	}
	return result;
}
	
// 处理已接收数据
bool RecvWarningTask::Handle(const TransportProtocol* tp)
{
	bool result = false;
		
	AmfParser parser;
	amf_object_handle root = parser.Decode((char*)tp->data, tp->GetDataLength());
	if (!root.isnull()
		&& root->type == DT_OBJECT)
	{
		// toId
		amf_object_handle toIdObject = root->get_child(TOID_PARAM);
		result = !toIdObject.isnull() && toIdObject->type == DT_STRING;
		if (result) {
			m_toId = toIdObject->strValue;
		}

		// fromId
		amf_object_handle fromIdObject = root->get_child(FROMID_PARAM);
		result = !fromIdObject.isnull() && fromIdObject->type == DT_STRING;
		if (result) {
			m_fromId = fromIdObject->strValue;
		}

		// fromName
		amf_object_handle fromNameObject = root->get_child(FROMNAME_PARAM);
		result = !fromNameObject.isnull() && fromNameObject->type == DT_STRING;
		if (result) {
			m_fromName = fromNameObject->strValue;
		}

		// inviteId
		amf_object_handle inviteIdObject = root->get_child(INVITEID_PARAM);
		result = !inviteIdObject.isnull() && inviteIdObject->type == DT_STRING;
		if (result) {
			m_inviteId = inviteIdObject->strValue;
		}

		// charge
		amf_object_handle chargeObject = root->get_child(CHARGE_PARAM);
		result = !chargeObject.isnull() && (chargeObject->type == DT_FALSE || chargeObject->type == DT_TRUE);
		if (result) {
			m_charge = chargeObject->type == DT_TRUE;
		}

		// ticket
		amf_object_handle ticketObject = root->get_child(TICKET_PARAM);
		result = !ticketObject.isnull() && ticketObject->type == DT_INTEGER;
		if (result) {
			m_ticket = ticketObject->intValue;
		}

		// msgType
		amf_object_handle msgTypeObject = root->get_child(MSGTYPE_PARAM);
		result = !msgTypeObject.isnull() && msgTypeObject->type == DT_INTEGER;
		if (result) {
			m_msgType = GetTalkMsgType(msgTypeObject->intValue);
		}

		// msg
		amf_object_handle msgObject = root->get_child(MESSAGE_PARAM);
		result = !msgObject.isnull() && msgObject->type == DT_STRING;
		if (result) {
			m_message = msgObject->strValue;
		}
	}

	// 通知listener
	if (NULL != m_listener 
		&& result) 
	{
		m_listener->OnRecvWarning(m_toId, m_fromId, m_fromName, m_inviteId, m_charge, m_ticket, m_msgType, m_message);
	}
	
	return result;
}
	
// 获取待发送的数据，可先获取data长度，如：GetSendData(NULL, 0, dataLen);
bool RecvWarningTask::GetSendData(void* data, unsigned int dataSize, unsigned int& dataLen)
{
	bool result = false;
	// 本协议没有返回
	return result;
}

// 获取待发送数据的类型
TASK_PROTOCOL_TYPE RecvWarningTask::GetSendDataProtocolType()
{
	return JSON_PROTOCOL;
}
	
// 获取命令号
int RecvWarningTask::GetCmdCode()
{
	return TCMD_RECVWARNING;	
}

// 设置seq
void RecvWarningTask::SetSeq(unsigned int seq)
{
	m_seq = seq;
}

// 获取seq
unsigned int RecvWarningTask::GetSeq()
{
	return m_seq;
}

// 是否需要等待回复。若false则发送后释放(delete掉)，否则发送后会被添加至待回复列表，收到回复后释放
bool RecvWarningTask::IsWaitToRespond()
{
	return false;
}

// 获取处理结果
void RecvWarningTask::GetHandleResult(LCC_ERR_TYPE& errType, string& errMsg)
{
	errType = m_errType;
	errMsg = m_errMsg;
}

// 未完成任务的断线通知
void RecvWarningTask::OnDisconnect()
{
	// 不用回调
}
