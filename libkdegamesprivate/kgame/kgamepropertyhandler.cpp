/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgamepropertyhandler.h"
#include "kgameproperty.h"
#include "kgamemessage.h"

#include <QMap>
#include <QQueue>

#include <klocale.h>
#include <typeinfo>

#define KPLAYERHANDLER_LOAD_COOKIE 6239

//---------------------- KGamePropertyHandler -----------------------------------
class KGamePropertyHandlerPrivate
{
public:
  KGamePropertyHandlerPrivate(KGamePropertyHandler *qq)
      : q(qq), mUniqueId(KGamePropertyBase::IdAutomatic), mId(0),
        mDefaultPolicy(KGamePropertyBase::PolicyLocal), mDefaultUserspace(true),
        mIndirectEmit(0)
  {
    //qDebug() << ": this=" << q;
  }

  KGamePropertyHandler *q;
        QMap<int, QString> mNameMap;
  QMultiHash<int, KGamePropertyBase*> mIdDict;
        int mUniqueId;
        int mId;
        KGamePropertyBase::PropertyPolicy mDefaultPolicy;
        bool mDefaultUserspace;
  int mIndirectEmit;
  QQueue<KGamePropertyBase*> mSignalQueue;
};

KGamePropertyHandler::KGamePropertyHandler(int id, const QObject* receiver, const char * sendf, const char *emitf, QObject* parent)
    : QObject(parent), d(new KGamePropertyHandlerPrivate(this))
{
 registerHandler(id,receiver,sendf,emitf);
}

KGamePropertyHandler::KGamePropertyHandler(QObject* parent)
    : QObject(parent), d(new KGamePropertyHandlerPrivate(this))
{
}

KGamePropertyHandler::~KGamePropertyHandler()
{
 //qDebug() ;
 clear();
 delete d;
 //qDebug() << "done";
}

int KGamePropertyHandler::id() const
{
 return d->mId;
}

void KGamePropertyHandler::setId(int id)
{
 d->mId = id;
}

void KGamePropertyHandler::registerHandler(int id,const QObject * receiver, const char * sendf, const char *emitf)
{
 setId(id); 
 if (receiver && sendf) {
        connect(this, SIGNAL(signalSendMessage(int,QDataStream&,bool*)), receiver, sendf);
 }
 if (receiver && emitf) {
        connect(this, SIGNAL(signalPropertyChanged(KGamePropertyBase*)), receiver, emitf);
 }
}

bool KGamePropertyHandler::processMessage(QDataStream &stream, int id, bool isSender)
{
// qDebug() << ": id=" << id << "mId=" << d->mId;
 if (id != d->mId) {
        return false; // Is the message meant for us?
 }
 KGamePropertyBase* p;
 int propertyId;
 KGameMessage::extractPropertyHeader(stream, propertyId);
// qDebug() << ": Got property" << propertyId;
 if (propertyId==KGamePropertyBase::IdCommand) {
        int cmd;
        KGameMessage::extractPropertyCommand(stream, propertyId, cmd);
//qDebug() << ": Got COMMAND for id= "<<propertyId;
  QMultiHash<int, KGamePropertyBase*>::iterator it = d->mIdDict.find(propertyId);
  if (it != d->mIdDict.end()) {
    p = *it;
                if (!isSender || p->policy()==KGamePropertyBase::PolicyClean) {
                        p->command(stream, cmd, isSender);
                }
        } else {
                qCritical() << ": (cmd): property" << propertyId << "not found";
        }
        return true;
 }
 QMultiHash<int, KGamePropertyBase*>::iterator it = d->mIdDict.find(propertyId);
 if (it != d->mIdDict.end()) {
  p = *it;
        //qDebug() << ": Loading" << propertyId;
        if (!isSender || p->policy()==KGamePropertyBase::PolicyClean) {
                p->load(stream);
        }
 } else {
        qCritical() << ": property" << propertyId << "not found";
 }
 return true;
}

