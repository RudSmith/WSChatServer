#pragma once
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <thread>
#include <map>

namespace WSChat {

	class WSServer {

	public:
		WSServer();
		~WSServer();

		void start();

	private:
		void loadWSLibrary();
		void startupServer();

		void handleNewConnection();
		void handleMessage();

		void closeExistingConnection();
		void shutDown();

	};

} // namespace WSChat