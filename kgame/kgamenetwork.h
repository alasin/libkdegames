/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/
#ifndef __KGAMENETWORK_H_
#define __KGAMENETWORK_H_

#include <qstring.h>
#include <qobject.h>

class KGameIO;
class KMessageClient;
class KMessageServer;

class KGameNetworkPrivate;

/**
 * The KGameNetwork class is the KGame class with network
 * support. All other features are the same but they are
 * now network transparent. It is not used directly but
 * only via a KGame object. So you do not really have
 * to bother with this object.
 *
 * @short The main KDE game object
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 *
 */
class KGameNetwork : public QObject
{
  Q_OBJECT

public:
    /**
     * Create a KGameNetwork object
     */
    KGameNetwork(int cookie=42,QObject* parent=0);
    virtual ~KGameNetwork();

    //enum PlayerId {IdBroadcast=-1,IdClient=0};

    /**
     * Gives debug output of the game status
     **/
    virtual void Debug();

    /**
     * @return TRUE if this is a network game - i.e. you are either MASTER or
     * connected to a remote MASTER.
     **/
    bool isNetwork() const;
    
    /**
     * Is this the game MASTER (i.e. has started the @ref KMessageServer). A
     * game has always exactly one MASTER. This is either a KGame object (i.e. a
     * Client) or an own MessageServer-process. A KGame object that has the
     * MASTER status is always admin.
     *
     * You probably don't want to use this. It is a mostly internal method which
     * will probably become protected. Better use @ref isAdmin
     *
     * See also @ref isAdmin
     * @return Whether this client has started the @ref KMessageServer
     **/
    bool isMaster() const;

    /**
     * The admin of a game is the one who initializes newly connected clients
     * using @ref negotiateNetworkGame and is allowed to configure the game.
     * E.g. only the admin is allowed to use @ref KGame::setMaxPlayers.
     *
     * If one KGame object in the game is MASTER then this client is the admin
     * as well. @ref isMaster and isAdmin differ only if the @ref KMessageServer
     * is running in an own process.
     * @ref Whether this client (KGame object) is the admin
     **/
    bool isAdmin() const;

    /**
     * The unique ID of this game
     *
     * @return int id
     **/
    Q_UINT32 gameId() const;

    /**
     * Inits a network game as network MASTER. Note that if the @ref
     * KMessageServer is not yet started it will be started here (see @ref
     * setMaster). Any existing connection will be disconnected.
     *
     * If you already offer connections the port is changed.
     *
     * @param The port on which the service is offered
     * @return true if it worked
     **/
    bool offerConnections (Q_UINT16 port);

    /**
     * Inits a network game as a network CLIENT
     *
     * @param host the host to which we want to connect
     * @param port the port we want to connect to
     *
     * @return true if connected
     **/
    bool connectToServer(const QString& host, Q_UINT16 port);

    /**
     * Stops offering server connections - only for game MASTER
     * @return true
     **/
    bool stopServerConnection();

    /**
     * Changes the maximal connection number of the @ref KMessageServer to max.
     * -1 Means infinite connections are possible. Note that existing
     * connections are not affected, so even if you set this to 0 in a running
     * game no client is being disconnected. You can call this onyl if you are
     * the ADMIN!
     *
     * See also @ref KMessageServer::setMaxClients
     * @param max The maximal number of connections possible.
     **/
    void setMaxClients(int max);

