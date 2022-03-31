#include "WSServer.h"

namespace WSChat {
	WSServer::WSServer(const std::string& ip, int port)
		: m_DLLVersion{ MAKEWORD(2, 1) },
		m_listenForNewConnection{},
		m_handleNewConnection{},
		m_users{},
		m_ip{ ip },
		m_port{ port },
		m_addr{ },
		m_wsaData{},
		m_isUp{ false }
	{ }

	void WSServer::initInput()
	{
		std::thread inputThread(&WSServer::handleInput, this);
		inputThread.join();
	}


	void WSServer::start()
	{
		if (!m_isUp)
		{
			if (startupServer() == ErrorCode::NoError)
			{
				m_isUp = true;
				std::cout << "LOG: Server started and ready to accept new connections." << std::endl;

				std::thread handleConnThread(&WSServer::handleNewConnections, this);
				handleConnThread.detach();
			}
			else
			{
				std::cout << "LOG: Server startup failure." << std::endl;
			}
		}
		else
		{
			std::cout << "LOG: Server is already up." << std::endl;
		}
	}

	void WSServer::shutDown()
	{
		if (m_isUp)
		{
			shutdown(m_handleNewConnection, SD_SEND);
			shutdown(m_listenForNewConnection, SD_SEND);
			closesocket(m_handleNewConnection);
			closesocket(m_listenForNewConnection);
			WSACleanup();

			m_isUp = false;

			std::cout << "LOG: Server is down." << std::endl;
		}
		else
		{
			std::cout << "LOG: Server is already down." << std::endl;
		}
	}


	ErrorCode WSServer::startupServer()
	{
		// Загружаем библиотеку winsock
		if (WSAStartup(m_DLLVersion, &m_wsaData) != 0)
		{
			std::cout << "LOG: Error loading winsock." << std::endl;
			return ErrorCode::StartupError;
		}

		unsigned long ipAddr = INADDR_NONE;
		ipAddr = inet_addr(m_ip.c_str());

		if (ipAddr == INADDR_NONE)
		{
			std::cout << "LOG: Wrong IP format." << std::endl;
			return ErrorCode::StartupError;
		}

		m_addr.sin_addr.s_addr = ipAddr;
		m_addr.sin_port = htons(m_port);
		m_addr.sin_family = AF_INET;

		m_listenForNewConnection = socket(AF_INET, SOCK_STREAM, NULL);
		// Привязываем к сокету его адрес
		bind(m_listenForNewConnection, (SOCKADDR*)&m_addr, sizeof(m_addr));
		// Запускаем сокет на прослушивание, указав максимальное количество подключений
		listen(m_listenForNewConnection, m_maxConnections);

		return ErrorCode::NoError;
	}

	void WSServer::handleInput()
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
			else
				std::cout << "Usage: start shutdown quit" << std::endl;
		}
	}

	void WSServer::handleNewConnections()
	{
		int addrSize = sizeof(m_addr);

		while (m_isUp == true)
		{

			if (m_users.size() < m_maxConnections)
			{
				m_handleNewConnection = accept(m_listenForNewConnection, (SOCKADDR*)&m_addr, &addrSize);

				if (m_handleNewConnection == 0)
					std::cout << "Error connecting to client." << std::endl;
				else
				{
					std::thread userThread(&WSServer::authorizeNewClient, this, m_handleNewConnection);
					userThread.detach();
				}
			}
		}
	}

	void WSServer::authorizeNewClient(SOCKET newConn)
	{
		int nicknameLen = 0;
		std::string nickname;

		// Получаем длину ника и ник
		recv(newConn, (char*)&nicknameLen, sizeof(int), NULL);
		nickname.resize(nicknameLen + 1);
		recv(newConn, (char*)nickname.data(), nicknameLen, NULL);
		nickname[nicknameLen] = '\0';

		ErrorCode error = ErrorCode::NoError;

		// Проверяем заполнение ника
		if (nicknameLen == 0)
			error = ErrorCode::EmptyNickname;

		m_usersMutex.lock();

		// Проверяем уникальность ника
		for (auto& user : m_users) {
			if (user.first == nickname)
				error = ErrorCode::NotUniqueNickname;
		}

		send(newConn, (char*)&error, sizeof(ErrorCode), NULL);

		if (error == ErrorCode::NoError)
			m_users.insert({ nickname, newConn });

		m_usersMutex.unlock();

		if (error == ErrorCode::NoError)
		{
			handleMessage(nickname);
		}
	}

	void WSServer::handleMessage(const std::string& nickname)
	{
		// Получаем длину сообщения и сообщение
		size_t msgLen;
		std::string msg;

		while (true) {
			recv(m_users.at(nickname), (char*)&msgLen, sizeof(size_t), NULL);
			msg.resize(msgLen + 1);
			recv(m_users.at(nickname), (char*)msg.data(), msgLen, NULL);
			msg[msgLen] = '\0';

			// Если пользователь закрывает подключение, удаляем запись из таблицы
			if (strcmp(msg.c_str(), "/quit") == 0)
			{
				m_usersMutex.lock();
				m_users.erase(nickname);
				m_usersMutex.unlock();
				return;
			}

			// Отправляем сообщение всем, кроме отправителя
			for (auto& user : m_users) {
				if (nickname != user.first) {
					send(user.second, (char*)&msgLen, sizeof(size_t), NULL);
					send(user.second, (char*)msg.data(), msgLen, NULL);
				}
			}
		}
	}

} // namespace WSChat