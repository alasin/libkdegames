/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

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

#ifndef __KGAMEPROPERTY_H_
#define __KGAMEPROPERTY_H_

#include <qdatastream.h>

#include <kdebug.h>
#include <typeinfo>

class KGame;
class KPlayer;
class KGamePropertyHandler;

/**
 * The KGamePropertyBase class is the base class of KGameProperty. See @ref
 * KGameProperty for further information.
 * 
 * @short Base class of KGameProperty
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGamePropertyBase
{
public:
	enum PropertyDataIds  { // these belong to KPlayer/KGame!
		//KPlayer
		IdGroup=1,
		IdName=2,
		IdAsyncInput=3,
		IdTurn=4,
		IdUserId=5,

		//KGame
		IdGameStatus=6,
		IdMaxPlayer=7,
		IdMinPlayer=8,

		IdCommand, // Reserved for internal use
		IdUser=256
	};

	/**
	 * Commands for advanced properties (Q_INT8)
	 **/
	enum PropertyCommandIds 
	{
		// Array
		CmdAt=1,
		CmdResize=2,
		CmdFill=3,
		CmdSort=4,
		// List (could be the same id's actually
		CmdInsert=5,
		CmdRemove=6,
		CmdClear=7
	};

	/**
	 * The policy of the property. This can be PolicyClean (@ref setVale uses
	 * @ref send), PolicyDirty (@ref setValue uses @ref changeValue) or
	 * PolicyLocal (@ref setValue uses @ref setLocal).
	 *
	 * A "clean" policy means that the property is always the same on every
	 * client. This is achieved by calling @ref send which actually changes
	 * the value only when the message from the MessageServer is received.
	 *
	 * A "dirty" policy means that as soon as @ref setValue is called the
	 * property is changed immediately. And additionally sent over network.
	 * This can sometimes lead to bugs as the other clients do not 
	 * immediately have the same value. For more information see 
	 * @ref changeValue.
	 *
	 * PolicyLocal means that a @ref KGameProperty behaves like ever
	 * "normal" variable. Whenever @ref setValue is called (e.g. using "=")
	 * the value of the property is changes immediately without sending it
	 * over network. You might want to use this if you are sure that all
	 * clients set the property at the same time.
	 **/
	enum PropertyPolicy
	{
		PolicyClean = 1,
		PolicyDirty = 2,
		PolicyLocal = 3
	};


	/**
	 * Constructs a KGamePropertyBase object and calls @ref registerData.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. 
	 * @param owner The owner of the object. Must be a KGamePropertyHandler which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KGamePropertyBase(int id, KGamePropertyHandler* owner);

	KGamePropertyBase(int id, KGame* parent);
	KGamePropertyBase(int id, KPlayer* parent);

	/**
	 * Creates a KGamePropertyBase object without an owner. Remember to call
	 * @ref registerData!
	 **/
	KGamePropertyBase();

	virtual ~KGamePropertyBase();

	/**
	 * Changes the consistency policy of a property. The @ref 
	 * PropertyPolicy is one of PolicyClean, PolicyDirty or PolicyLocal.
	 *
	 * It is up to you to decide how you want to work. 
	 **/
	void setPolicy(PropertyPolicy p) { mPolicy = p; } // TODO: bit field?

	/**
	 * @return The default policy of the property
	 **/
	PropertyPolicy policy() const { return mPolicy; }

	/**
	 * Sets this property to emit a signal on value changed.
	 * As the proerties do not inehrit QObject for optimisation
	 * this signal is emited via the KPlayer or KGame object
	 **/
	void setEmittingSignal(bool p)	{ mFlags.bits.emitsignal=p&1; }

	/**
	 * See also @ref setEmittingSignal
	 * @return Whether this property emits a signal on value change
	 **/
	bool isEmittingSignal()	const { return mFlags.bits.emitsignal; }

	/**
	 * See also @ref setReadOnly
	 * @return Whether the property is read only
	 **/
	bool isReadOnly() const { return mFlags.bits.readonly; }

	/**
	 * Sets this property to try to optimize signal and network handling
	 * by not sending it out when the property value is not changed.
	 **/
	void setOptimized(bool p) { mFlags.bits.optimize = p & 1; }

	/**
	 * See also @ref setOptimize
	 * @return Whether the property optimizes access (signals,network traffic)
	 **/
	bool isOptimized() const { return mFlags.bits.optimize; }

	/**
	 * @return Whether this property is "dirty". See alos @ref setDirty
	 **/
	bool isDirty() const { return mFlags.bits.dirty; }

	/**
	 * A readonly property cannot be changed. Use this if you to prevent a
	 * player from changing something, e.g. for a money-bases card game you
	 * will want to lock the "bet" property after a player has bet.
	 * 
	 * You have to call setReadOnly(false) before you are able to change the
	 * value of the property again. The default is not readonly.
	 * @param p True to lock this property, false to unlock it
	 **/
	void setReadOnly(bool p) { mFlags.bits.readonly = p&1; }
	
	/**
	 * This will read the value of this property from the stream. You MUST
	 * overwrite this method in order to use this class
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s) = 0;

	/**
	 * Write the value into a stream. MUST be overwritten
	 **/
	virtual void save(QDataStream& s) = 0;

	/** 
	 * send a command to advanced properties like arrays
	 **/
	virtual void command(QDataStream &, int ) { }

	/**
	 * @return The id of this property
	 **/
	int id() const { return mId; }

	/**
	 * @return a type_info of the data this property contains. This is used
	 * e.g. by @ref KGameDebugDialog
	 **/
	virtual const type_info* typeinfo() { return &typeid(this); }

	/**
	 * You have to register a KGamePropertyBase before you can use it.
	 *
	 * You MUST call this before you can use KGamePropertyBase!
	 *
	 * @param id the id of this KGamePropertyBase object. The id MUST be
	 * unique, i.e. you cannot have two properties with the same id for one
	 * player, although (currently) nothing prevents you from doing so. But
	 * you will get strange results!
	 * @param owner The owner of this data. This will send the data
	 * using @ref KPropertyHandler::sendProperty whenever you call @ref send
	 *
	 **/
	void registerData(int id, KGamePropertyHandler* owner, QString name=0);

	/** 
	 * This is an overloaded member function, provided for convenience.
	 * It differs from the above function only in what argument(s) it accepts.
	 **/
	void registerData(int id, KGame* owner, QString name=0);

	/** 
	 * This is an overloaded member function, provided for convenience.
	 * It differs from the above function only in what argument(s) it accepts.
	 **/
	void registerData(int id, KPlayer* owner, QString name=0);
 
