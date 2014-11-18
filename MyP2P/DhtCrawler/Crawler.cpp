#include <process.h>
#include "Crawler.h"


Crawler::Crawler(void)
{
	OnRecvHashInfo = NULL;
}


Crawler::~Crawler(void)
{
}

void Crawler::add_dht_router(std::string addr, int port)
{
	PeerInfo inf;
	inf.addr = addr;
	inf.port = port;
	peer_list.push_back(inf);
}

int Crawler::init(int sPort, int ePort)
{
	if (peer_list.size() <= 0)
	{
		return -1;
	}

	libtorrent::error_code ec;
	s.set_alert_mask(libtorrent::alert::all_categories);
	s.listen_on(std::make_pair(sPort, ePort), ec);
	if (ec)
	{
		//fprintf(stderr, "failed to open listen socket: %s\n", ec.message().c_str());
		return -1;
	}

	std::list<PeerInfo>::iterator it;
	for (it = peer_list.begin(); it != peer_list.end(); it++)
	{
		s.add_dht_router(std::make_pair(it->addr, it->port));
	}

	s.start_dht();

	return 0;
}

void Crawler::run()
{
	AddOneTorrent("46CF7770781B3A28C02403956688F6C4D589C3A3");

	libtorrent::time_duration td(3000);
	while(1)
	{
		const libtorrent::alert *alt = s.wait_for_alert(td);
		if (alt == NULL)
		{
			continue;
		}

		std::deque<libtorrent::alert*> alerts;
		s.pop_alerts(&alerts);

		std::deque<libtorrent::alert*>::iterator it;
		for (it = alerts.begin(); it != alerts.end(); it++)
		{
			switch((*it)->type())
			{
			case libtorrent::dht_announce_alert::alert_type:
				{
					libtorrent::dht_announce_alert * alt = (libtorrent::dht_announce_alert *)(*it);
					std::string infohash = alt->info_hash.to_string();
					std::string inf_hex = libtorrent::to_hex(infohash);
					AddOneTorrent(inf_hex);
					//if (OnRecvHashInfo != NULL)
					//{
					//	OnRecvHashInfo(inf_hex);
					//}
				}
				break;

			case libtorrent::dht_get_peers_alert::alert_type:
				{
					libtorrent::dht_get_peers_alert * alt = (libtorrent::dht_get_peers_alert *)(*it);
					std::string infohash = alt->info_hash.to_string();
					std::string inf_hex = libtorrent::to_hex(infohash);
					AddOneTorrent(inf_hex);
					//if (OnRecvHashInfo != NULL)
					//{
					//	OnRecvHashInfo(inf_hex);
					//}
				}
				break;

			case libtorrent::metadata_received_alert::alert_type:
				{
					libtorrent::metadata_received_alert * alt = (libtorrent::metadata_received_alert *)(*it);

					libtorrent::torrent_handle h = alt->handle;
					if (h.is_valid()) 
					{
						boost::intrusive_ptr<libtorrent::torrent_info const> ti = h.torrent_file();
						//create_torrent ct(*ti);
						//entry te = ct.generate();
						//std::vector<char> buffer;
						//bencode(std::back_inserter(buffer), te);
						//FILE* f = fopen((to_hex(ti->info_hash().to_string()) + ".torrent").c_str(), "wb+");
						//if (f) {
						//	fwrite(&buffer[0], 1, buffer.size(), f);
						//	fclose(f);
						//}

						//ti->info()
						//printf("tname = %s, tsize = %d", h.name().c_str(), 123);

						printf("tname = %s, tsize = %d\n", ti->name().c_str(), ti->total_size());

					}

					s.remove_torrent(h, libtorrent::session::delete_files);
				}
				break;

			default:

				//printf("category:%d, alert_type:%d\r\n", (*it)->category(), (*it)->type());

				break;
			}

			delete *it;
		}


	}
}

int Crawler::start()
{
	_beginthread(ThreadFun, 0, this);
	return 0;
}

void Crawler::ThreadFun(void * lp)
{
	Crawler * cler = (Crawler*)lp;
	cler->run();
}

void Crawler::set_handler(HandlerHashInfo h)
{
	OnRecvHashInfo = h;
}

void Crawler::AddOneTorrent(std::string sha1)
{
	std::set<std::string>::iterator it = sha1_set.find(sha1);
	if (it != sha1_set.end())
	{
		return;
	}
	
	libtorrent::error_code ec;
	libtorrent::add_torrent_params p;
	p.save_path = ".\\tmp\\";
	p.url = "magnet:?xt=urn:btih:" + sha1;
	libtorrent::torrent_handle h = s.add_torrent(p, ec);

	if (ec != 0)
	{
		printf("add_torrent fail,ec=%d\n", ec.value());
		return;
	}

	sha1_set.insert(sha1);
	printf("add %s, sha1_set.size() = %d\n", sha1.c_str(), sha1_set.size());
}
