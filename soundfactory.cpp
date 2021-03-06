/***************************************************************************
 *   Copyright (C) 1999-2006 by Éric Bischoff <ebischoff@nerim.net>        *
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/* Sound factory */

#include "soundfactory.h"

#include <stdlib.h>

#include <kmessagebox.h>
#include <KLocalizedString>

#include <phonon/MediaObject>

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QStandardPaths>

#include "toplevel.h"

// Constructor
SoundFactory::SoundFactory(TopLevel *parent)
{
  topLevel = parent;
  player = Phonon::createPlayer(Phonon::GameCategory);
  player->setParent(parent);
}

// Destructor
SoundFactory::~SoundFactory()
{
}

// Play some sound
void SoundFactory::playSound(const QString &soundRef) const
{
  int sound;
  QString soundFile;

  if (!topLevel->isSoundEnabled()) return;

  for (sound = 0; sound < sounds; sound++)
	  if (!namesList[sound].compare(soundRef)) break;
  if (sound == sounds) return;

  soundFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String( "sounds/" ) + filesList[sound]);
  if (soundFile.isEmpty()) return;

//printf("%s\n", (const char *) soundFile);
  player->setCurrentSource(QUrl::fromLocalFile(soundFile));
  player->play();
}

// Register the various languages
void SoundFactory::registerLanguages()
{
  QSet<QString> list;
  const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("sounds"), QStandardPaths::LocateDirectory);
  Q_FOREACH (const QString &dir, dirs)
  {
    const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.soundtheme"));
    Q_FOREACH (const QString &file, fileNames)
    {
        list <<dir + '/' + file;
    }
  }

  foreach(const QString &soundTheme, list)
  {
    QFile file(soundTheme);
    if (file.open(QIODevice::ReadOnly))
    {
      QDomDocument document;
      if (document.setContent(&file))
      {
        QString code = document.documentElement().attribute(QStringLiteral( "code" ));
        bool enabled = !(QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String( "sounds/" ) + code + QLatin1Char( '/' ), QStandardPaths::LocateDirectory).isEmpty());
        topLevel->registerLanguage(code, soundTheme, enabled);
      }
    }
  }
}

// Load the sounds of one given language
bool SoundFactory::loadLanguage(const QString &selectedLanguageFile)
{
  QDomNodeList languagesList,
               soundNamesList;
  QDomElement languageElement,
              soundNameElement;
  QDomAttr nameAttribute, fileAttribute;

  QFile file(selectedLanguageFile);
  if (!file.open(QIODevice::ReadOnly)) return false;

  QDomDocument document;
  if (!document.setContent(&file)) return false;

  languageElement = document.documentElement();

  soundNamesList = languageElement.elementsByTagName(QStringLiteral( "sound" ));
  sounds = soundNamesList.count();
  if (sounds < 1)
    return false;


  namesList.clear();
  filesList.clear();
  for (int sound = 0; sound < sounds; sound++)
  {
    soundNameElement = (const QDomElement &) soundNamesList.item(sound).toElement();

    nameAttribute = soundNameElement.attributeNode(QStringLiteral( "name" ));
    namesList << nameAttribute.value();
    fileAttribute = soundNameElement.attributeNode(QStringLiteral( "file" ));
    filesList << fileAttribute.value();
  }

  currentSndFile = selectedLanguageFile;

  return true;
}

QString SoundFactory::currentSoundFile() const
{
  return currentSndFile;
}
