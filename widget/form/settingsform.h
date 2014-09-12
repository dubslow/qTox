/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QFileDialog>
#include "widget/croppinglabel.h"

namespace Ui {class MainWindow;}
class QString;

class SettingsForm : public QObject
{
    Q_OBJECT
public:
    SettingsForm();
    ~SettingsForm();

    void show(Ui::MainWindow &ui);
    static QList<QString> searchProfiles();

public slots:
    void setFriendAddress(const QString& friendAddress);

private slots:
    void onLoadClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onImportClicked();
    void onTestVideoClicked();
    void onEnableIPv6Updated();
    void onUseTranslationUpdated();
    void onMakeToxPortableUpdated();
    void onSmileyBrowserIndexChanged(int index);
    void copyIdClicked();

private:
    QLabel headLabel, smileyPackLabel;
    QTextEdit id;
    CroppingLabel idLabel;
    QLabel profilesLabel;
    QComboBox profiles;
    QPushButton loadConf, exportConf, delConf, importConf, videoTest;
    QHBoxLayout cbox, buttons;
    QCheckBox enableIPv6, useTranslations, makeToxPortable;
    QVBoxLayout layout, headLayout;
    QWidget *main, *head, *hboxcont1, *hboxcont2;
	QComboBox smileyPackBrowser;
    QString getSelectedSavePath();
};

#endif // SETTINGSFORM_H