bool KGamePropertyHandler::removeProperty(KGamePropertyBase* data)
{
  if (!data) {
         return false;
  }
  
  d->mNameMap.remove(data->id());
  return d->mIdDict.remove(data->id());
}

bool KGamePropertyHandler::addProperty(KGamePropertyBase* data, const QString& name)
{
 //qDebug() << ":" << data->id();
 if ( d->mIdDict.find(data->id()) != d->mIdDict.end() ) {
        // this id already exists
        qCritical() << "  -> cannot add property" << data->id();
        return false;
 } else {
        d->mIdDict.insert(data->id(), data);
  // if here is a check for "is_debug" or so we can add the strings only in debug mode
  // and save memory!!
        if (!name.isNull()) {
                d->mNameMap[data->id()] = name;
                //qDebug() << ": nid="<< (data->id()) << "inserted in Map name=" << d->mNameMap[data->id()];
                //qDebug() << "Typeid=" << typeid(data).name();
        //qDebug() << "Typeid call=" << data->typeinfo()->name();
        }
 }
 return true;
}

QString KGamePropertyHandler::propertyName(int id) const
{
 QString s;
 if (d->mIdDict.find(id) != d->mIdDict.end()) {
        if (d->mNameMap.contains(id)) {
                s = i18n("%1 (%2)", d->mNameMap[id], id);
        } else {
                s = i18n("Unnamed - ID: %1", id);
        }
 } else {
        // Should _never_ happen
        s = i18np("%1 unregistered", "%1 unregistered", id);
 }
 return s;
}

bool KGamePropertyHandler::load(QDataStream &stream)
{
 // Prevent direct emmiting until all is loaded
 lockDirectEmit();
 uint count,i;
 stream >> count;
 qDebug() << ":" << count << "KGameProperty objects";
 for (i = 0; i < count; ++i) {
        processMessage(stream, id(),false);
 }
 qint16 cookie;
 stream >> cookie;
 if (cookie == KPLAYERHANDLER_LOAD_COOKIE) {
        qDebug() << "   KGamePropertyHandler loaded propertly";
 } else {
        qCritical() << "KGamePropertyHandler loading error. probably format error";
 }
 // Allow direct emmiting (if no other lock still holds)
 unlockDirectEmit();
 return true;
}

bool KGamePropertyHandler::save(QDataStream &stream)
{
 qDebug() << ":" << d->mIdDict.count() << "KGameProperty objects";
 stream << (uint)d->mIdDict.count();
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        KGamePropertyBase *base=it.value();
        if (base) {
                KGameMessage::createPropertyHeader(stream, base->id());
                base->save(stream);
        }
 }
 stream << (qint16)KPLAYERHANDLER_LOAD_COOKIE;
 return true;
}

KGamePropertyBase::PropertyPolicy KGamePropertyHandler::policy()
{
// qDebug() << ":" << d->mDefaultPolicy;
 return d->mDefaultPolicy;
}
void KGamePropertyHandler::setPolicy(KGamePropertyBase::PropertyPolicy p,bool userspace)
{
 // qDebug() << ":" << p;
 d->mDefaultPolicy=p;
 d->mDefaultUserspace=userspace;
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        if (!userspace || it.value()->id()>=KGamePropertyBase::IdUser) {
                it.value()->setPolicy((KGamePropertyBase::PropertyPolicy)p);
        }
 }
}

void KGamePropertyHandler::unlockProperties()
{
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        it.value()->unlock();
 }
}

void KGamePropertyHandler::lockProperties()
{
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        it.value()->lock();
 }
}

int KGamePropertyHandler::uniquePropertyId()
{
 return d->mUniqueId++;
}

void KGamePropertyHandler::flush()
{
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        if (it.value()->isDirty()) {
                it.value()->sendProperty();
        }
 }
}

/* Fire all property signal changed which are collected in
 * the queque
 **/
void KGamePropertyHandler::lockDirectEmit()
{
  d->mIndirectEmit++;
}

