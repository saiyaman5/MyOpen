// DhtCrawler.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Crawler.h"
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <iostream>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include "pugixml.hpp"

boost::shared_mutex g_mutex;
std::set<std::string> hasinf_list;
int nCrlerCount;
int sPort,ePort;
std::vector<PeerInfo> vPeerInfo;
std::string strWorkPath;  
std::string strTmpPath; 
std::string strDataPath; 
int loadConfig();
int savefile();

void OnRecv(std::string hash_inf)
{
	boost::unique_lock<boost::shared_mutex> lock(g_mutex);
	hasinf_list.insert(hash_inf);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (loadConfig() != 0)
	{
		return -1;
	}

	Crawler *crler_array = new Crawler[nCrlerCount];
	int failCount = 0;
	for (int i = 0; i < nCrlerCount; i++)
	{
		for (int j = 0; j < vPeerInfo.size(); j++)
		{
			crler_array[i].add_dht_router(vPeerInfo[j].addr, vPeerInfo[j].port);
		}

		crler_array[i].set_handler(OnRecv);

		if (crler_array[i].init(sPort + i, ePort) == 0)
		{
			crler_array[i].start();
		}
		else
		{
			failCount += 1;
		}
	}

	printf("nCrlerCount=%d, success=%d, fail=%d\r\n", nCrlerCount, nCrlerCount - failCount, failCount);

	//while(1)
	//{
	//	printf("HashInfo Count:%d\r\n", hasinf_list.size());
	//	savefile();
	//	Sleep(60000);
	//}

	getchar();
}

int loadConfig()
{
	pugi::xml_document doc;  
	if (!doc.load_file("DHTCrawler.xml")) 
	{
		return -1;
	}

	pugi::xml_node ndCount = doc.child("root").child("CrlerCount");
	nCrlerCount = atoi(ndCount.child_value());

	pugi::xml_node ndsPort = doc.child("root").child("sPort");
	sPort = atoi(ndsPort.child_value());

	pugi::xml_node ndePort = doc.child("root").child("ePort");
	ePort = atoi(ndePort.child_value());

	pugi::xml_node ndRouter = doc.child("root").child("routers").child("router");
	while(ndRouter)
	{
		PeerInfo pi;
		pi.addr = ndRouter.attribute("addr").value();
		pi.port = atoi(ndRouter.attribute("port").value());
		vPeerInfo.push_back(pi);

		ndRouter = ndRouter.next_sibling("router");
	}

	pugi::xml_node ndWorkPath = doc.child("root").child("workPath");
	strWorkPath = ndWorkPath.child_value();
	if (strWorkPath == "")
	{
		strWorkPath = ".\\";
	}
	else
	{
		if (strWorkPath[strWorkPath.size() -1] != '\\')
		{
			strWorkPath += "\\";
		}
	}

	strTmpPath = strWorkPath + "tmp\\";
	strDataPath = strWorkPath + "data\\";

	if (_access(strTmpPath.c_str(),0) == -1)
	{
		_mkdir(strTmpPath.c_str());
	}

	if (_access(strDataPath.c_str(),0) == -1)
	{
		_mkdir(strDataPath.c_str());
	}


	return 0;
}

int savefile()
{
	boost::shared_lock<boost::shared_mutex> lock(g_mutex, boost::defer_lock);

	lock.lock();
	std::vector<std::string> vhash_inf(hasinf_list.size());
	std::copy(hasinf_list.begin(), hasinf_list.end(), vhash_inf.begin());
	hasinf_list.clear();
	lock.unlock();

	if (vhash_inf.size() <= 0)
	{
		return 0;
	}
	
	char buf[64] = {0};
	_i64toa(time(0), buf, 10);

	std::string fileName = strTmpPath + buf + std::string(".t");
	std::string fileName2 = strDataPath + buf + std::string(".t");

	FILE *f = fopen(fileName.c_str(), "w+");
	if (f == NULL)
	{
		return -1;
	}
	
	for (int i = 0; i < vhash_inf.size(); i++)
	{
		fprintf(f, "%s|", vhash_inf[i].c_str());
	}

	fclose(f);

	MoveFileA(fileName.c_str(), fileName2.c_str());

	return 0;
}
