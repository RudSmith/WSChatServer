#pragma once
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <thread>
#include <map>

namespace WSChat {

	enum class ErrorCode
	{
		NoError = 0,
		StartupError,
		NetworkError,
		UndefinedError
	};

	using UserList = std::vector<std::pair<std::string, SOCKET>>;

	class WSServer {

	public:
		WSServer(const std::string &ip = "127.0.0.1", int port = 1111);

	private:
		ErrorCode startupServer();

		void handleInput();
		void handleNewConnections();
		void handleMessage();

		void closeExistingConnection();

		void start();
		void shutDown();

		UserList m_users;

		WSAData m_wsaData;
		WORD m_DLLVersion;

		std::string m_ip;
		int m_port;

		SOCKADDR_IN m_addr;
		SOCKET m_listenForNewConnection;
		SOCKET m_handleNewConnection;

		bool m_isUp;

		const unsigned short m_maxConnections = 16;

	};

} // namespace WSChat