void KGamePropertyHandler::unlockDirectEmit()
{
  // If the flag is <=0 we emit the queued signals
  d->mIndirectEmit--;
  if (d->mIndirectEmit<=0)
  {
    while(!d->mSignalQueue.isEmpty())
    {
      KGamePropertyBase *prop=d->mSignalQueue.dequeue();
//       qDebug() << "emmiting signal for" << prop->id();
      emit signalPropertyChanged(prop);
    }
  }
}

void KGamePropertyHandler::emitSignal(KGamePropertyBase *prop)
{
 // If the indirect flag is set (load and network transmit)
 // we cannot emit the signals directly as it can happend that
 // a sigal causes an access to a property which is e.g. not
 // yet loaded or received

 if (d->mIndirectEmit>0)
 {
  // Queque the signal
  d->mSignalQueue.enqueue(prop);
 }
 else
 {
  // directly emit
  emit signalPropertyChanged(prop);
 }
}

bool KGamePropertyHandler::sendProperty(QDataStream &s)
{
 bool sent = false;
 emit signalSendMessage(id(), s, &sent);
 return sent;
}

KGamePropertyBase* KGamePropertyHandler::find(int id)
{
 if (d->mIdDict.find(id) == d->mIdDict.end())
  return 0;
 return *(d->mIdDict.find(id));
}

void KGamePropertyHandler::clear()
{
  // Note: Hash iterator method 'toFront()' crashes when applied to first item.
  // Therefore we get the keys as list first.
  QList<int> list = d->mIdDict.keys();
  for (int i = 0; i < list.size(); ++i) 
  {
    int key = list.at(i);
    KGamePropertyBase* p = d->mIdDict.value(key);
    p->unregisterData();
    if (d->mIdDict.find(p->id()) != d->mIdDict.end())
    {
      // shouldn't happen - but if mOwner in KGamePropertyBase is NULL
      // this might be possible
      removeProperty(p); 
    }
  }
}

QMultiHash<int, KGamePropertyBase*>& KGamePropertyHandler::dict() const
{ 
 return d->mIdDict; 
}

QString KGamePropertyHandler::propertyValue(KGamePropertyBase* prop)
{
 if (!prop) {
         return i18n("NULL pointer");
 }
           
 int id = prop->id();
 QString name = propertyName(id);
 QString value;

 const type_info* t = prop->typeinfo();
 if (*t == typeid(int)) {
        value = QString::number(((KGamePropertyInt*)prop)->value());
 } else if (*t == typeid(unsigned int)) {
        value = QString::number(((KGamePropertyUInt *)prop)->value());
 } else if (*t == typeid(long int)) {
        value = QString::number(((KGameProperty<qint64> *)prop)->value()); 
 } else if (*t == typeid(unsigned long int)) {
        value = QString::number(((KGameProperty<quint64> *)prop)->value());
 } else if (*t == typeid(QString)) { 
        value = ((KGamePropertyQString*)prop)->value();
 } else if (*t == typeid(qint8)) { 
        value = ((KGamePropertyBool*)prop)->value() ?  i18n("True") : i18n("False");
 } else {
        emit signalRequestValue(prop, value);
 }
                   
 if (value.isNull()) {
        value = i18n("Unknown");
 }
 return value;
}

void KGamePropertyHandler::Debug()
{
 qDebug() << "-----------------------------------------------------------";
 qDebug() << "KGamePropertyHandler:: Debug this=" << this;

 qDebug() << "  Registered properties: (Policy,Lock,Emit,Optimized, Dirty)";
 QHashIterator<int, KGamePropertyBase*> it(d->mIdDict);
 while (it.hasNext()) {
it.next();
        KGamePropertyBase *p=it.value();
        qDebug() << "  "<< p->id() << ": p=" << p->policy() 
                        << "l="<<p->isLocked()
                        << "e="<<p->isEmittingSignal() 
                        << "o=" << p->isOptimized() 
                        << "d="<<p->isDirty();
 }
 qDebug() << "-----------------------------------------------------------";
}

#include "kgamepropertyhandler.moc"