    //AB: is this now internal only? Can we make it protected (maybe with
    //friends)? sendSystemMessage AND sendMessage is very confusing to the
    //user.
    /**
     * Sends a network message msg with a given msg id msgid to all clients.
     * Use this to communicate with KGame (e.g. to add a player ot to configure
     * the game - usually not necessary). 
     *
     * For your own messages use @ref sendMessage instead! This is mostly
     * internal!
     *
     * @param buffer the message which will be send. See messages.txt for contents
     * @param msgid an id for this message. See @ref
     * KGameMessage::GameMessageIds
     * @param receiver the @ref KGame / @ref KPlayer this message is for. See
     * @ref KGameMessage::calcMessageId to create this parameter
     * @param sender The @ref KGame / @ref KPlayer this message is from (i.e.
     * you). See @ref KGameMessage::calcMessageId to create this parameter. You
     * probably want to leave this 0, then KGameNetwork will create the correct
     * value for you. You might want to use this if you send a message from a
     * specific player.
     * @return true if worked
     */
    bool sendSystemMessage(const QByteArray& buffer, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendSystemMessage(int data, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendSystemMessage(const QDataStream &msg, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendSystemMessage(const QString& msg, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * Sends a network message 
     * @param error The error code
     * @param message The error message - use @ref KGameError
     * @param receiver the @ref KGame / @ref KPlayer this message is for. 0 For
     * all
     * @param sender The @ref KGame / @ref KPlayer this message is from (i.e.
     * you). You probably want to leave this 0, then KGameNetwork will create 
     * the correct value for you. You might want to use this if you send a 
     * message from a specific player.
     **/
    void sendError(int error, const QByteArray& message, Q_UINT32 receiver=0, Q_UINT32 sender=0);
    
    /**
     * Are we still offer offering server connections - only for game MASTER
     * @return true/false
     **/
    bool isOfferingConnections() const;

    /**
     * Application cookie. this idendifies the game application. It
     * help to distinguish between e.g. KPoker and KWin4
     * @return the application cookie
     **/
    int cookie() const;

    /**
     * Send a network message msg with a given message ID msgid to all clients.
     * You want to use this to send a message to the clients.
     *
     * Note that a message is always sent to ALL clients! This is necessary so
     * that all clients always have the same data and can easily be changed from
     * network to non-network without restarting the game. If you want a
     * specific @ref KGame / @ref KPlayer to react to the message use the
     * receiver and sender parameters. See @ref KGameMessage::calsMessageId
     *
     * SendMessage differs from @ref sendSystemMessage only by the msgid parameter.
     * @ref sendSystemMessage is thought as a KGame only mehtod while
     * sendMessage is for public use. The msgid paramter will be
     * +=KGameMessage::IdUser and in @ref KGame::signalNetworkData msgid will
     * be -= KGameMessage::IdUser again, so that one can easily distinguish
     * between system and user messages.
     *
     * Use @ref sendSystemMessage to comunicate with KGame (e.g. by adding a
     * player) and sendMessage for your own user message.
     *
     * Note: a player should send messages through a @ref KGameIO!
     *
     * @param buffer the message which will be send. See messages.txt for contents
     * @param msgid an id for this message. See @ref
     * KGameMessage::GameMessageIds
     * @param receiver the @ref KGame / @ref KPlayer this message is for. See
     * @ref KGameMessage::calcMessageId to create this parameter
     * @param sender The @ref KGame / @ref KPlayer this message is from (i.e.
     * you). See @ref KGameMessage::calcMessageId to create this parameter. You
     * probably want to leave this 0, then KGameNetwork will create the correct
     * value for you. You might want to use this if you send a message from a
     * specific player.
     * @return true if worked
     **/
    bool sendMessage(const QByteArray& buffer, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(const QDataStream &msg, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(const QString& msg, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(int data, int msgid, Q_UINT32 receiver=0, Q_UINT32 sender=0);


    /**
     * Called by @ref ReceiveNetworkTransmission(). Will be overwritten by
     * @ref KGame and handle the incoming message.
     **/
    virtual void networkTransmission(QDataStream&, int, Q_UINT32, Q_UINT32, Q_UINT32 clientID) = 0;


    /**
     * Disconnect the current connection and establish a new local one.
     **/
    void disconnect();


    /**
     * If you are the ADMIN of the game you can give the ADMIN status away to
     * another client. Use this e.g. if you want to quit the game or if you want
     * another client to administrate the game (note that @ref disconnect calls
     * this automatically).
     * @param clientID the ID of the new ADMIN (note: this is the _client_ID
     * which has nothing to do with the player IDs. See @ref KMessageServer)
     **/
    void electAdmin(Q_UINT32 clientID);

    /**
     * Don't use this unless you really know what youre doing! You might
     * experience some strange behaviour if you send your messages directly
     * through the @ref KMessageClient!
     *
     * @return a pointer to the @ref KMessageClient used internally to send the
     * messages. You should rather use one of the send functions!
     **/
    KMessageClient* messageClient() const;

    /**
     * Don't use this unless you really know what you are doing! You might
     * experience some strange behaviour if you use the message server directly!
     *
     * @return a pointer to the message server if this is the MASTER @ref KGame
     * object. Note that it might be possible that no @ref KGame object contains
     * the @ref KMessageServer at all! It might even run stand alone!
     **/
    KMessageServer* messageServer() const;

    /**
     * You should call this before doing thigs like, e.g. qApp->processEvents().
     * Don't forget to call @ref unlock once you are done!
     * 
     * See @ref KMessageClient::lock
     **/
    virtual void lock();
    
    /**
     * See @ref KMessageClient::unlock
     **/
    virtual void unlock();

signals:
    /**
     * A network error occured
     * @param error the error code
     * @param text the error text
     */
    void signalNetworkErrorMessage(int error, QString text);

    /**
     * Our connection to the @ref KMessageServer has broken.
     * See @ref KMessageClient::connectionBroken
     **/
    void signalConnectionBroken();

    /**
     * This signal is emitted whenever the @ref KMessageServer sends us a message that a
     * new client connected. @ref KGame uses this to call KGame::negotiateNetworkGame
     * for the newly connected client if we are admin (see @ref isAdmin)
     *
     * See @ref KMessageClient::eventClientConnected
     *
     * @param clientID the ID of the newly connected client
     **/
    void signalClientConnected(Q_UINT32 clientID);

    /**
     * This signal is emitted whenever the @ref KMessageServer sends us a message
     * that a connection to a client was detached. The second parameter can be used
     * to distinguish between network errors or removing on purpose.
     *
     * See @ref KMessageClient::eventClientDisconnected
     *
     * @param clientID the client that has disconnected
     * @param broken true if the connection was lost because of a network error, false
     *        if the connection was closed by the message server admin.
     */
    void signalClientDisconnected(Q_UINT32 clientID, bool broken);

    /**
     * This client gets or looses the admin status. See @ref
     * KMessageClient::adminStatusChanged
     * @param isAdmin True if this client gets the ADMIN status otherwise FALSE
     **/
    void signalAdminStatusChanged(bool isAdmin);

protected:
    /**
     * @internal
     * Start a @ref KMessageServer object and use it as the MASTER of the game.
     * Note that you must not call this if there is already another master
     * running!
     **/
    void setMaster();

protected slots:
    /**
     * Called by @ref KMessageClient::broadcastReceived() and will check if the
     * message format is valid. If it is not, it will generate an error (see
     * @ref signalNetworkVersionError and @ref signalNetworkErorrMessage).
     * If it is valid, the pure virtual method @ref networkTransmission() is called.
     * (This one is overwritten in KGame.)
     **/
    void receiveNetworkTransmission(const QByteArray& a, Q_UINT32 clientID);

    /**
     * This KGame object receives or looses the admin status.
     * @param isAdmin Whether we are admin or not
     **/
    void slotAdminStatusChanged(bool isAdmin);

    /**
     * Called when the network connection is about to terminate. Is used
     * to store the network parameter like the game id
     */
     void aboutToLooseConnection(Q_UINT32 id);

    /**
     * Called when the network connection is terminated. Used to clean
     * up the disconnect parameter
     */
     void slotResetConnection();


private:

private:
    KGameNetworkPrivate* d;
};

#endif
