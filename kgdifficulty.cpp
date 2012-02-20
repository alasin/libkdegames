/***************************************************************************
 *   Copyright 2007 Nicolas Roffet <nicolas-kde@roffet.com>                *
 *   Copyright 2007 Pino Toscano <toscano.pino@tiscali.it>                 *
 *   Copyright 2011-2012 Stefan Majewsky <majewsky@gmx.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kgdifficulty.h"

#include <QtCore/QVector>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KGuiItem>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

//BEGIN KgDifficultyLevel

struct KgDifficultyLevel::Private
{
	int m_hardness;
	StandardLevel m_level;
	QByteArray m_key;
	QString m_title;

	Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level);
	static Private* fromStandardLevel(StandardLevel level);
};

KgDifficultyLevel::KgDifficultyLevel(int hardness, const QByteArray& key, const QString& title)
	: d(new Private(hardness, key, title, Custom))
{
}

KgDifficultyLevel::Private::Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level)
	: m_hardness(hardness)
	, m_level(level)
	, m_key(key)
	, m_title(title)
{
}

KgDifficultyLevel::KgDifficultyLevel(StandardLevel level)
	: d(Private::fromStandardLevel(level))
{
}

KgDifficultyLevel::Private* KgDifficultyLevel::Private::fromStandardLevel(KgDifficultyLevel::StandardLevel level)
{
	Q_ASSERT_X(level != Custom,
		"KgDifficultyLevel(StandardLevel) constructor",
		"Custom level not allowed here"
	);
	//The first entry in the pair is to be used as a key so don't change it. It doesn't have to match the string to be translated
	QPair<QByteArray, QString> data;
	switch (level)
	{
		case RidiculouslyEasy:
			data = qMakePair(QByteArray("Ridiculously Easy"), i18nc("Game difficulty level 1 out of 8", "Ridiculously Easy"));
			break;
		case VeryEasy:
			data = qMakePair(QByteArray("Very Easy"), i18nc("Game difficulty level 2 out of 8", "Very Easy"));
			break;
		case Easy:
			data = qMakePair(QByteArray("Easy"), i18nc("Game difficulty level 3 out of 8", "Easy"));
			break;
		case Medium:
			data = qMakePair(QByteArray("Medium"), i18nc("Game difficulty level 4 out of 8", "Medium"));
			break;
		case Hard:
			data = qMakePair(QByteArray("Hard"), i18nc("Game difficulty level 5 out of 8", "Hard"));
			break;
		case VeryHard:
			data = qMakePair(QByteArray("Very Hard"), i18nc("Game difficulty level 6 out of 8", "Very Hard"));
			break;
		case ExtremelyHard:
			data = qMakePair(QByteArray("Extremely Hard"), i18nc("Game difficulty level 7 out of 8", "Extremely Hard"));
			break;
		case Impossible:
			data = qMakePair(QByteArray("Impossible"), i18nc("Game difficulty level 8 out of 8", "Impossible"));
			break;
		case Custom:
			return 0;
	}
	return new KgDifficultyLevel::Private(level, data.first, data.second, level);
}

KgDifficultyLevel::~KgDifficultyLevel()
{
	delete d;
}

int KgDifficultyLevel::hardness() const
{
	return d->m_hardness;
}

QByteArray KgDifficultyLevel::key() const
{
	return d->m_key;
}

QString KgDifficultyLevel::title() const
{
	return d->m_title;
}

KgDifficultyLevel::StandardLevel KgDifficultyLevel::standardLevel() const
{
	return d->m_level;
}

//END KgDifficultyLevel
//BEGIN KgDifficulty

struct KgDifficulty::Private
{
	QList<const KgDifficultyLevel*> m_levels;
	const KgDifficultyLevel* m_currentLevel;
	bool m_editable, m_gameRunning;

	Private() : m_currentLevel(0), m_editable(true), m_gameRunning(false) {}
};

KgDifficulty::KgDifficulty(QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	qRegisterMetaType<const KgDifficultyLevel*>();
}

KgDifficulty::~KgDifficulty()
{
	//save current difficulty level in config file (no sync() call here; this
	//will most likely be called at application shutdown when others are also
	//writing to KGlobal::config(); also KConfig's dtor will sync automatically)
	KConfigGroup cg(KGlobal::config(), "KgDifficulty");
	cg.writeEntry("Level", currentLevel()->key());
	//cleanup
	while (!d->m_levels.isEmpty())
	{
		delete const_cast<KgDifficultyLevel*>(d->m_levels.takeFirst());
	}
}

void KgDifficulty::addLevel(KgDifficultyLevel* level)
{
	//The intended use is to create the KgDifficulty object, add levels, *then*
	//start to work with the currentLevel(). The first call to currentLevel()
	//will load the previous selection from the config, and the level list will
	//be considered immutable from this point.
	Q_ASSERT_X(d->m_currentLevel == 0,
		"KgDifficulty::addLevel",
		"Only allowed before currentLevel() is called."
	);
	//ensure that list stays sorted
	QList<const KgDifficultyLevel*>::iterator it = d->m_levels.begin();
	while (it != d->m_levels.end() && (*it)->hardness() < level->hardness())
	{
		++it;
	}
	d->m_levels.insert(it, level);
}

typedef KgDifficultyLevel::StandardLevel DS;
void KgDifficulty::addStandardLevelRange(DS from, DS to)
{
	const QVector<DS> levels = QVector<DS>()
		<< KgDifficultyLevel::RidiculouslyEasy
		<< KgDifficultyLevel::VeryEasy
		<< KgDifficultyLevel::Easy
		<< KgDifficultyLevel::Medium
		<< KgDifficultyLevel::Hard
		<< KgDifficultyLevel::VeryHard
		<< KgDifficultyLevel::ExtremelyHard
		<< KgDifficultyLevel::Impossible
	;
	const int fromIndex = levels.indexOf(from);
	const int toIndex = levels.indexOf(to);
	Q_ASSERT_X(fromIndex > 0 && toIndex > 0,
		"KgDifficulty::addStandardLevelRange",
		"No argument may be KgDifficultyLevel::Custom."
	);
	for (int i = fromIndex; i <= toIndex; ++i)
	{
		addLevel(new KgDifficultyLevel(levels[i]));
	}
}

QList<const KgDifficultyLevel*> KgDifficulty::levels() const
{
	return d->m_levels;
}

const KgDifficultyLevel* KgDifficulty::currentLevel() const
{
	if (d->m_currentLevel)
	{
		return d->m_currentLevel;
	}
	Q_ASSERT(!d->m_levels.isEmpty());
	//check configuration file for saved difficulty level
	KConfigGroup cg(KGlobal::config(), "KgDifficulty");
	const QByteArray key = cg.readEntry("Level", QByteArray());
	foreach (const KgDifficultyLevel* level, d->m_levels)
	{
		if (level->key() == key)
		{
			return d->m_currentLevel = level;
		}
	}
	//no level predefined - easiest level is probably a sane default
	return d->m_currentLevel = d->m_levels[0];
}

bool KgDifficulty::isEditable() const
{
	return d->m_editable;
}

void KgDifficulty::setEditable(bool editable)
{
	if (d->m_editable == editable)
	{
		return;
	}
	d->m_editable = editable;
	emit editableChanged(editable);
}

bool KgDifficulty::isGameRunning() const
{
	return d->m_gameRunning;
}

void KgDifficulty::setGameRunning(bool gameRunning)
{
	if (d->m_gameRunning == gameRunning)
	{
		return;
	}
	d->m_gameRunning = gameRunning;
	emit gameRunningChanged(gameRunning);
}

void KgDifficulty::select(const KgDifficultyLevel* level)
{
	Q_ASSERT(d->m_levels.contains(level));
	if (d->m_currentLevel == level)
	{
		return;
	}
	//ask for confirmation if necessary
	if (d->m_gameRunning)
	{
		const int result = KMessageBox::warningContinueCancel(0,
			i18n("Changing the difficulty level will end the current game!"),
			QString(), KGuiItem(i18n("Change the difficulty level"))
		);
		if (result != KMessageBox::Continue)
		{
			emit selected(d->m_currentLevel);
			return;
		}
	}
	d->m_currentLevel = level;
	emit selected(level);
	emit changed(level);
}

//END KgDifficulty

#include "kgdifficulty.moc"