protected:
	/**
	 * Sets the "dirty" flag of the property. If a property is "dirty" i.e.
	 * @ref KGameProperty::setLocal has been called there is no guarantee
	 * that all clients share the same value. You have to ensure this
	 * yourself e.g. by calling @ref KGameProperty::setLocal on every
	 * client. You can also ignore the dirty flag and continue working withe
	 * the property depending on your situation.
	 **/
	void setDirty(bool d) { mFlags.bits.dirty = d & 1; }

	/**
	 * Forward the data to the owner of this property which then sends it
	 * over network. @ref save is used to store the data into a stream so
	 * you have to make sure that function is working properly if you
	 * implement your own property!
	 **/
	bool sendProperty();
	
	/**
	 * @return TRUE if the message could be sent successfully, otherwise
	 * FALSE
	 **/
	bool sendProperty(const QByteArray& b);
	
	/**
	 * Causes the parent object to emit a signal on value change
	 **/
	void emitSignal();

protected:
	KGamePropertyHandler* mOwner;
	
	// Having this as a union of the bitfield and the char
	// allows us to stream this quantity easily (if we need to)
	// At the moment it is not yet transmitted
	union Flags {
		char flag;
		struct {
			// unsigned char dosave : 1;   // do save this property
			// unsigned char delaytransmit : 1;   // do not send immediately on
                                             // change but a KPlayer:QTimer
                                             // sends it later on - fast
                                             // changing variables
			unsigned char emitsignal : 1; // KPlayer notifies on variable change (true)
						      // can used 2 more
			unsigned char readonly : 1; // whether the property can be changed (false)
			unsigned char optimize : 1; // whether the property tries to optimize send/emit (false)
			unsigned char dirty: 1; // whether the property dirty (setLocal() was used)
			unsigned char cleanPolicy : 1; // whether the property is always consistent (true)
		} bits;
	} mFlags;
	
