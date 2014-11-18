#include "libtorrent\session.hpp"
#include "libtorrent\alert_types.hpp"
#include <set>

struct PeerInfo
{
	std::string addr;
	int         port;
};

typedef void (*HandlerHashInfo)(std::string hs_inf);

class Crawler
{
public:
	Crawler(void);
	~Crawler(void);
	void add_dht_router(std::string addr, int port);
	int init(int sPort, int ePort);
	int start();
	void stop();
	void wait();
	void set_handler(HandlerHashInfo h);
	
private:
	libtorrent::session s;
	void run();
	std::list<PeerInfo> peer_list;
	static void ThreadFun(void * lp);

	HandlerHashInfo OnRecvHashInfo;
	std::set<std::string> sha1_set;
	void AddOneTorrent(std::string sha1);
};

