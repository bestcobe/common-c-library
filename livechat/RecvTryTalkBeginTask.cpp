/*
 * author: Samson.Fan
 *   date: 2015-04-01
 *   file: RecvTryTalkBeginTask.cpp
 *   desc: 接收试聊开始Task实现类
 */

#include "RecvTryTalkBeginTask.h"
#include "ITaskManager.h"
#include "ILiveChatClient.h"
#include "AmfPublicParse.h"
#include <json/json.h>

// 请求参数定义
#define TOID_PARAM		"to"
#define FROMID_PARAM	"from"
#define TIME_PARAM		"time"

RecvTryTalkBeginTask::RecvTryTalkBeginTask(void)
{
	m_listener = NULL;

	m_seq = 0;
	m_errType = LCC_ERR_FAIL;
	m_errMsg = "";

	m_toId = "";
	m_fromId = "";
	m_time = 0;
}

RecvTryTalkBeginTask::~RecvTryTalkBeginTask(void)
{
}

// 初始化
bool RecvTryTalkBeginTask::Init(ILiveChatClientListener* listener)
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
bool RecvTryTalkBeginTask::Handle(const TransportProtocol* tp)
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

		// time
		amf_object_handle timeObject = root->get_child(TIME_PARAM);
		result = !timeObject.isnull() && timeObject->type == DT_INTEGER;
		if (result) {
			m_time = timeObject->intValue;
		}
	}

	// 通知listener
	if (NULL != m_listener 
		&& result) 
	{
		m_listener->OnRecvTryTalkBegin(m_toId, m_fromId, m_time);
	}
	
	return result;
}
	
// 获取待发送的数据，可先获取data长度，如：GetSendData(NULL, 0, dataLen);
bool RecvTryTalkBeginTask::GetSendData(void* data, unsigned int dataSize, unsigned int& dataLen)
{
	bool result = false;
	// 本协议没有返回
	return result;
}

// 获取待发送数据的类型
TASK_PROTOCOL_TYPE RecvTryTalkBeginTask::GetSendDataProtocolType()
{
	return JSON_PROTOCOL;
}
	
// 获取命令号
int RecvTryTalkBeginTask::GetCmdCode()
{
	return TCMD_RECVTRYBEGIN;
}

// 设置seq
void RecvTryTalkBeginTask::SetSeq(unsigned int seq)
{
	m_seq = seq;
}

// 获取seq
unsigned int RecvTryTalkBeginTask::GetSeq()
{
	return m_seq;
}

// 是否需要等待回复。若false则发送后释放(delete掉)，否则发送后会被添加至待回复列表，收到回复后释放
bool RecvTryTalkBeginTask::IsWaitToRespond()
{
	return false;
}

// 获取处理结果
void RecvTryTalkBeginTask::GetHandleResult(LCC_ERR_TYPE& errType, string& errMsg)
{
	errType = m_errType;
	errMsg = m_errMsg;
}

// 未完成任务的断线通知
void RecvTryTalkBeginTask::OnDisconnect()
{
	// 不用回调
}
