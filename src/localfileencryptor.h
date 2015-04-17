/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

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

#include <QObject>
#include <tox/toxencryptsave.h>

class LocalFileEncryptor : public QObject
{
    Q_OBJECT

public:
    enum PasswordType {ptMain = 0, ptHistory, ptCounter};

    bool isPasswordSet(PasswordType pt);
    void setPassword(QString& password, PasswordType passtype, const QByteArray& salt = QByteArray());
    void useOtherPassword(PasswordType type);
    void clearPassword(PasswordType passtype);
    QByteArray encryptData(const QByteArray& data, PasswordType passtype);
    QByteArray decryptData(const QByteArray& data, PasswordType passtype);

    static QByteArray getSaltFromFile(QString file);

private:
    void printKeyDerivationError(TOX_ERR_KEY_DERIVATION &error);
    void printEncryptionError(TOX_ERR_ENCRYPTION &error);
    void printDecryptionError(TOX_ERR_DECRYPTION &error);

private:
    TOX_PASS_KEY *pwsaltedkeys[PasswordType::ptCounter];
};
