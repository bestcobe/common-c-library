/*
 * HttpRequestHostManager.h
 *
 *  Created on: 2015-3-12
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef HTTPREQUESTHOSTMANAGER_H_
#define HTTPREQUESTHOSTMANAGER_H_

#include <string>
using namespace std;

#include <common/KLog.h>

typedef enum SiteType {
	AppSite,
	WebSite,
	ChatVoiceSite,
    FakeSite,
} SiteType;

class HttpRequestHostManager {
public:
	HttpRequestHostManager();
	virtual ~HttpRequestHostManager();

	string GetHostByType(SiteType type);
	void SetWebSite(string webSite);
	string GetWebSite();
	void SetAppSite(string appSite);
	string GetAppSite();
	void SetChatVoiceSite(string chatVoiceSite);
	string GetChatVoiceSite();
    void SetFakeSite(string fakeSite);
    string GetFakeSite();
    
private:
	string mWebSite;
	string mAppSite;
	string mChatVoiceSite;
    string mFakeSite;
};

#endif /* HTTPREQUESTHOSTMANAGER_H_ */
