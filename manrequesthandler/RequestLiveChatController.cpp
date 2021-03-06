/*
 * RequestLiveChatController.cpp
 *
 *  Created on: 2015-2-27
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "RequestLiveChatController.h"
#include <common/CheckMemoryLeak.h>
#include <ProtocolCommon/DeviceTypeDef.h>

RequestLiveChatController::RequestLiveChatController(HttpRequestManager *phttprequestManager, IRequestLiveChatControllerCallback* callback/* CallbackManager* pCallbackManager*/) {
	// TODO Auto-generated constructor stub
	SetHttpRequestManager(phttprequestManager);
	mRequestLiveChatControllerCallback = callback;
}

RequestLiveChatController::~RequestLiveChatController() {
	// TODO Auto-generated destructor stub
}

// ----- IhttprequestManagerCallback -----
void RequestLiveChatController::onSuccess(long requestId, string url, const char* buf, int size) {
	FileLog("httprequest", "RequestLiveChatController::onSuccess( url : %s, buf( size : %d ), requestId:%ld)", url.c_str(), size, requestId);
	if ( size < MAX_LOG_BUFFER && size > 0 ) {
		FileLog("httprequest", "RequestLiveChatController::onSuccess(), buf: %s", buf);
	}

	// parse base result
	string errnum = "";
	string errmsg = "";
	Json::Value data;

	bool bFlag = HandleResult(buf, size, errnum, errmsg, &data);

	// resopned parse ok, callback success
	if( url.compare(CHECK_COUPON_PATH) == 0 ) 
	{
		// 5.1.查询是否符合试聊条件
		Coupon item;

		// parsing json
		Json::Value root;
		Json::Reader reader;
		bFlag = false;
		if (reader.parse(buf, root, false)) {
			bFlag = item.Parse(root);
			FileLog("httprequest", "RequestLiveChatController::onSuccess() CHECK_COUPON_PATH, item.status:%d", item.status);
		}

		// 找回userId
		string userId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				userId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		mRequestLiveChatControllerCallback->OnCheckCoupon(requestId, bFlag, item, userId, errnum, errmsg);
	} 
	else if( url.compare(USE_COUPON_PATH) == 0 ) 
	{
		// 5.2.使用试聊券
		// 找回userId
		string userId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				userId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		mRequestLiveChatControllerCallback->OnUseCoupon(requestId, bFlag, errnum, errmsg, userId);
	} 
	else if( url.find(QUERY_CHAT_VIRTUAL_GIFT_PATH) != string::npos ) 
	{
		// 5.3.获取虚拟礼物列表
		list<Gift> giftList;
		int totalCount;
		string path;
		string version;

		TiXmlDocument doc;
		bFlag = HandleResult(buf, size, errnum, errmsg, doc);
		if( bFlag ) {
			HandleQueryChatVirtualGift(doc, giftList, totalCount, path, version);
		}

		mRequestLiveChatControllerCallback->OnQueryChatVirtualGift(requestId, bFlag, giftList,
				totalCount, path, version, errnum, errmsg);
	} 
	else if( url.compare(QUERY_CHAT_RECORD_PATH) == 0 ) 
	{
		// 5.4.查询聊天记录
		// 找回inviteId
		string inviteId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				inviteId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		int dbTime = 0;
		if (data[LIVECHAT_DBTIME].isIntegral())
		{
			dbTime = data[LIVECHAT_DBTIME].asInt();
		}

		list<Record> recordList;
		if( data[LIVECHAT_INVITEMSG].isArray() ) {
			for(int i = 0; i < data[LIVECHAT_INVITEMSG].size(); i++ ) {
				Record record;
				record.Parse(data[LIVECHAT_INVITEMSG].get(i, Json::Value::null));
				recordList.push_back(record);
			}
		}

		mRequestLiveChatControllerCallback->OnQueryChatRecord(requestId, bFlag, dbTime, recordList, errnum, errmsg, inviteId);
	} 
	else if( url.compare(QUERY_CHAT_RECORD_MUTI_PATH) == 0 ) 
	{
		// 5.5.批量查询聊天记录
		list<RecordMutiple> recordMutipleList;
		if (data[COMMON_DATA_LIST].isArray()) {
			for (int indexDataList = 0; indexDataList < data[COMMON_DATA_LIST].size(); indexDataList++) {
				Json::Value jsonInviteItem = data[COMMON_DATA_LIST].get(indexDataList, Json::Value::null);
				if (!jsonInviteItem.isNull()) {
					Json::Value::Members jsonMember = jsonInviteItem.getMemberNames();
					for (int indexInviteId = 0; indexInviteId < jsonMember.size(); indexInviteId++) {
						RecordMutiple mutiple;
						// 获取 inviteId
						mutiple.inviteId = jsonMember[indexInviteId];
						if (!mutiple.inviteId.empty()) {
							// 获取 inviteId 对应的聊天记录列表
							Json::Value jsonRecordArray = jsonInviteItem[mutiple.inviteId];
							if( jsonRecordArray.isArray() ) {
								for(int i = 0; i < jsonRecordArray.size(); i++ ) {
									Record record;
									record.Parse(jsonRecordArray.get(i, Json::Value::null));
									if (record.messageType != LRM_UNKNOW) {
										mutiple.recordList.push_back(record);
									}
								}
							}
							recordMutipleList.push_back(mutiple);
						}
					}
				}
			}
		}

		int dbTime = 0;
		if (data[LIVECHAT_DBTIME].isIntegral())
		{
			dbTime = data[LIVECHAT_DBTIME].asInt();
		}

		mRequestLiveChatControllerCallback->OnQueryChatRecordMutiple(requestId, bFlag, dbTime, recordMutipleList, errnum, errmsg);
	} 
	else if ( url.compare(LC_SENDPHOTO_PATH) == 0 ) 
	{
		// 5.6.发送私密照片
		LCSendPhotoItem item;
		TiXmlDocument doc;
		bFlag = HandleResult(buf, size, errnum, errmsg, doc);
		if (bFlag) {
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			TiXmlNode *infoNode = rootNode->FirstChild(LIVECHAT_INFO);
			bFlag = item.Parsing(infoNode);
		}
		mRequestLiveChatControllerCallback->OnSendPhoto(requestId, bFlag, errnum, errmsg, item);
	} 
	else if ( url.compare(LC_PHOTOFEE_PATH) == 0 ) 
	{
		// 5.7.付费获取私密照片
		TiXmlDocument doc;
		bFlag = HandleResult(buf, size, errnum, errmsg, doc);
		FileLog("httprequest", "RequestLiveChatController::onSuccess() bFlag:%d", bFlag);
		// 解析已购买
		do {
			if ( bFlag || doc.Error() ) {
				break;
			}

			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			if( rootNode == NULL ) {
				break;
			}

			TiXmlNode *resultNode = rootNode->FirstChild(COMMON_RESULT);
			if (resultNode == NULL) {
				break;
			}

			TiXmlNode *statusNode = resultNode->FirstChild(COMMON_STATUS);
			if (statusNode == NULL) {
				break;
			}

			TiXmlElement* itemElement = statusNode->ToElement();
			if (itemElement == NULL) {
				break;
			}

			string status = itemElement->GetText();
			if (!status.empty()) {
				FileLog("httprequest", "RequestLiveChatController::onSuccess() status:%s", status.c_str());
				bFlag = atoi(status.c_str()) == 2;
			}

			FileLog("httprequest", "RequestLiveChatController::onSuccess() atoi() bFlag:%d", bFlag);
		} while (false);

		mRequestLiveChatControllerCallback->OnPhotoFee(requestId, bFlag, errnum, errmsg);
	} 
	else if ( url.compare(LC_GETPHOTO_PATH) == 0 ) 
	{
		// 5.8.获取对方私密照片
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}
		string tempPath = GetTempFilePath(filePath);

		string errnum = "";
		string errmsg = "";
		bool bFlag = false;
		string contentType = GetContentTypeById(requestId);
		if (string::npos != contentType.find("image")) {
			FILE* file = fopen(tempPath.c_str(), "rb");
			if (NULL != file) {
				fseek(file, 0, SEEK_END);
				size_t fileSize = ftell(file);
				fclose(file);
				file = NULL;

				int recv = 0;
				int total = 0;
				GetRecvLength(requestId, total, recv);
				if (total <= 0
					|| fileSize == (size_t)total)
				{
					bFlag = true;
					rename(tempPath.c_str(), filePath.c_str());
				}
				else {
					FileLog("httprequest", "RequestLiveChatController::onSuccess"
								"( requestId:%ld, url:%s, tempPath:%s, totalSize:%d, fileSize:%d ) file size error",
								requestId, url.c_str(), tempPath.c_str(), total, fileSize);
				}
			}
			else {
				FileLog("httprequest", "RequestLiveChatController::onSuccess"
							"( requestId:%ld, url:%s, tempPath:%s ) open file fail",
							requestId, url.c_str(), tempPath.c_str());
			}
		}
		else {
			FileLog("httprequest", "RequestLiveChatController::onSuccess"
						"( requestId:%ld, url:%s, tempPath:%s ) file size error",
						requestId, url.c_str(), tempPath.c_str());
		}

		if (!bFlag) {
			errnum = LOCAL_ERROR_CODE_FILEOPTFAIL;
			errmsg = LOCAL_ERROR_CODE_FILEOPTFAIL_DESC;
			remove(filePath.c_str());
		}

		mRequestLiveChatControllerCallback->OnGetPhoto(requestId, bFlag, errnum, errmsg, filePath);
	}
	else if ( string::npos != url.find(LC_UPLOADVOICE_SUBPATH) ) {
		// 5.9上传语音文件
		bool bFlag = false;
		string voiceId("");
		string errnum("");
		string errmsg("");
		if (size < 512) {
			voiceId.append(buf, size);
			bFlag = true;
		}
		else {
			errnum = LOCAL_ERROR_CODE_PARSEFAIL;
			errmsg = LOCAL_ERROR_CODE_PARSEFAIL_DESC;
		}

		mRequestLiveChatControllerCallback->OnUploadVoice(requestId, bFlag, errnum, errmsg, voiceId);
	}
	else if ( string::npos != url.find(LC_PLAYVOICE_SUBPATH) ) {
		// 5.10获取语音文件
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		string errnum = "";
		string errmsg = "";
		bool bFlag = false;

		// 判断是否音频类型
		string contentType = GetContentTypeById(requestId);
		if ( string::npos != contentType.find("mpeg")
			|| string::npos != contentType.find("aac")
			|| string::npos != contentType.find("audio"))
		{
			string tempPath = GetTempFilePath(filePath);
			FILE* file = fopen(tempPath.c_str(), "rb");
			if (NULL != file) {
				fseek(file, 0, SEEK_END);
				size_t fileSize = ftell(file);
				fclose(file);
				file = NULL;

				int recv = 0;
				int total = 0;
				GetRecvLength(requestId, total, recv);
				if (total <= 0
					|| fileSize == (size_t)total)
				{
					bFlag = true;
					rename(tempPath.c_str(), filePath.c_str());
				}
			}
		}

		if (!bFlag) {
			errnum = LOCAL_ERROR_CODE_FILEOPTFAIL;
			errmsg = LOCAL_ERROR_CODE_FILEOPTFAIL_DESC;
			remove(filePath.c_str());
		}

		mRequestLiveChatControllerCallback->OnPlayVoice(requestId, bFlag, errnum, errmsg, filePath);
	} else if ( string::npos != url.find(LC_SENDGIFT_PATH) ) {
		// 6.11.发送虚拟礼物
		TiXmlDocument doc;
		bFlag = HandleResult(buf, size, errnum, errmsg, doc);

		mRequestLiveChatControllerCallback->OnSendGift(requestId, bFlag, errnum, errmsg);
	} else if( url.compare(LC_RECENT_VIDEO_PATH) == 0) {
		// 6.12.获取最近已看微视频列表
		list<LCVideoItem> itemList;
		TiXmlDocument doc;
		bFlag = HandleResult(buf, size, errnum, errmsg, doc);
		if( bFlag ) {
			HandleQueryRecentVideo(doc, itemList);
		}

		mRequestLiveChatControllerCallback->OnQueryRecentVideoList(requestId, bFlag, itemList, errnum, errmsg);
	} else if( url.compare(LC_GET_VIDEO_PHOTO_PATH) == 0 ) {
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}
		string tempPath = GetTempFilePath(filePath);

		string errnum = "";
		string errmsg = "";
		bool bFlag = false;
		string contentType = GetContentTypeById(requestId);
		if (string::npos != contentType.find("image")) {
			FILE* file = fopen(tempPath.c_str(), "rb");
			if (NULL != file) {
				fseek(file, 0, SEEK_END);
				size_t fileSize = ftell(file);
				fclose(file);
				file = NULL;

				int recv = 0;
				int total = 0;
				GetRecvLength(requestId, total, recv);
				if (total <= 0
					|| fileSize == (size_t)total)
				{
					bFlag = true;
					rename(tempPath.c_str(), filePath.c_str());
				}
				else {
					FileLog("httprequest", "RequestLiveChatController::onSuccess"
								"( requestId:%ld, url:%s, tempPath:%s, totalSize:%d, fileSize:%d ) file size error",
								requestId, url.c_str(), tempPath.c_str(), total, fileSize);
				}
			}
			else {
				FileLog("httprequest", "RequestLiveChatController::onSuccess"
							"( requestId:%ld, url:%s, tempPath:%s ) open file fail",
							requestId, url.c_str(), tempPath.c_str());
			}
		}
		else {
			FileLog("httprequest", "RequestLiveChatController::onSuccess"
						"( requestId:%ld, url:%s, tempPath:%s ) file size error",
						requestId, url.c_str(), tempPath.c_str());
		}

		if (!bFlag) {
			errnum = LOCAL_ERROR_CODE_FILEOPTFAIL;
			errmsg = LOCAL_ERROR_CODE_FILEOPTFAIL_DESC;
			remove(filePath.c_str());
		}

		mRequestLiveChatControllerCallback->OnGetVideoPhoto(requestId, bFlag, errnum, errmsg, filePath);
	} else if( url.compare(LC_GET_VIDEO_PATH) == 0 ) {
		// 6.14.获取微视频文件URL
		TiXmlDocument doc;
		string url = "";

		bFlag = HandleResult(buf, size, errnum, errmsg, doc);
		if( bFlag ) {
			HandleGetVideo(doc, url);
		}

		mRequestLiveChatControllerCallback->OnGetVideo(requestId, bFlag, errnum, errmsg, url);
	}else if( url.compare(LC_GET_MAGICICON_CONFIG_PATH) == 0 ) {
		// 6.15.查询小高级表情配置
		TiXmlDocument doc;
		MagicIconConfig config ;

		if(HandleResult(buf, size, errnum, errmsg, doc)){
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			if(NULL != rootNode){
				if(config.parsing(rootNode)){
					bFlag = true;
				}
			}
		}
		mRequestLiveChatControllerCallback->OnGetMagicIconConfig(requestId, bFlag, errnum, errmsg, config);
	} else if (url.compare(LC_CHAT_RECHARGE_PATH) == 0) {
		// 6.16.开聊自动买点
		TiXmlDocument doc;
		string url = "";

		double credits = -1;
		if( HandleResult(buf, size, errnum, errmsg, doc) ) {
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			TiXmlNode *infoNode = rootNode->FirstChild(LIVECHAT_INFO);
			if ( NULL != infoNode ) {
				TiXmlNode *creditsNode = infoNode->FirstChild(LC_CHAT_RECHARGE_CREDITS);
				if (NULL != creditsNode) {
					TiXmlElement* itemElement = creditsNode->ToElement();
					if ( NULL != itemElement ) {
						const char* p = itemElement->GetText();
						if( p != NULL ) {
							credits = atof(p);
							bFlag = true;
						}
					}
				}
			}
		}

		mRequestLiveChatControllerCallback->OnChatRecharge(requestId, bFlag, errnum, errmsg, credits);
	}else if (url.compare(LC_GET_THEME_CONFIG_PATH) == 0) {
		// 6.17.查询主题配置
		TiXmlDocument doc;
		ThemeConfig config;

		if( HandleResult(buf, size, errnum, errmsg, doc) ) {
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			if(NULL != rootNode){
				if(config.parsing(rootNode)){
					bFlag = true;
				}
			}
		}
		mRequestLiveChatControllerCallback->OnGetThemeConfig(requestId, bFlag, errnum, errmsg, config);
	}else if (url.compare(LC_GET_THEME_DETAIL_PATH) == 0) {
		// 6.18.获取指定主题
		TiXmlDocument doc;
		ThemeItemList themeItemList;

		if( HandleResult(buf, size, errnum, errmsg, doc) ) {
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			if(NULL != rootNode){
				TiXmlNode* themeRootNode = rootNode->FirstChild(LC_GET_THEME_CONFIG_THEME_FORMAN);
				if(NULL != themeRootNode){
					TiXmlNode* themeNode = themeRootNode->FirstChild(LC_GET_THEME_CONFIG_THEME_LIST);
					while(NULL != themeNode){
						ThemeItem themeItem;
						if(themeItem.parsing(themeNode)){
							themeItemList.push_back(themeItem);
						}
						themeNode = themeNode->NextSibling();
					}
				}
			}
			bFlag = true;
		}
		mRequestLiveChatControllerCallback->OnGetThemeDetail(requestId, bFlag, errnum, errmsg, themeItemList);
	}else if (url.compare(LC_CHECK_FUNCTIONS_PATH) == 0) {
		// 6.19.检测功能是否开通
		TiXmlDocument doc;
		const string split = ",";
		list<string> flagList;

		if( HandleResult(buf, size, errnum, errmsg, doc) ) {
			TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
			TiXmlElement* dataElement = rootNode->FirstChildElement(LC_CHECK_FUNCTIONS_DATA);
			if(NULL != dataElement){
				string flags = dataElement->GetText();
				size_t pos = 0;
				do {
					size_t cur = flags.find(split, pos);
					if (cur != string::npos) {
						string temp = flags.substr(pos, cur - pos);
						if (!temp.empty()) {
							flagList.push_back(temp);
						}
						pos = cur + 1;
					}
					else {
						string temp = flags.substr(pos);
						if (!temp.empty()) {
							flagList.push_back(temp);
						}
						break;
					}
				} while(true);
			}
		}
		if(flagList.size()>0){
			bFlag = true;
		}

		mRequestLiveChatControllerCallback->OnCheckFunctions(requestId, bFlag, errnum, errmsg, flagList);
	}


	FileLog("httprequest", "RequestLiveChatController::onSuccess() url: %s, end", url.c_str());
}
void RequestLiveChatController::onFail(long requestId, string url) {
	FileLog("httprequest", "RequestLiveChatController::onFail( url : %s ), requestId:%ld", url.c_str(), requestId);
	// request fail, callback fail
	if( url.compare(CHECK_COUPON_PATH) == 0 ) {
		// 5.1.查询是否符合试聊条件
		// 找回userId
		string userId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				userId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		Coupon item;
		mRequestLiveChatControllerCallback->OnCheckCoupon(requestId, false, item, userId, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if( url.compare(USE_COUPON_PATH) == 0 ) {
		// 5.2.使用试聊券
		// 找回userId
		string userId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				userId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		mRequestLiveChatControllerCallback->OnUseCoupon(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, userId);
	} else if( url.find(QUERY_CHAT_VIRTUAL_GIFT_PATH) != string::npos ) {
		list<Gift> giftList;
		mRequestLiveChatControllerCallback->OnQueryChatVirtualGift(requestId, false, giftList,
						0, "", "", LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if( url.compare(QUERY_CHAT_RECORD_PATH) == 0 ) {
		// 5.4.查询聊天记录
		// 找回inviteId
		string inviteId("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				inviteId = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		list<Record> recordList;
		mRequestLiveChatControllerCallback->OnQueryChatRecord(requestId, false, 0, recordList, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, inviteId);
	} else if( url.compare(QUERY_CHAT_RECORD_MUTI_PATH) == 0 ) {
		// 5.5.批量查询聊天记录
		list<RecordMutiple> recordMutiList;
		mRequestLiveChatControllerCallback->OnQueryChatRecordMutiple(requestId, false, 0, recordMutiList, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if ( url.compare(LC_SENDPHOTO_PATH) == 0 ) {
		// 5.6.发送私密照片
		LCSendPhotoItem item;
		mRequestLiveChatControllerCallback->OnSendPhoto(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, item);
	} else if ( url.compare(LC_PHOTOFEE_PATH) == 0 ) {
		// 5.7.付费获取私密照片
		mRequestLiveChatControllerCallback->OnPhotoFee(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if ( url.compare(LC_GETPHOTO_PATH) == 0 ) {
		// 5.8.获取对方私密照片
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		// 删除文件
		remove(filePath.c_str());

		mRequestLiveChatControllerCallback->OnGetPhoto(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, filePath);
	} else if ( string::npos != url.find(LC_UPLOADVOICE_SUBPATH) ) {
		mRequestLiveChatControllerCallback->OnUploadVoice(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, "");
	} else if ( string::npos != url.find(LC_PLAYVOICE_SUBPATH) ) {
		mRequestLiveChatControllerCallback->OnPlayVoice(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, "");
	} else if ( url.compare(LC_SENDGIFT_PATH) == 0 ) {
		// 6.11.发送虚拟礼物
		mRequestLiveChatControllerCallback->OnSendGift(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if( url.compare(LC_RECENT_VIDEO_PATH) == 0) {
		// 6.12.获取最近已看微视频列表
		list<LCVideoItem> itemList;
		mRequestLiveChatControllerCallback->OnQueryRecentVideoList(requestId, false, itemList, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC);
	} else if( url.compare(LC_GET_VIDEO_PHOTO_PATH) == 0 ) {
		// 6.13.获取微视频图片
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
				delete param;
			}
			mCustomParamMap.erase(iter);
		}

		// 删除文件
		remove(filePath.c_str());

		mRequestLiveChatControllerCallback->OnGetVideoPhoto(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, filePath);
	} else if( url.compare(LC_GET_VIDEO_PATH) == 0 ) {
		// 6.14.获取微视频文件URL
		mRequestLiveChatControllerCallback->OnGetVideo(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, "");
	}else if( url.compare(LC_GET_MAGICICON_CONFIG_PATH) == 0 ) {
		// 6.15.查询小高级表情配置
		MagicIconConfig config;
		mRequestLiveChatControllerCallback->OnGetMagicIconConfig(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, config);
	}else if (url.compare(LC_CHAT_RECHARGE_PATH) == 0) {
		// 6.16.开聊自动买点
		mRequestLiveChatControllerCallback->OnChatRecharge(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, -1);
	}else if (url.compare(LC_GET_THEME_CONFIG_PATH) == 0) {
		// 6.17.查询主题配置
		ThemeConfig themeConfig;
		mRequestLiveChatControllerCallback->OnGetThemeConfig(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, themeConfig);
	}else if (url.compare(LC_GET_THEME_DETAIL_PATH) == 0) {
		// 6.18.获取指定主题
		ThemeItemList themeItemList;
		mRequestLiveChatControllerCallback->OnGetThemeDetail(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, themeItemList);
	}else if(url.compare(LC_CHECK_FUNCTIONS_PATH) == 0){
		list<string> flagList;
		mRequestLiveChatControllerCallback->OnCheckFunctions(requestId, false, LOCAL_ERROR_CODE_TIMEOUT, LOCAL_ERROR_CODE_TIMEOUT_DESC, flagList);
	}

	FileLog("httprequest", "RequestLiveChatController::onFail() url: %s, end", url.c_str());
}

void RequestLiveChatController::onReceiveBody(long requestId, string url, const char* buf, int size)
{
	if ( string::npos != url.find(LC_PLAYVOICE_SUBPATH) ) {
		// 5.10获取语音文件
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
			}
		}

		if (!filePath.empty()) {
			string tempPath = GetTempFilePath(filePath);
			FILE* pFile = fopen(tempPath.c_str(), "a+b");
			if (NULL != pFile) {
				fwrite(buf, 1, size, pFile);
				fclose(pFile);

				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( write file requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
			else {
				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( open file fail, requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
		}
		else {
			FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
						"( param error, requestId:%ld, url:%s, filePath is empty)",
						requestId, url.c_str() );
		}
	}
	else if ( url.compare(LC_GETPHOTO_PATH) == 0 ) {
		// 5.8.获取对方私密照片
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
			}
		}

		string errnum = "";
		string errmsg = "";
		bool bFlag = false;
		if (!filePath.empty()) {
			string tempPath = GetTempFilePath(filePath);
			FILE* pFile = fopen(tempPath.c_str(), "a+b");
			if (NULL != pFile) {
				fwrite(buf, 1, size, pFile);
				fclose(pFile);

				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( write file success:%d, requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							bFlag, requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
			else {
				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( open file fail, requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
		}
		else {
			FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
						"( param error, requestId:%ld, url:%s, filePath is empty)",
						requestId, url.c_str() );
		}
	} else if( url.compare(LC_GET_VIDEO_PHOTO_PATH) == 0 ) {
		// 6.13.获取微视频图片
		// 找回文件路径
		string filePath("");
		RequestCustomParamMap::iterator iter = mCustomParamMap.find(requestId);
		if (mCustomParamMap.end() != iter) {
			string* param = (string*)((*iter).second);
			if (NULL != param) {
				filePath = *param;
			}
		}

		string errnum = "";
		string errmsg = "";
		bool bFlag = false;
		if (!filePath.empty()) {
			string tempPath = GetTempFilePath(filePath);
			FILE* pFile = fopen(tempPath.c_str(), "a+b");
			if (NULL != pFile) {
				fwrite(buf, 1, size, pFile);
				fclose(pFile);

				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( write file success:%d, requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							bFlag, requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
			else {
				FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
							"( open file fail, requestId:%ld, url:%s, filePath:%s, tempPath:%s )",
							requestId, url.c_str(), filePath.c_str(), tempPath.c_str());
			}
		}
		else {
			FileLog("httprequest", "RequestLiveChatController::onReceiveBody"
						"( param error, requestId:%ld, url:%s, filePath is empty)",
						requestId, url.c_str() );
		}
	}
}

/**
 * 5.1.查询是否符合试聊条件
 * @param womanId			女士ID
 * @return					请求唯一标识
 */
long RequestLiveChatController::CheckCoupon(string womanId) {
	HttpEntiy entiy;

	if( womanId.length() > 0 ) {
		entiy.AddContent(LIVECHAT_WOMAN_ID, womanId);
	}

	string url = CHECK_COUPON_PATH;
	FileLog("httprequest", "RequestLiveChatController::CheckCoupon( "
			"url : %s, "
			"womanId : %s, "
			")",
			url.c_str(),
			womanId.c_str()
			);

	long requestId = StartRequest(url, entiy, this);
	if (requestId != -1) {
		string* param = new string(womanId);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}
	return requestId;
}

/**
 * 5.2.使用试聊券
 * @param womanId			女士ID
 * @return					请求唯一标识
 */
long RequestLiveChatController::UseCoupon(string womanId) {
	HttpEntiy entiy;

	if( womanId.length() > 0 ) {
		entiy.AddContent(LIVECHAT_WOMAN_ID, womanId);
	}

	string url = USE_COUPON_PATH;
	FileLog("httprequest", "RequestLiveChatController::UseCoupon( "
			"url : %s, "
			"womanId : %s"
			")",
			url.c_str(),
			womanId.c_str()
			);

	long requestId = StartRequest(url, entiy, this);
	FileLog("httprequest", "RequestLiveChatController::UseCoupon() StartRequest ok, requestId: %ld", requestId);
	if (requestId != -1) {
		string* param = new string(womanId);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}

	FileLog("httprequest", "RequestLiveChatController::UseCoupon( "
			"requestId: %ld,"
			"url : %s, "
			"womanId : %s"
			")",
			requestId,
			url.c_str(),
			womanId.c_str()
			);
	return requestId;
}

/**
 * 5.3.获取虚拟礼物列表
 * @param sessionId			登录成功返回的sessionid
 * @param userId			登录成功返回的manid
 * @return					请求唯一标识
 */
long RequestLiveChatController::QueryChatVirtualGift(string sessionId, string userId) {
	HttpEntiy entiy;
	entiy.SetGetMethod(true);

	string url = QUERY_CHAT_VIRTUAL_GIFT_PATH;

	if( sessionId.length() > 0 ) {
		url += LIVECHAT_SESSIONID;
		url += "=";
		url += sessionId;
		url += "&";
	}

	if( userId.length() > 0 ) {
		url += LIVECHAT_USER_ID;
		url += "=";
		url += userId;
		url += "&";
	}

	url = url.substr(0, url.length() - 1);

	FileLog("httprequest", "RequestLiveChatController::QueryChatVirtualGift( "
			"url : %s, "
			"sessionId : %s, "
			"userId : %s "
			")",
			url.c_str(),
			sessionId.c_str(),
			userId.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

void RequestLiveChatController::HandleQueryChatVirtualGift(TiXmlDocument &doc, list<Gift> &giftList,
		int &totalCount, string &path, string &version) {
	const char *p = NULL;
	TiXmlElement* itemElement;
	TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
	if( rootNode != NULL ) {
		TiXmlNode *totalNode = rootNode->FirstChild(LIVECHAT_VIRTUALGIFT_TOTAL);
		if( totalNode != NULL ) {
			itemElement = totalNode->ToElement();
			if ( itemElement != NULL ) {
				p = itemElement->GetText();
				if( p != NULL ) {
					totalCount = atoi(p);
				}
			}
		}

		TiXmlNode *pathNode = rootNode->FirstChild(LIVECHAT_VIRTUALGIFT_PATH);
		if( pathNode != NULL ) {
			itemElement = pathNode->ToElement();
			if ( itemElement != NULL ) {
				p = itemElement->GetText();
				if( p != NULL ) {
					path = p;
				}
			}
		}

		TiXmlNode *versionNode = rootNode->FirstChild(LIVECHAT_VIRTUALGIFT_VERSION);
		if( versionNode != NULL ) {
			itemElement = versionNode->ToElement();
			if ( itemElement != NULL ) {
				p = itemElement->GetText();
				if( p != NULL ) {
					version = p;
				}
			}
		}

		TiXmlNode *manNode = rootNode->FirstChild(LIVECHAT_VIRTUALGIFT_FORMAN);
		if( manNode != NULL ) {
			TiXmlNode *listNode = manNode->FirstChild(LIVECHAT_LIST);

			while( listNode != NULL ) {
				Gift gift;
				gift.Parse(listNode);
				giftList.push_back(gift);

				listNode = listNode->NextSibling();
			}
		}
	}
}

/**
 * 5.4.查询聊天记录
 * @param inviteId			邀请ID
 * @return					请求唯一标识
 */
long RequestLiveChatController::QueryChatRecord(string inviteId) {
	HttpEntiy entiy;

	if( inviteId.length() > 0 ) {
		entiy.AddContent(LIVECHAT_INVITEID, inviteId);
	}

	string url = QUERY_CHAT_RECORD_PATH;
	FileLog("httprequest", "RequestLiveChatController::QueryChatRecord( "
			"url : %s, "
			"inviteId : %s, "
			")",
			url.c_str(),
			inviteId.c_str()
			);

	long requestId = StartRequest(url, entiy, this);
	if (requestId != -1) {
		string* param = new string(inviteId);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}

	return requestId;
}

/**
 * 5.5.批量查询聊天记录
 * @param inviteId			邀请ID数组
 * @return					请求唯一标识
 */
long RequestLiveChatController::QueryChatRecordMutiple(list<string> inviteIdList) {
	HttpEntiy entiy;

	string inviteIds = "";

	for(list<string>::iterator itr = inviteIdList.begin(); itr != inviteIdList.end(); itr++) {
		inviteIds += *itr;
		inviteIds += ",";
	}

	if( inviteIds.length() > 0 ) {
		inviteIds = inviteIds.substr(0, inviteIds.length() - 1);
	}

	if( inviteIds.length() > 0 ) {
		entiy.AddContent(LIVECHAT_INVITEIDS, inviteIds);
	}

	string url = QUERY_CHAT_RECORD_MUTI_PATH;
	FileLog("httprequest", "RequestLiveChatController::QueryChatRecord( "
			"url : %s, "
			"inviteIds : %s, "
			")",
			url.c_str(),
			inviteIds.c_str()
			);

	return StartRequest(url, entiy, this);
}

// 发送私密照片
long RequestLiveChatController::SendPhoto(
		const string& targetId
		, const string& inviteId
		, const string& userId
		, const string& sid
		, SENDPHOTO_MODE_TYPE modeType
		, const string& photoId
		, const string& filePath)
{
	HttpEntiy entiy;

	// targetId
	entiy.AddContent(LIVECHAT_TARGETID, targetId);
	// inviteId
	entiy.AddContent(LIVECHAT_INVITEID, inviteId);
	// userId
	entiy.AddContent(LIVECHAT_USER_ID, userId);
	// sid
	entiy.AddContent(LIVECHAT_SESSIONID, sid);
	// mode
	char temp[32] = {0};
	snprintf(temp, sizeof(temp), "%d", modeType);
	entiy.AddContent(LIVECHAT_SENDMODE, temp);
	// photoId
	entiy.AddContent(LIVECHAT_PHOTOID, photoId);
	// file
	entiy.AddFile(LIVECHAT_PHOTOFILE, filePath);

	string url = LC_SENDPHOTO_PATH;
	FileLog("httprequest", "RequestLiveChatController::SendPhoto( "
			"url: %s, "
			"targetId: %s, "
			"inviteId: %s, "
			"modeType: %d, "
			"photoId: %s, "
			"filePath: %s"
			")",
			url.c_str(),
			targetId.c_str(),
			inviteId.c_str(),
			modeType,
			photoId.c_str(),
			filePath.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

// 付费获取私密照片
long RequestLiveChatController::PhotoFee(const string& targetId, const string& inviteId, const string& userId, const string& sid, const string& photoId)
{
	HttpEntiy entiy;

	// targetId
	entiy.AddContent(LIVECHAT_TARGETID, targetId);
	// inviteId
	entiy.AddContent(LIVECHAT_INVITEID, inviteId);
	// userId
	entiy.AddContent(LIVECHAT_USER_ID, userId);
	// sid
	entiy.AddContent(LIVECHAT_SESSIONID, sid);
	// photoId
	entiy.AddContent(LIVECHAT_PHOTOID, photoId);

	string url = LC_PHOTOFEE_PATH;
	FileLog("httprequest", "RequestLiveChatController::PhotoFee( "
			"url: %s, "
			"targetId: %s, "
			"inviteId: %s, "
			"photoId: %s, "
			")",
			url.c_str(),
			targetId.c_str(),
			inviteId.c_str(),
			photoId.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

// 获取私密照片文件
long RequestLiveChatController::GetPhoto(
		GETPHOTO_TOFLAG_TYPE toFlag
		, const string& targetId
		, const string& userId
		, const string& sid
		, const string& photoId
		, GETPHOTO_PHOTOSIZE_TYPE sizeType
		, GETPHOTO_PHOTOMODE_TYPE modeType
		, const string& filePath)
{
	HttpEntiy entiy;
	char temp[32] = {0};

	// delete file
	string tempPath = GetTempFilePath(filePath);
	remove(tempPath.c_str());

	// toFlag
	snprintf(temp, sizeof(temp), "%d", toFlag);
	entiy.AddContent(LIVECHAT_TOFLAG, temp);
	// targetId
	entiy.AddContent(LIVECHAT_TARGETID, targetId);
	// userId
	entiy.AddContent(LIVECHAT_USER_ID, userId);
	// sid
	entiy.AddContent(LIVECHAT_SESSIONID, sid);
	// photoId
	entiy.AddContent(LIVECHAT_PHOTOID, photoId);
	// sizeType
	string strSizeType("");
	if ( GPT_LARGE <= sizeType && sizeType <= GPT_ORIGINAL ) {
		strSizeType = GETPHOTO_PHOTOSIZE_PROTOCOL[sizeType];
	}
	entiy.AddContent(LIVECHAT_PHOTOSIZE, strSizeType);
	// modeType
	snprintf(temp, sizeof(temp), "%d", modeType);
	entiy.AddContent(LIVECHAT_PHOTOMODE, temp);

	string url = LC_GETPHOTO_PATH;
	FileLog("httprequest", "RequestLiveChatController::GetPhoto( "
			"url: %s, "
			"targetId: %s, "
			"photoId: %s, "
			"sizeType: %s, "
			"modeType: %d, "
			"filePath: %s, "
			")",
			url.c_str(),
			targetId.c_str(),
			photoId.c_str(),
			strSizeType.c_str(),
			modeType,
			filePath.c_str()
			);

	// 删除文件
	remove(filePath.c_str());

	long requestId = -1;
	requestId = StartRequest(url, entiy, this, WebSite, false);
	if (requestId != -1) {
		string* param = new string(filePath);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}
	return requestId;
}

// 上传语音文件
long RequestLiveChatController::UploadVoice(
		const string& voiceCode
		, const string& inviteId
		, const string& mineId
		, bool isMan
		, const string& userId
		, OTHER_SITE_TYPE siteType
		, const string& fileType
		, int voiceLen
		, const string& filePath)
{
	HttpEntiy entiy;
	char temp[2048] = {0};

	// inviteId
	entiy.AddContent(LC_UPLOADVOICE_INVITEID, inviteId);
	// mineId
	entiy.AddContent(LC_UPLOADVOICE_MINEID, mineId);
	// sex
	entiy.AddContent(LC_UPLOADVOICE_SEX, isMan ? LC_UPLOADVOICE_SEX_MAN : LC_UPLOADVOICE_SEX_WOMAN);
	// userId
	entiy.AddContent(LC_UPLOADVOICE_WOMANID, userId);
	// site
	const char* siteId = GetSiteId(siteType);
	entiy.AddContent(LC_UPLOADVOICE_SITEID, siteId);
	// fileType
	entiy.AddContent(LC_UPLOADVOICE_FILETYPE, fileType);
	// voiceLen
	snprintf(temp, sizeof(temp), "%d", voiceLen);
	entiy.AddContent(LC_UPLOADVOICE_VOICELENGTH, temp);
	// add file
	entiy.AddFile("voicefile", filePath.c_str());

	// make url
	snprintf(temp, sizeof(temp), LC_UPLOADVOICE_PATH, voiceCode.c_str());
	string url = temp;

	FileLog("httprequest", "RequestLiveChatController::UploadVoice( "
			"url: %s, "
			"voiceCode: %s, "
			"inviteId: %s, "
			"mineId: %s, "
			"userId: %s, "
			"siteId: %s, "
			"fileType: %s, "
			"voiceLen: %d, "
			"filePath: %s"
			")",
			url.c_str(),
			voiceCode.c_str(),
			inviteId.c_str(),
			mineId.c_str(),
			userId.c_str(),
			siteId,
			fileType.c_str(),
			voiceLen,
			filePath.c_str());

	return StartRequest(url, entiy, this, ChatVoiceSite);
}

// 获取语音文件
long RequestLiveChatController::PlayVoice(const string& voiceId, OTHER_SITE_TYPE siteType, const string& filePath)
{
	HttpEntiy entiy;
	char temp[2048] = {0};

	// delete file
	string tempPath = GetTempFilePath(filePath);
	remove(tempPath.c_str());

	// make url
	const char* siteId = GetSiteId(siteType);
	snprintf(temp, sizeof(temp), LC_PLAYVOICE_PATH, voiceId.c_str(), siteId);
	// change to http get
	entiy.SetGetMethod(true);

	string url = temp;

	// 删除文件
	remove(filePath.c_str());

	long requestId = -1;
	requestId = StartRequest(url, entiy, this, ChatVoiceSite, false);
	if (requestId != -1) {
		string* param = new string(filePath);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}

	FileLog("httprequest", "RequestLiveChatController::PlayVoice( "
			"requestId: %ld,"
			"url: %s, "
			"voiceId: %s, "
			"siteId: %s, "
			"filePath: %s"
			")",
			requestId,
			url.c_str(),
			voiceId.c_str(),
			siteId,
			filePath.c_str());

	return requestId;
}

string RequestLiveChatController::GetTempFilePath(const string& filePath)
{
	static const char* tempFileName = ".tmp";
	return filePath + tempFileName;
}

/**
 * 6.11.发送虚拟礼物
 * @param womanId		女士ID
 * @param vg_id			虚拟礼物ID
 * @param device_id		设备唯一标识
 * @param chat_id		livechat邀请ID或EMF邮件ID
 * @param use_type		模块类型<UseType>
 * @param user_sid		登录成功返回的sessionid
 * @param user_id		登录成功返回的manid
 * @return				请求唯一Id
 */
long RequestLiveChatController::SendGift(string womanId, string vg_id, string device_id, string chat_id, int use_type, string user_sid, string user_id) {
	HttpEntiy entiy;
	entiy.SetGetMethod(true);

	string url = LC_SENDGIFT_PATH;

	// womanId
	entiy.AddContent(LIVECHAT_WOMAN_ID, womanId.c_str());

	// vg_id
	entiy.AddContent(LIVECHAT_SENDGIFT_VG_ID, vg_id.c_str());

	// device_id
	entiy.AddContent(LIVECHAT_SENDGIFT_DEVICE_ID, device_id.c_str());

	// chat_id
	entiy.AddContent(LIVECHAT_SENDGIFT_CHAT_ID, chat_id.c_str());

	// use_type
	string type;
	if( use_type > -1 && use_type < _countof(USET_TYPE_ARRAY) ) {
		type = USET_TYPE_ARRAY[use_type];
		entiy.AddContent(LIVECHAT_SENDGIFT_USE_TYPE, type.c_str());
	}

	// user_sid
	entiy.AddContent(LIVECHAT_SENDGIFT_USER_SID, user_sid.c_str());

	// user_id
	entiy.AddContent(LIVECHAT_SENDGIFT_USER_ID, user_id.c_str());

	FileLog("httprequest", "RequestLiveChatController::SendGift( "
			"womanId: %s,"
			"vg_id: %s, "
			"device_id: %s, "
			"chat_id: %s, "
			"use_type: %s, "
			"user_sid : %s, "
			"user_id : %s "
			""
			")",
			womanId.c_str(),
			vg_id.c_str(),
			device_id.c_str(),
			chat_id.c_str(),
			type.c_str(),
			user_sid.c_str(),
			user_id.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.12.获取最近已看微视频列表（http post）（New）
 * @param womanId		女士ID
 * @return				请求唯一Id
 */
long RequestLiveChatController::QueryRecentVideo(
		string user_sid,
		string user_id,
		string womanId
		) {
	HttpEntiy entiy;

	string url = LC_RECENT_VIDEO_PATH;

	if( user_sid.length() > 0 ) {
		// user_sid
		entiy.AddContent(LC_RECENT_VIDEO_USER_SID, user_sid.c_str());
	}

	if( user_id.length() > 0 ) {
		// user_id
		entiy.AddContent(LC_RECENT_VIDEO_USER_ID, user_id.c_str());
	}

	if( womanId.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_RECENT_VIDEO_TARGETID, womanId.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::QueryRecentVideo( "
			"url : %s, "
			"targetid: %s "
			")",
			url.c_str(),
			womanId.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}
void RequestLiveChatController::HandleQueryRecentVideo(TiXmlDocument &doc, list<LCVideoItem> &itemList) {
//	const char *p = NULL;
	TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
	if( rootNode != NULL ) {
		TiXmlNode *videoNode = rootNode->FirstChild(LC_RECENT_VIDEO_VIDEO);
		if( videoNode != NULL ) {
			TiXmlNode *listNode = videoNode->FirstChild(LC_RECENT_VIDEO_LIST);
			while( listNode != NULL ) {
				LCVideoItem item;
				item.Parse(listNode);
				itemList.push_back(item);

				listNode = listNode->NextSibling();
			}
		}
	}
}

/**
 * 6.13.获取微视频图片（http get）（New）
 * @param womanId		女士ID
 * @param videoid		视频ID
 * @param type			图片尺寸
 * @return				请求唯一Id
 */
long RequestLiveChatController::GetVideoPhoto(
		string user_sid,
		string user_id,
		string womanId,
		string videoid,
		VIDEO_PHOTO_TYPE type,
		const string& filePath
		) {
	HttpEntiy entiy;
	char temp[32] = {0};

	// delete file
	string tempPath = GetTempFilePath(filePath);
	remove(tempPath.c_str());

	string url = LC_GET_VIDEO_PHOTO_PATH;

	// user_sid
	if( user_sid.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_PHOTO_USER_SID, user_sid.c_str());
	}

	// user_id
	if( user_id.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_PHOTO_USER_ID, user_id.c_str());
	}

	// womanId
	if( womanId.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_PHOTO_TARGETID, womanId.c_str());
	}

	// videoid
	if( videoid.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_PHOTO_VIDEOID, videoid.c_str());
	}

	// type
	snprintf(temp, sizeof(temp), "%d", type);
	entiy.AddContent(LC_GET_VIDEO_PHOTO_SIZE, temp);

	FileLog("httprequest", "RequestLiveChatController::GetVideoPhoto( "
			"url : %s, "
			"targetid: %s, "
			"videoid : %s, "
			"type : %d, "
			"filePath : %s "
			")",
			url.c_str(),
			womanId.c_str(),
			videoid.c_str(),
			type,
			filePath.c_str()
			);

	// 删除文件
	remove(filePath.c_str());

	long requestId = -1;
	requestId = StartRequest(url, entiy, this, WebSite, false);
	if (requestId != -1) {
		string* param = new string(filePath);
		if (NULL != param) {
			mCustomParamMap.insert(RequestCustomParamMap::value_type(requestId, (void*)param));
		}
	}
	return requestId;
}

void RequestLiveChatController::HandleGetVideo(TiXmlDocument &doc, string &url) {
//	const char *p = NULL;
	TiXmlNode *rootNode = doc.FirstChild(COMMON_ROOT);
	if( rootNode != NULL ) {
		TiXmlNode *videoNode = rootNode->FirstChild(LC_GET_VIDEO_VIDEO_URL);
		if( videoNode != NULL ) {
			TiXmlElement* itemElement = videoNode->ToElement();
			if ( itemElement != NULL ) {
				const char* p = itemElement->GetText();
				if( p != NULL ) {
					url = p;
				}
			}
		}
	}
}
/**
 * 6.14.获取微视频文件URL（http post）（New）
 * @param womanId		女士ID
 * @param videoid		视频ID
 * @param inviteid		邀请ID
 * @param toflag		客户端类型（0：女士端，1：男士端）
 * @param sendid		发送ID，在LiveChat收到女士端发出的消息中
 * @return				请求唯一Id
 */
long RequestLiveChatController::GetVideo(
		string user_sid,
		string user_id,
		string womanId,
		string videoid,
		string inviteid,
		GETVIDEO_CLIENT_TYPE toflag,
		string sendid
		) {
	HttpEntiy entiy;
	char temp[32] = {0};

	string url = LC_GET_VIDEO_PATH;

	if( user_sid.length() > 0 ) {
		// user_sid
		entiy.AddContent(LC_GET_VIDEO_USER_SID, user_sid.c_str());
	}

	if( user_id.length() > 0 ) {
		// user_id
		entiy.AddContent(LC_GET_VIDEO_USER_ID, user_id.c_str());
	}

	// womanId
	if( womanId.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_TARGETID, womanId.c_str());
	}

	// videoid
	if( videoid.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_VIDEOID, videoid.c_str());
	}

	// inviteid
	if( inviteid.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_INVITEID, inviteid.c_str());
	}

	// type
	if( toflag >= GVCT_WOMAN && toflag <= GVCT_MAN ) {
		snprintf(temp, sizeof(temp), "%d", toflag);
		entiy.AddContent(LC_GET_VIDEO_TOFLAG, temp);
	}

	// sendid
	if( sendid.length() > 0 ) {
		entiy.AddContent(LC_GET_VIDEO_SENDID, sendid.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::GetVideo( "
			"url : %s, "
			"targetid : %s, "
			"videoid : %s, "
			"inviteid : %s, "
			"toflag : %d, "
			"sendid : %s "
			")",
			url.c_str(),
			womanId.c_str(),
			videoid.c_str(),
			inviteid.c_str(),
			toflag,
			sendid.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.15 查询小高级表情配置
 */
long RequestLiveChatController::GetMagicIconConfig(){
	HttpEntiy entiy;
	string url = LC_GET_MAGICICON_CONFIG_PATH;
	FileLog("httprequest", "RequestLiveChatController::GetMagicIconConfig( "
				"url : %s "
				")",
				url.c_str()
				);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.16.开聊自动买点（http post）
 * @param womanId		女士ID
 */
long RequestLiveChatController::ChatRecharge(
		string womanId,
		string user_sid,
		string user_id
		)
{
	HttpEntiy entiy;
	string url = LC_CHAT_RECHARGE_PATH;

	if( womanId.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_CHAT_RECHARGE_WOMAN_ID, womanId.c_str());
	}
	if( user_sid.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_CHAT_RECHARGE_USER_SID, user_sid.c_str());
	}
	if( user_id.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_CHAT_RECHARGE_USER_ID, user_id.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::ChatRecharge( "
			"url : %s, "
			"womanId : %s,"
			"user_sid : %s,"
			"user_id : %s"
			")",
			url.c_str(),
			womanId.c_str(),
			user_sid.c_str(),
			user_id.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.17.查询主题配置（http post）
 * user_sid 用户SessionId
 * user_id  用户Id
 */
long RequestLiveChatController::GetThemeConfig(
		string user_sid,
		string user_id
		)
{
	HttpEntiy entiy;
	string url = LC_GET_THEME_CONFIG_PATH;

	if( user_sid.length() > 0 ) {
		// user_sid
		entiy.AddContent(LC_GET_THEME_CONFIG_USER_SID, user_sid.c_str());
	}
	if( user_id.length() > 0 ) {
		// user_id
		entiy.AddContent(LC_GET_THEME_CONFIG_USER_ID, user_id.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::GetThemeConfig( "
			"url : %s, "
			"user_sid : %s,"
			"user_id : %s"
			")",
			url.c_str(),
			user_sid.c_str(),
			user_id.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.18.获取指定主题（http post）
 * themeIds 指定主题Id列表
 * user_sid 用户SessionId
 * user_id  用户Id
 */
long RequestLiveChatController::GetThemeDetail(
		string themeIds,
		string user_sid,
		string user_id
		){
	HttpEntiy entiy;
	string url = LC_GET_THEME_DETAIL_PATH;

	if( themeIds.length() > 0 ) {
		// themeIds
		entiy.AddContent(LC_GET_THEME_DETAIL_THEMEIDS, themeIds.c_str());
	}
	if( user_sid.length() > 0 ) {
		// user_sid
		entiy.AddContent(LC_GET_THEME_DETAIL_USER_SID, user_sid.c_str());
	}
	if( user_id.length() > 0 ) {
		// user_id
		entiy.AddContent(LC_GET_THEME_DETAIL_USER_ID, user_id.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::GetThemeDetail( "
			"url : %s, "
			"themeIds : %s, "
			"user_sid : %s,"
			"user_id : %s"
			")",
			themeIds.c_str(),
			url.c_str(),
			user_sid.c_str(),
			user_id.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

/**
 * 6.19.检测功能是否开通（http post）
 * @param functions 功能列表
 * @param deviceType 设备类型
 * @param versionCode 版本号
 * @param user_sid sessionId
 * @param user_id 用户ID
 */
long RequestLiveChatController::CheckFunctions(
		string functionIds,
		TDEVICE_TYPE deviceType,
		string versionCode,
		string user_sid,
		string user_id)
{
	HttpEntiy entiy;
	string url = LC_CHECK_FUNCTIONS_PATH;
	char temp[16];

	entiy.AddContent(LC_CHECK_FUNCTIONS_FOUNCTIONID, functionIds.c_str());

	sprintf(temp, "%d", deviceType);
	entiy.AddContent(LC_CHECK_FUNCTIONS_DEVICETYPE, temp);

	if( versionCode.length() > 0 ) {
		// versionCode
		entiy.AddContent(LC_CHECK_FUNCTIONS_VERSIONCODE, versionCode.c_str());
	}

	if( user_sid.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_CHECK_FUNCTIONS_USER_SID, user_sid.c_str());
	}
	if( user_id.length() > 0 ) {
		// womanId
		entiy.AddContent(LC_CHECK_FUNCTIONS_USERID, user_id.c_str());
	}

	FileLog("httprequest", "RequestLiveChatController::CheckFunctions( "
			"url : %s, "
			"functionIds : %s,"
			"deviceType : %d,"
			"versionCode : %s,"
			"user_sid : %s,"
			"user_id : %s"
			")",
			url.c_str(),
			functionIds.c_str(),
			deviceType,
			versionCode.c_str(),
			user_sid.c_str(),
			user_id.c_str()
			);

	return StartRequest(url, entiy, this, WebSite);
}

