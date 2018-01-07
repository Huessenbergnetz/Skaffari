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

#ifndef CONSOLEOUTPUT_H
#define CONSOLEOUTPUT_H

#include <QString>
#include <QCoreApplication>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QStringList;

/*!
 * \brief Handles console output and input.
 */
class ConsoleOutput {

    Q_DECLARE_TR_FUNCTIONS(ConsoleOutput)
public:
    /*!
     * \brief Constructs a new ConsoleOutput object.
     * \param quiet Set to \c true to disable console output.
     */
    ConsoleOutput(bool quiet = false);

    /*!
     * \brief Deconstructs the ConsoleOutput object.
     */
    ~ConsoleOutput();

protected:
    /*!
     * \brief Prints \a message to stdout and returns \a code.
     */
    int error(const QString &message, int code = 99) const;
    /*!
     * \brief Prints \a message to stdout and returns \c 1.
     */
    int inputError(const QString &message) const;
    /*!
     * \brief Prints \a message to stdout and returns \c 2.
     */
    int configError(const QString &message) const;
    /*!
     * \brief Prints \a message to stdout and returns \c 3.
     */
    int dbError(const QString &message) const;
    /*!
     * \brief Prints last error text of \a db to stdout and returns \c 3.
     */
    int dbError(const QSqlDatabase &db) const;
    /*!
     * \brief Prints last error text of \a q to stdout and returns \c 3.
     */
    int dbError(const QSqlQuery &q) const;
    /*!
     * \brief Prints error text of \a e to stdout and returns \c 3.
     */
    int dbError(const QSqlError &e) const;
    /*!
     * \brief Prints \a message to stdout and returns \c 4.
     */
    int fileError(const QString &message) const;
    /*!
     * \brief Prints \a message to stdout and returns \c 5.
     */
    int imapError(const QString &message) const;
    /*!
     * \brief Prints \a status message to stdout with a width of 100 and no new line at the end.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printStatus(const QString &status) const;
    /*!
     * \brief Prints \a done to the current line on stdout and adds a new line.
     *
     * If \a done is empty, a translated version of "Done" will be printed.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printDone(const QString &done = QString()) const;
    /*!
     * \brief Prints \a failed to the current line on stdout and adds a new line.
     *
     * If \a failed is empty, a translated version of "Failed" will be printed.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printFailed(const QString &failed = QString()) const;
    /*!
     * \brief Prints \a error to stdout and adds a new line.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printError(const QString &error) const;
    /*!
     * \brief Prints all \a errors to stdout in new lines.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printError(const QStringList &errors) const;
    /*!
     * \brief Prints \a message to stdout and adds a new line.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printMessage(const QString &message) const;
    /*!
     * \brief Prints \a message to stdout and adds a new line.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printSuccess(const QString &message) const;
    /*!
     * \brief Prints \a desc to stdout.
     *
     * If \a desc is longer than 95, it will be split up into multiple lines. Every line will
     * be prepended by '# '.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printDesc(const QString &desc) const;
    /*!
     * \brief Prints \a descList to stdout.
     *
     * If the strings in \a descList are longer than 95, they will be split up into multiple lines.
     * Every line will be prepended by '# '.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printDesc(const QStringList &descList) const;
    /*!
     * \brief Prints the content of \a table in an ASCII table to stdout, together with the \a header.
     *
     * The first part in the \a table pair will be the label, the second will be the value.
     *
     * If ConsoleOutput has been constructed with \a quiet = \c false, this will do nothing.
     */
    void printTable(std::initializer_list<std::pair<QString, QString>> table, const QString &header = QString()) const;

    /*!
     * \brief Reads a string from stdin.
     * \param name              the name of the value to read
     * \param defaultVal        the default value that will be used if the user only hits enter
     * \param desc              description for the value to read
     * \param acceptableInput   optional list of acceptable input values, if input is not acceptable, input request will be retried
     * \return                  the entered input value
     */
    QString readString(const QString &name, const QString &defaultVal, const QStringList &desc = QStringList(), const QStringList acceptableInput = QStringList()) const;
    /*!
     * \brief Reads a network port (0-65535) from stdin.
     * \param name              the name of the value to read
     * \param defaultVal        the default value that will be used if the user only hits enter
     * \param desc              description for the value to read
     * \return                  the entered input value
     */
    quint16 readPort(const QString &name, quint16 defaultVal, const QStringList &desc = QStringList()) const;
    /*!
     * \brief Reads a quint8 from stdin.
     * \param name              the name of the value to read
     * \param defaultVal        the default value that will be used if the user only hits enter
     * \param desc              description for the value to read
     * \param acceptableInput   optional list of acceptable input values, if input is not acceptable, input request will be retried
     * \return                  the entered input value
     */
    quint8 readChar(const QString &name, quint8 defaultVal, const QStringList &desc = QStringList(), const QList<quint8> acceptableInput = QList<quint8>()) const;
    /*!
     * \brief Reads a quint32 from stdin.
     * \param name              the name of the value to read
     * \param defaultVal        the default value that will be used if the user only hits enter
     * \param desc              description for the value to read
     * \return                  the entered input value
     */
    quint32 readInt(const QString &name, quint32 defaultVal, const QStringList &desc = QStringList()) const;
    /*!
     * \brief Reads a boolean value from stdin.
     *
     * Boolean values are quried as translated versions of "Yes/Y" and "No/N".
     *
     * \param name              the name of the value to read
     * \param defaultVal        the default value that will be used if the user only hits enter
     * \param desc              description for the vale to read
     * \return                  the entered input value
     */
    bool readBool(const QString &name, bool defaultVal, const QStringList &desc = QStringList()) const;

private:
    bool m_quiet = false;
};

#endif // CONSOLEOUTPUT_H