private:
	void init();
	
private:
	bool mPublic;
	int mId;

	PropertyPolicy mPolicy;
};

/**
 * Note: The entire API documentation is obsolete!
 *
 *
 * 
 * The class KGameProperty can store any form of data and will transmit it via
 * network whenver you call @ref send. This makes network transparent games
 * very easy. You first have to register the data to a @ref KGamePropertyHandler
 * using @ref KGamePropertyBase::registerData (which is called by the
 * constructor). For the @ref KGamePropertyHandler you can use @ref
 * KGame::dataHandler or @ref KPlayer::dataHandler but you can also create your
 * own data handler.
 *
 * There are several concepts you can follow when writing network games. These
 * concepts differ completely from the way how data is transferred so you should
 * decide which one to use. You can also mix these concepts for a single
 * property but we do not recommend this. The concepts:
 * <ul>
 * <li> Always Consistent (clean)
 * <li> Not Always Consistent (dirty)
 * <li> A Mixture (very dirty)
 * </ul>
 * I repeat: we do <em>not</em> recommend the third option ("a mixture"). Unless
 * you have a good reason for this you will probably introduce some hard to find
 * (and to fix) bugs.
 *
 * @sect Always consistent (clean):
 * 
 * This "policy" is default. Whenever you create a KGameProperty it is always
 * consistent. This means that consistency is the most important thing for the
 * property. This is achieved by using @ref send to change the value of the
 * property. @ref send needs a running @ref KMessageServer and therefore
 * <em>MUST</em> be plugged into a @ref KGamePropertyHandler using either @ref
 * registerData or the constructor. The parent of the dataHandler must be able
 * to send messages (see above: the message server must be running). If you use
 * @ref send to change the value of a property you won't see the effect
 * immediately: The new value is first transferred to the message server which
 * queues the message. As soon as <em>all</em> messages in the message server
 * which are before the changed property have been transferred the message
 * server delivers the new value of the KGameProperty to all clients. A @ref
 * QTimer::singleShot is used to queue the messages inside the @ref
 * KMessageServer. 
 *
 * This means that if you do the following:
 * <pre>
 * KGamePropertyInt myProperty(id, dataHandler());
 * myProperty.initData(0);
 * myProperty = 10;
 * int value = myProperty.value();
 * </pre>
 * then "value" will be "0". @ref initData is used to initialize the property
 * (e.g. when the @ref KMessageServer is not yet running or can not yet be
 * reached). This is because "myProperty = 10" or "myProperty.send(10)" send a
 * message to the @ref KMessageServer which uses @ref QTimer::singleShot to
 * queue the message. The game first has to go back into the event loop where
 * the message is received. The @ref KGamePropertyHandler receives the new value
 * sets the property. So if you need the new value you need to store it in a
 * different variable (see @ref setLocal which creates one for you until the
 * message is received). The @ref KGamePropertyHandler emits a signal (unless
 * you calles @ref setEmitSignal with false) when the new value is received:
 * @ref KGamePropertyHandler::signalPropertyChanged. You can use this to react
 * to a changed property.
 *
 * This may look quite confusing but it has a <em>big</em> advantage: all @ref
 * KGameProperty objects are ensured to have the same value on all clients in
 * the game at every time. This way you will save you a lot of trouble as
 * debugging can be very difficult if the value of a property changes
 * immediately on client A but only after one or two additianal messages
 * (function calls, status changes, ...) on client B.
 *
 * The only disadvantage of this (clean) concept is that you cannot use a
 * changed variable immediately but have to wait for the @ref KMessageServer to
 * change it. You probably want to use @ref
 * KGamePropertyHandler::signalPropertyChanged for this.
 *
 * @sect Not Always Consistent (dirty):
 * 
 * There are a lot of people who don't want to use the (sometimes quite complex)
 * "clean" way. You can use @ref setAlwaysConsistent to change the default
 * behaviour of the @ref KGameProperty. If a property is not always consistent
 * it will use @ref changeValue to send the property. @ref changeValue also uses
 * @ref send to send the new value over network but it also uses @ref
 * setLocal to create a local copy of the property. This copy is created
 * dynamically and is deleted again as soon as the next message from the network
 * is received. To use the example above again:
 * <pre>
 * KGamePropertyInt myProperty(id, dataHandler());
 * myProperty.setAlwaysConsistent(false);
 * myProperty.initData(0);
 * myProperty = 10;
 * int value = myProperty.value();
 * </pre>
 * Now this example will "work" so value now is 10. Additionally the @ref
 * KMessageServer receives a message from the local client (just as explained
 * above in "Always Consistent"). As soon as the message returns to the local
 * client again the local value is deleted, as the "network value" has the same
 * value as the local one. So you won't loose the ability to use the always
 * consistent "clean" value of the property if you use the "dirty" way. Just use
 * @ref networkValue to access the value which is consistent among all clients. 
 *
 * The advantage of this concept is clear: you can use a @ref KGameProperty as
 * every other variable as the changes value takes immediate effect.
 * Additionally you can be sure that the value is transferred to all clients.
 * You will usually not experience serious bugs just because you use the "dirty"
 * way. Several events have to happen at once to get these "strange errors"
 * which result in inconsistent properties (like "game running" on client A but
 * "game ended/paused" on client B).  But note that there is a very good reason
 * for the existence of these different concepts of @ref KGameProperty. I have
 * myself experience such a "strange error" and it took me several days to find
 * the reason until I could fix it. So I personally recommend the "clean" way.
 * On the other hand if you want to port a non-network game to a network game
 * you will probably start with "dirty" properties as it is you will not have to
 * change that much code...
 *
 * @sect A Mixture (very dirty):
 * 
 * You can also mix the concepts above. Note that we really don't recommend
 * this. With a mixture I mean something like this:
 * <pre>
 * KGamePropertyInt myProperty(id, dataHandler());
 * myProperty.setAlwaysConsistent(false);
 * myProperty.initData(0);
 * myProperty = 10;
 * myProperty.setAlwaysConsistent(true);
 * myProperty = 20;
 * </pre>
 * (totally senseless example, btw) I.e. I am speaking of mixing both concepts
 * for a single property. Things like
 * <pre>
 * KGamePropertyInt myProperty1(id1, dataHandler());
 * KGamePropertyInt myProperty2(id2, dataHandler());
 * myProperty1.initData(0);
 * myProperty2.initData(0);
 * myProperty1.setAlwaysConsistent(false);
 * myProperty2.setAlwaysConsistent(true);
 * myProperty1 = 10;
 * myProperty2 = 20;
 * </pre>
 * are ok. But mixing the concepts for a single property will make it nearly
 * impossible to you to debug your game. 
 *
 * So the right thing to do(tm) is to decide in the constructor whether you want
 * a "clean" or "dirty" property. 
 *
 * Even if you have decided for one of the concepts you still can manually
 * follow another concept than the "policy" of your property. So if you use an
 * always consistent @ref KGameProperty you still can manually call @ref
 * changeValue as if it was not always consistent. Note that although this is
 * also kind of a "mixture" as described above this is very useful sometimes. In
 * contrast to the "mixture" above you don't have the problem that you don't
 * exactly know which concept you are currently following because you used the
 * function of the other concept only once. 
 *
 * @sect Custom classes:
 *
 * If you want to use a custum class with KGameProperty you have to implement the
 * operators << and >> for QDataStream:
 * <pre>
 * class Card
 * {
 * public:
 * int type;
 * int suite;
 * };
 * QDataStream& operator<<(QDataStream& stream, Card& card)
 * {
 * Q_INT16 type = card.type;
 * Q_INT16 suite = card.suite;
 * s << type;
 * s << suite;
 * return s;
 * }
 * QDataStream& operator>>(QDataStream& stream, Card& card)
 * {
 * Q_INT16 type;
 * Q_INT16 suite;
 * s >> type;
 * s >> suite;
 * card.type = (int)type;
 * card.suite = (int)suite;
 * return s;
 * }
 *
 * class Player : KPlayer
 * {
 * [...]
 * KGameProperty<Card> mCards;
 * };
 * </pre>
 *
 *
 * Note: unlike most QT classes KGameProperty objects are *not* deleted
 * automatically! So if you create an object using e.g. KGameProperty<int>* data =
 * new KGameProperty(id, dataHandler()) you have to put a delete data into your
 * destructor!
 * @short A class for network transparent games
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
template<class type>
class KGameProperty  : public KGamePropertyBase
{
public:
	/**
	 * Constructs a KGameProperty object. A KGameProperty object will transmit
	 * any changes to the @ref KMessageServer and then to all clients in the
	 * game (including the one that has sent the new value)
	 * @param id The id of this property. <em>MUST be UNIQUE</em>! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. 
	 * @param parent The parent of the object. Must be a KGame which manages
	 * the changes made to this object, i.e. which will send the new data.
	 * Note that in contrast to most KDE/QT classes KGameProperty objects
	 * are <em>not</em> deleted automatically!
	 **/
