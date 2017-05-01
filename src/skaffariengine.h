/*
 * Skaffari - a mail account administration web interface based on Cutelyst
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SKAFFARIENGINE_H
#define SKAFFARIENGINE_H

#include <QObject>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(SK_ENGINE)


/*!
 * \brief Initializes the Skaffari configuration.
 *
 * Has to be created and initialized in the Cutelyst::Application::postFork() and added to every
 * Cutelyst::Controller.
 */
class SkaffariEngine : public QObject
{
    Q_OBJECT
public:
	/*!
	 * \brief Constructs a SkaffariEngine object.
	 *
	 * \note You have to use init() to initialize the data engine.
	 */
    explicit SkaffariEngine(QObject *parent = nullptr);

	/*!
	 * \brief Deconstructs the SkaffariEngine object.
	 */
    ~SkaffariEngine();

	/*!
	 * \brief Initializes the data engine.
	 *
     * Call this function in Application::postFork() to set the values from the configuratino file.
	 *
     * \param adminsConfig Takes the config for admin accounts from the \a Admins group in the config file.
     * \param defaults Takes the default values from the \a Defaults group in the config file.
     * \param accountsConfig Takes the config for user accounts from the \a Accounts group in the config file.
     * \param imapConfig Takes the config for the IMAP server from the \a IMAP group in the config file.
     * \return \c true if the engine was initialized successfully, otherwise \c false.
	 */
    bool init(const QVariantMap &adminsConfig, const QVariantMap &defaults, const QVariantMap &accountsConfig, const QVariantMap &imapConfig);

    /*!
     * \brief Returns the value of the admin configuration for \a key.
     * \param key The configuration key.
     * \param defaultValue Value to return if the \a key is not present in the configuration.
     * \return The value for the config \a key.
     */
    QVariant adminConfig(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /*!
     * \brief Returns the IMAP configuration value for \a key.
     * \param key The configuration key.
     * \param defaultValue Value to return if the \a key is not present in the configuration.
     * \return The value for the config \a key.
     */
    QVariant imapConfig(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /*!
     * \brief Returns the complete IMAP configuration.
     * \return All IMAP settings in a map.
     */
    QVariantMap imapConfig() const;

    /*!
     * \brief Returns the default value configuration for \a key.
     * \param key The configuration key.
     * \param defaultValue Value to return if the \a key is not present in the configuration.
     * \return The value for the config \a key.
     */
    QVariant defaultValue(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /*!
     * \brief Return the value of the account configuratino for \a key.
     * \param key The configuration key.
     * \param defaultValue Value to return if the \a key is not present in the configuration.
     * \return The value for the config \a key.
     */
    QVariant accountConfig(const QString &key, const QVariant &defaultValue = QVariant()) const;

private:
    bool initImap();

    QVariantMap m_imapConfig;
    QVariantMap m_adminsConfig;
    QVariantMap m_defaultValues;
    QVariantMap m_accountsConfig;
    QVariantMap m_generalConfig;
    bool m_updateRequired = false;
};

#endif // SKAFFARIENGINE_H
