/*
 * lobby.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include <QTcpSocket>
#include <QAbstractSocket>

const unsigned int ProtocolVersion = 1;
const std::string ProtocolEncoding = "utf8";

class ProtocolError: public std::runtime_error
{
public:
	ProtocolError(const char * w): std::runtime_error(w) {}
};

enum ProtocolConsts
{
	//client consts
	GREETING, USERNAME, MESSAGE, VERSION, CREATE, JOIN, LEAVE, READY,

	//server consts
	SESSIONS, CREATED, JOINED, KICKED, SRVERROR, CHAT, START, STATUS, HOST
};

const QMap<ProtocolConsts, QString> ProtocolStrings
{
	//client consts
	{GREETING, "%1<GREETINGS>%2<VER>%3"}, //protocol_version byte, encoding bytes, encoding, name, version
	{USERNAME, "<USER>%1"},
	{MESSAGE, "<MSG>%1"},
	{CREATE, "<NEW>%1<PSWD>%2<COUNT>%3"},
	{JOIN, "<JOIN>%1<PSWD>%2"},
	{LEAVE, "<LEAVE>%1"}, //session
	{READY, "<READY>%1"}, //session

	//server consts
	{CREATED, "CREATED"},
	{SESSIONS, "SESSIONS"}, //amount:session_name:joined_players:total_players:is_protected
	{JOINED, "JOIN"}, //session_name:username
	{KICKED, "KICK"}, //session_name:username
	{START, "START"}, //session_name:uuid
	{HOST, "HOST"}, //host_uuid:players_count
	{STATUS, "STATUS"}, //joined_players:player_name:is_ready
	{SRVERROR, "ERROR"},
	{CHAT, "MSG"} //username:message
};

class ServerCommand
{
public:
	ServerCommand(ProtocolConsts, const QStringList & arguments);

	const ProtocolConsts command;
	const QStringList arguments;
};

class SocketLobby : public QObject
{
	Q_OBJECT
public:
	explicit SocketLobby(QObject *parent = 0);
	void connectServer(const QString & host, int port, const QString & username);
	void disconnectServer();
	void requestNewSession(const QString & session, int totalPlayers, const QString & pswd);
	void requestJoinSession(const QString & session, const QString & pswd);
	void requestLeaveSession(const QString & session);
	void requestReadySession(const QString & session);

	void send(const QString &);

signals:

	void text(QString);
	void receive(QString);
	void disconnect();

public slots:

	void connected();
	void disconnected();
	void bytesWritten(qint64 bytes);
	void readyRead();

private:
	QTcpSocket *socket;
	bool isConnected = false;
	QString username;

};