// TODO: ID: Very ugly - better use something like parent()->propertyId() or so which assigns a free id automatically.
	KGameProperty(int id, KGamePropertyHandler* owner) : KGamePropertyBase(id, owner) { init(); }

	/**
	 * This constructor does nothing. You have to call @ref
	 * KGamePropertyBase::registerData
	 * yourself before using the KGameProperty object.
	 **/
	KGameProperty() : KGamePropertyBase() { init(); }

	virtual ~KGameProperty() {}

	/**
	 * Set the value depending on the current policy (see @ref
	 * setConsistent). By default KGameProperty just uses @ref send to set
	 * the value of a property. This behaviour can be changed by using @ref
	 * setConsistent.
	 **/
	void setValue(type v)
	{
		switch (policy()) {
			case PolicyClean:
				send(v);
				break;
			case PolicyDirty:
				changeValue(v);
				break;
			case PolicyLocal:
				setLocal(v);
				break;
			default: // NEVER!
				return;
		}
	}


	/**
	 * This function sends a new value over network.
	 *
	 * Note that the value DOES NOT change when you call this function. This
	 * function saves the value into a @ref QDataStream and calls @ref
	 * sendProperty where it gets forwarded to the owner and finally the
	 * value is sent over network. The @ref KMessageServer now send the
	 * value to ALL clients - even the one who called this function. As soon
	 * as the value from the message server is received @ref load is called
	 * and _then_ the value of the KGameProperty has been set.
	 *
	 * This ensures that a KGameProperty has _always_ the same value on
	 * _every_ client in the network. Note that this means you can NOT do
	 * something like
	 * <pre>
	 * myProperty = 1;
	 * doSomething(myProperty);
	 * </pre>
	 * as myProperty has not yet been set when doSomething is being called.
	 * You are informed about a value change by a singal from the parent of
	 * the property which can be deactivated by @ref setEmittingSignal because of
	 * performance (you probably don't have to deactivate it - except you
	 * want to write a real-time game like Command&Conquer with a lot of
	 * acitvity). See @ref emitSignal
	 * 
	 * @param sendOnly If FALSE and the value couldn't be sent by any reason
	 * (e.g. because no owner has been set who can send the value) then the
	 * local value is set using @ref setLocal. If TRUE then the value is
	 * sent only and nothing is done if sending is not possible
	 * @return whether the property could be sent successfully
	 * @see setValue setLocal changeValue value networkValue
	 **/
	bool send(type v)
	{
		if (isOptimized() && mData == v) {
			return true;
		}
		if (isReadOnly()) {
			return false;
		}
		QByteArray b;
		QDataStream stream(b, IO_WriteOnly);
		stream << v;
		return sendProperty(b);
	}

	/**
	 * This function is a workaround for a big problem of KProperty: if you
	 * do e.g.
	 * <pre>
	 * myProperty = 1;
	 * doSomething(myProperty);
	 * </pre>
	 * then this will fail. The myProperty = 1 part calls @ref send which
	 * sends the value over network. But it doesn't actually set the value
	 * of the property - this is done as soon as the @ref KMessageServer
	 * sends this message to this client. So doSomething(myProperty) still
	 * contains the old value (i.e. NOT 1).
	 *
	 * To solve this problem you can call setLocal(). This function creates
	 * a duplicated version of the property which is ONLY locally available.
	 * The value is NOT sent over network. 
	 *
	 * As soon as a value over network is received the local duplication is
	 * deleted again. Note that this behaviour might be disturbing or even
	 * confusing!
	 *
	 * If you want to set the value locally AND send it over network you
	 * want to call @ref changeValue!
	 *
	 * @see setValue send changeValue value networkValue
	 **/
	void setLocal(type v) 
	{
		if (!isOptimized() || mData != v) {
			if (isReadOnly()) {
				return;
			}
			mData = v;
			setDirty(true);
			if (isEmittingSignal()) {
				emitSignal();
			}
		}
	}

	/**
	 * This function does both, change the local value and change the
	 * network value. The local value is created/changed first, then the
	 * value is sent over network.
	 *
	 * This function is a convenience function and just calls @ref setLocal
	 * followed by @ref send
	 *
	 * @see send setLocal setValue value networkValue
	 **/
	void changeValue(type v)
	{
		send(v);
		setLocal(v);
	}

	/**
	 * Saves the object to a stream.
	 * @ref stream The stream to save to
	 **/
	virtual void save(QDataStream &stream)
	{
		stream << mData;
	}

	/**
	 * @return The local value (see @ref setLocal) if it is existing,
	 * otherwise the network value which is always consistent on every
	 * client.
	 **/
	const type& value() const
	{
		return mData;
	}

	/**
	 * Reads from a stream and assigns the read value to this object. This
	 * also deletes an existing local value (see @ref setLocal)
	 *
	 * This function is called automatically when a new value is received
	 * over network (i.e. it has been sent using @ref send on this or any
	 * other client) or when a game is loaded (and maybe on some other
	 * events).
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s)
	{
		s >> mData;
		setDirty(false);
		if (isEmittingSignal()) {
			emitSignal();
		}
	}

	/**
	 * This calls @ref setValue to change the value of the property. Note
	 * that depending on the policy (see @ref setAlwaysConsistent) the
	 * returned value might be different from the assigned value!!
	 *
	 * So if you use @ref setAlwaysConsistent(false):
	 * <pre>
	 * int a, b = 10;
	 * myProperty.initData(0);
	 * myProperty = b;
	 * a = myProperty.value();
	 * </pre>
	 * Here a and b would differ!
	 * The value is actually set as soon as it is received form the @ref
	 * KMessageServer which forwards it to ALL clients in the network.
	 *
	 * If you use a clean policy (i.e. @ref isAlwaysConsistent == true) then
	 * the returned value is the assigned value
	 * @return See @ref value
	 **/
	const type& operator=(const type& t) 
	{ 
		setValue(t); 
		return value();
	}

	/**
	 * Yeah, you can do it!
	 * <pre>
	 * 	KGameProperty<int> integerData(0, owner);
	 * 	integerData.setValue(100);
	 * 	kdDebug(11001) << integerData << endl;
	 * </pre>
	 * If you don't see it: you don't have to use integerData.value()
	 * @return See @ref value
	 **/
	operator type() const { return value(); }

	virtual const type_info* typeinfo() { return &typeid(type); }

private:
	void init()
	{
	}
		
private:
	type mData;
};


typedef KGameProperty<int>   KGamePropertyInt;
typedef KGameProperty<unsigned int>   KGamePropertyUInt;
typedef KGameProperty<QString>   KGamePropertyQString;
typedef KGameProperty<Q_INT8>   KGamePropertyBool;

#endif
