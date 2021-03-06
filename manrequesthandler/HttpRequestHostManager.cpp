/*
 * HttpRequestHostManager.cpp
 *
 *  Created on: 2015-3-12
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "HttpRequestHostManager.h"
#include <common/CheckMemoryLeak.h>

HttpRequestHostManager::HttpRequestHostManager() {
	// TODO Auto-generated constructor stub

}

HttpRequestHostManager::~HttpRequestHostManager() {
	// TODO Auto-generated destructor stub
}

string HttpRequestHostManager::GetHostByType(SiteType type) {
	string host = "";

	switch( type ) {
	case AppSite:{
		host = GetAppSite();
	}break;
	case WebSite:{
		host = GetWebSite();
	}break;
	case ChatVoiceSite:{
		host = GetChatVoiceSite();
	}break;
    case FakeSite:{
        host = GetFakeSite();
    }break;
	default:break;
	}

	FileLog("httprequest", "HttpRequestHostManager::GetHostByType( "
			"type : %d, "
			"host : %s "
			")",
			type,
			host.c_str()
			);

	return host;
}

void HttpRequestHostManager::SetWebSite(string webSite) {
	mWebSite = webSite;
}

string HttpRequestHostManager::GetWebSite() {
	return mWebSite;
}

void HttpRequestHostManager::SetAppSite(string appSite) {
	mAppSite = appSite;
}

void HttpRequestHostManager::SetChatVoiceSite(string chatVoiceSite) {
	mChatVoiceSite = chatVoiceSite;
}

string HttpRequestHostManager::GetChatVoiceSite() {
	return mChatVoiceSite;
}

string HttpRequestHostManager::GetAppSite() {
	return mAppSite;
}

void HttpRequestHostManager::SetFakeSite(string fakeSite) {
    mFakeSite = fakeSite;
}

string HttpRequestHostManager::GetFakeSite() {
    return mFakeSite;
}
