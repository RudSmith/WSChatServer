#include "WSServer.h"

WSChat::WSServer::WSServer(const std::string& ip, int port)
	: m_DLLVersion{MAKEWORD(2, 1)},
	  m_listenForNewConnection{},
	  m_handleNewConnection{},
	  m_users{},
	  m_ip { ip },
	  m_port { port },
	  m_addr { },
	  m_wsaData {},
	  m_isUp { false }
{
	std::thread inputThread (&WSServer::handleInput, this);
	inputThread.join();
}


void WSChat::WSServer::start()
{
	if (startupServer() != WSChat::ErrorCode::NoError)
		return;

	m_isUp = true;
	std::cout << "LOG: Server started and ready to accept new connections. \n";

	std::thread handleConnThread(&WSServer::handleNewConnections, this);
	handleConnThread.detach();
}

void WSChat::WSServer::shutDown()
{
	shutdown(m_handleNewConnection, SD_SEND);
	shutdown(m_listenForNewConnection, SD_SEND);
	closesocket(m_handleNewConnection);
	closesocket(m_listenForNewConnection);
	WSACleanup();

	m_isUp = false;

	std::cout << "LOG: Server is down. \n";
}


WSChat::ErrorCode WSChat::WSServer::startupServer()
{
	// Загружаем библиотеку winsock
	if (WSAStartup(m_DLLVersion, &m_wsaData) != 0)
	{
		std::cout << "LOG: Error loading winsock.\n";
		return WSChat::ErrorCode::StartupError;
	}

	unsigned long ipAddr = INADDR_NONE;
	ipAddr = inet_addr(m_ip.c_str());

	if (ipAddr == INADDR_NONE)
	{
		std::cout << "LOG: Wrong IP format.\n";
		return WSChat::ErrorCode::StartupError;
	}

	m_addr.sin_addr.s_addr = ipAddr;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_family = AF_INET;

	m_listenForNewConnection = socket(AF_INET, SOCK_STREAM, NULL);
	// Привязываем к сокету его адрес
	bind(m_listenForNewConnection, (SOCKADDR*)&m_addr, sizeof(m_addr));
	// Запускаем сокет на прослушивание, указав максимальное количество подключений
	listen(m_listenForNewConnection, m_maxConnections);

	return WSChat::ErrorCode::NoError;
}

void WSChat::WSServer::handleInput()
{
	std::string command = "";

	while (command != "quit") {
		std::cout << "WSServer: ";
		std::cin >> command;

		if (command == "start") {
			start();
		}
		else if (command == "shutdown") {
			shutDown();
		}
	}
}

void WSChat::WSServer::handleNewConnections()
{
	int addrSize = sizeof(m_addr);
	char msg[256];

	while (m_isUp == true) 
	{
		if (m_users.size() < m_maxConnections) 
		{
			m_handleNewConnection = accept(m_listenForNewConnection, (SOCKADDR*)&m_addr, &addrSize);

			if (m_handleNewConnection == 0)
				std::cout << "Error connecting to client.\n";
			else 
			{
				recv(m_handleNewConnection, msg, 255, NULL);
				std::cout << msg;
			}

		}
	}
}

