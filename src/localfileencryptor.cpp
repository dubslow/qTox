#include "localfileencryptor.h"
#include "misc/cstring.h"
#include <QDebug>
#include <QFile>
#include <QString>

bool LocalFileEncryptor::isPasswordSet(PasswordType passtype)
{
    if (pwsaltedkeys[passtype])
        return true;

    return false;
}

void LocalFileEncryptor::setPassword(QString& password, PasswordType passtype, const QByteArray& salt)
{
    clearPassword(passtype);
    if (password.isEmpty())
        return;

    CString str(password);
    pwsaltedkeys[passtype] = new TOX_PASS_KEY;
    TOX_ERR_KEY_DERIVATION error;

    if (salt.size())
    {
        if (!tox_derive_key_with_salt(str.data(), str.size(), reinterpret_cast<uint8_t *>(salt.data()), pwsaltedkeys[passtype], &error))
            printKeyDerivationError(error);
    }
    else
    {
        if (!tox_derive_key_from_pass(str.data(), str.size(), pwsaltedkeys[passtype], &error))
            printKeyDerivationError(error);
    }
    password.clear();
}

void LocalFileEncryptor::useOtherPassword(PasswordType type)
{
    clearPassword(type);
    pwsaltedkeys[type] = new TOX_PASS_KEY;

    PasswordType other = (type == ptMain) ? ptHistory : ptMain;
    //std::copy(pwsaltedkeys[other], pwsaltedkeys[other]+TOX_PASS_KEY_LENGTH(), pwsaltedkeys[type]);
    *pwsaltedkeys[type] = *pwsaltedkeys[other];
}

void LocalFileEncryptor::clearPassword(PasswordType passtype)
{
    delete[] pwsaltedkeys[passtype];
    pwsaltedkeys[passtype] = nullptr;
}

QByteArray LocalFileEncryptor::encryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();

    uint8_t encrypted[data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH];
    TOX_ERR_ENCRYPTION error;

    if (!tox_pass_key_encrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(), pwsaltedkeys[passtype], encrypted, &error))
    {
        printEncryptionError(error);
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(encrypted), data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
}

QByteArray LocalFileEncryptor::decryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();

    uint8_t decrypted[data.size() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH];
    TOX_ERR_DECRYPTION error;

    if(!tox_pass_key_decrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(), pwsaltedkeys[passtype], decrypted, &error))
    {
        printDecryptionError(error);
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(decrypted), data.size() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
}

QByteArray LocalFileEncryptor::getSaltFromFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "LocalFileEncryptor: file" << filename << "doesn't exist";
        return QByteArray();
    }
    QByteArray data = file.read(TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
    file.close();

    uint8_t *salt = new uint8_t[TOX_PASS_SALT_LENGTH];
    int err = tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
    if (err)
    {
        qWarning() << "LocalFileEncryptor: can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res = QByteArray::fromRawData(reinterpret_cast<const char*>(salt), TOX_PASS_SALT_LENGTH);
    delete[] salt;
    return res;
}

//TODO: add appropriate warnings in below functions
void LocalFileEncryptor::printKeyDerivationError(TOX_ERR_KEY_DERIVATION &error)
{
    switch(error)
    {
        case TOX_ERR_KEY_DERIVATION_NULL :
            qWarning()<<"";
        case TOX_ERR_KEY_DERIVATION_FAILED:
            qWarning()<<"";
    }
}

void LocalFileEncryptor::printEncryptionError(TOX_ERR_ENCRYPTION &error)
{
    switch(error)
    {
        case TOX_ERR_ENCRYPTION_NULL :
            qWarning()<<"";
        case TOX_ERR_ENCRYPTION_KEY_DERIVATION_FAILED:
            qWarning()<<"";
        case TOX_ERR_ENCRYPTION_FAILED:
            qWarning()<<"";
    }
}

void LocalFileEncryptor::printDecryptionError(TOX_ERR_DECRYPTION &error)
{
    switch(error)
    {
        case TOX_ERR_DECRYPTION_NULL :
            qWarning()<<"";
        case TOX_ERR_DECRYPTION_INVALID_LENGTH:
            qWarning()<<"";
        case TOX_ERR_DECRYPTION_BAD_FORMAT:
            qWarning()<<"";
        case TOX_ERR_DECRYPTION_KEY_DERIVATION_FAILED:
            qWarning()<<"";
        case TOX_ERR_DECRYPTION_FAILED:
            qWarning()<<"";
    }
}
