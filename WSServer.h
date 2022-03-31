#pragma once
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <map>
#include <mutex>

/*
* TODO:
* 1) отправлять ники, чтобы у всех юзеров они отображались
* 2) убрать появление блока usage при отключении сервера
* 3) расширить функционал (список пользователей, сообщение от админа, кик, бан)
* 4) при отключении сервера проверять, остались ли подключения к клиентам, и не разрывать их, а отправлять всем служебное сообщение об отключении сервера (мб добавить деструктор)
* 5) добавить комменты 
*/

namespace WSChat {

	enum class ErrorCode
	{
		NoError = 0,
		StartupError,
		NetworkError,
		EmptyNickname,
		NotUniqueNickname,
		UndefinedError
	};

	using UserList = std::map<std::string, SOCKET>;

	class WSServer {

	public:
		WSServer(const std::string &ip = "127.0.0.1", int port = 1111);
		void initInput();

	private:
		ErrorCode startupServer();

		void handleInput();

		void handleNewConnections();
		void authorizeNewClient(SOCKET newConn);
		void handleMessage(const std::string &nickname);

		void start();
		void shutDown();

		// Critical resourse
		UserList m_users;
		std::mutex m_usersMutex;

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