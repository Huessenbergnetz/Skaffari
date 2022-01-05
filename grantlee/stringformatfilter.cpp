#include "stringformatfilter.h"
#include <grantlee5/grantlee/util.h>
#include <grantlee/exception.h>
#include <limits>

QVariant StringformatFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    QVariant ret;

    Q_UNUSED(autoescape);

    if (input.isNull() || !input.isValid()) {
        qWarning("%s", "sk_stringformat: invalid input value");
        return ret;
    }

    ret = input;

    QString format = Grantlee::getSafeString(argument).get();

    if (format.size() > 1 && format.startsWith(QLatin1Char('%'))) {
        format.remove(0, 1);
    }

    if (format.isEmpty()) {
        qWarning("%s", "sk_stringformat: empty format string");
        return ret;
    }

    static const QString flags{QStringLiteral("-+ #0")};
    static const QString convSpecs{QStringLiteral("csdioxXufFeEaAgG")};
    static const QString lengthMods{QStringLiteral("hljztL")};

    bool leftJustified = false;
    bool forceSign = false;
    bool prependSpace = false;
    bool altForm = false;
    bool leadingZeroPad = false;

    QChar conversionSpecifier;
    LengthModifier lengthModifier = None;

    QString fieldWidthStr;
    QString precisionStr;

    FormatPosition fPos = Start;
    const QString constFormat = format;

    for (const QChar &c : constFormat) {

        switch(c.unicode()) {
        case 32: // space sign
        {
            if (fPos <= Flags) {
                prependSpace = true;
                fPos = Flags;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 35: // hash tag
        {
            if (fPos <= Flags) {
                altForm = true;
                fPos = Flags;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 43: // plus sign
        {
            if (fPos <= Flags) {
                forceSign = true;
                fPos = Flags;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 45: // minus sign
        {
            if (fPos <= Flags) {
                leftJustified = true;
                fPos = Flags;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 46: // dot (.)
        {
            if (fPos < Precision) {
                fPos = Precision;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 48: // 0
        {
            if (fPos <= Flags) {
                leadingZeroPad = true;
                fPos = Flags;
            } else if (fPos == MinFieldWidth) {
                fieldWidthStr.append(c);
                fPos = MinFieldWidth;
            } else if (fPos == Precision) {
                precisionStr.append(c);
                fPos = Precision;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        {
            if (fPos <= MinFieldWidth) {
                fieldWidthStr.append(c);
                fPos = MinFieldWidth;
            } else if (fPos == Precision) {
                precisionStr.append(c);
                fPos = Precision;
            }
        }
            break;
        default:
        {
            if (fPos <= LengthMod && lengthMods.contains(c)) {
                lengthModifier = getLengthModifier(c, lengthModifier);
                fPos = LengthMod;
            } else if (fPos <= ConvSpec && conversionSpecifier.isNull() && convSpecs.contains(c)) {
                conversionSpecifier = c;
                fPos = ConvSpec;
            } else {
                qWarning("sk_stringformat: invalid format modifier (at this position): %s", qUtf8Printable(QString(c)));
            }
        }
            break;
        }
    }

    bool ok = true;

    const int fieldWidth = fieldWidthStr.isEmpty() ? 0 : fieldWidthStr.toInt(&ok);
    if (!ok) {
        qWarning("sk_stringformat: invalid field width: %s", qUtf8Printable(fieldWidthStr));
    }

    const int precision = precisionStr.isEmpty() ? 0 : precisionStr.toInt(&ok);
    if (!ok) {
        qWarning("sk_stringformat: invalid precision: %s", qUtf8Printable(fieldWidthStr));
    }

    QString str;
    QTextStream out(&str, QIODevice::WriteOnly);

    if (leftJustified) {
        out.setFieldAlignment(QTextStream::AlignLeft);
    } else {
        out.setFieldAlignment(QTextStream::AlignRight);
    }

    out.setFieldWidth(fieldWidth);

    QTextStream::NumberFlags numberFlags;
    numberFlags.setFlag(QTextStream::ForceSign, forceSign);

    if (conversionSpecifier == QLatin1Char('c')) {

        out << input.toChar();

    } else if (conversionSpecifier == QLatin1Char('s')) {

        if (precision > 0) {
            out << input.toString().left(precision);
        } else {
            out << input.toString();
        }

    } else if (conversionSpecifier == QLatin1Char('d') || conversionSpecifier == QLatin1Char('i')) {

        if (leadingZeroPad) {
            out.setPadChar(QLatin1Char('0'));
        }

    }

//    const QChar conversionSpecifier = format.at(format.size() - 1);
//    format.chop(1);

//    QString str;
//    QTextStream out(&str, QIODevice::WriteOnly);

//    if (conversionSpecifier == QLatin1Char('s')) {

//        int precision = 0;

//        if (!format.isEmpty()) {
//            if (format.startsWith(QLatin1Char('-'))) {
//                out.setFieldAlignment(QTextStream::AlignLeft);
//                format.remove(0, 1);
//            } else {
//                out.setFieldAlignment(QTextStream::AlignRight);
//            }
//            if (!format.isEmpty()) {
//                QString fieldWidthStr;
//                QString precisionStr;
//                bool atPrecisionPart = false;
//                for (const QChar &c : format) {
//                    if (c.isDigit() && !atPrecisionPart) {
//                        fieldWidthStr.append(c);
//                    } else if (c.isDigit() && atPrecisionPart) {
//                        precisionStr.append(c);
//                    } else if (!c.isDigit() && c == QLatin1Char('.')) {
//                        atPrecisionPart = true;
//                    } else {
//                        qWarning("sk_stringformat: invalid string format specifier: %s", qUtf8Printable(QString(c)));
//                    }
//                }

//                bool ok = true;

//                const int fieldWidth = fieldWidthStr.isEmpty() ? 0 : fieldWidthStr.toInt(&ok);
//                if (!ok) {
//                    qWarning("%s", "sk_stringformat: invalid string field with");
//                }
//                out.setFieldWidth(fieldWidth);

//                precision = precisionStr.isEmpty() ? 0 : precisionStr.toInt(&ok);
//                if (!ok) {
//                    qWarning("%s", "sk_stringformat: invalid string precision");
//                }
//            }
//        }

//        if (precision > 0) {
//            out << input.toString().left(precision);
//        } else {
//            out << input.toString();
//        }

//    } else if (conversionSpecifier == QLatin1Char('c')) {

//        out << input.toChar();

//    } else if (conversionSpecifier == QLatin1Char('i') || conversionSpecifier == QLatin1Char('d') || conversionSpecifier == QLatin1Char('u')) {

//        out.setIntegerBase(10);

//        LengthModifier lm = None;
//        QString fieldWidthString;
//        bool useZeroAsPad = false;

//        for (const QChar &c : format) {
//            if (c == QLatin1Char('-')) {
//                out.setFieldAlignment(QTextStream::AlignLeft);
//            } else if (c == QLatin1Char('+')) {
//                out.setNumberFlags(QTextStream::ForceSign);
//            } else if (c.isDigit()) {
//                if (c == QLatin1Char('0')) {
//                    if (fieldWidthString.isEmpty()) {
//                        useZeroAsPad = true;
//                    } else {
//                        fieldWidthString.append(c);
//                    }
//                } else {
//                    fieldWidthString.append(c);
//                }
//            } else {
//                lm = getLengthModifier(c, lm);
//            }
//        }

//        bool ok = true;

//        const int fieldWidth = fieldWidthString.isEmpty() ? 0 : fieldWidthString.toInt(&ok);
//        if (ok) {
//            out.setFieldWidth(fieldWidth);
//        } else {
//            qWarning("%s", "sk_stringformat: invalid field width");
//        }

//        if (useZeroAsPad) {
//            out.setPadChar(QLatin1Char('0'));
//        }

//        if (conversionSpecifier != QLatin1Char('u')) {
//            if (lm == h || lm == hh || lm == None) {
//                const int i = input.toInt(&ok);
//                if (!ok) {
//                    qWarning("%s", "sk_stringformat: failed to convert input value to int");
//                    return ret;
//                }
//                if (lm == None) {
//                    out << i;
//                } else if (lm == h) {
//                    if (i < std::numeric_limits<short>::min() || i > std::numeric_limits<short>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to short");
//                        return ret;
//                    }
//                    out << static_cast<short>(i);
//                } else if (lm == hh) {
//                    if (i < std::numeric_limits<char>::min() || i > std::numeric_limits<char>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to char");
//                        return ret;
//                    }
//                    out << static_cast<char>(i);
//                }
//            } else {
//                const qlonglong qll = input.toLongLong(&ok);
//                if (!ok) {
//                    qWarning("%s", "sk_stringformat: failed to convert input value to long long");
//                    return ret;
//                }
//                if (lm == ll) {
//                    out << qll;
//                } else if (lm == l) {
//                    if (qll < std::numeric_limits<long>::min() || qll > std::numeric_limits<long>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to long");
//                        return ret;
//                    }
//                    out << static_cast<long>(qll);
//                } else if (lm == j) {
//                    if (qll < std::numeric_limits<intmax_t>::min() || qll > std::numeric_limits<intmax_t>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to intmax_t");
//                        return ret;
//                    }
//                    out << static_cast<intmax_t>(qll);
//                } else if (lm == z) {
//                    if (qll < std::numeric_limits<ssize_t>::min() || qll > std::numeric_limits<ssize_t>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to invalid signed size_t");
//                        return ret;
//                    }
//                    out << static_cast<ssize_t>(qll);
//                } else if (lm == t) {
//                    if (qll < std::numeric_limits<ptrdiff_t>::min() || qll > std::numeric_limits<ptrdiff_t>::max()) {
//                        qWarning("%s", "sk_stringformat: failed to convert input value to intmax_t");
//                        return ret;
//                    }
//                    out << static_cast<ptrdiff_t>(qll);
//                }
//            }
//        } else {
//            if (lm == hh) {
//                const uchar v = convertUNumber<uchar>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == h) {
//                const ushort v = convertUNumber<ushort>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == None) {
//                const uint v = convertUNumber<uint>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == l) {
//                const ulong v = convertUNumber<ulong>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == ll) {
//                const qulonglong v = convertUNumber<qulonglong>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == j) {
//                const uintmax_t v = convertUNumber<uintmax_t>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else if (lm == z) {
//                const size_t v = convertUNumber<size_t>(input, &ok);
//                if (!ok) {
//                    return ret;
//                }
//                out << v;
//            } else {
//                qWarning("%s", "sk_stringformat: invalid length modifier");
//                return ret;
//            }
//        }
//    }

    ret = QVariant::fromValue<QString>(str);

    return ret;
}

StringformatFilter::LengthModifier StringformatFilter::getLengthModifier(const QChar &c, LengthModifier current) const
{
    if (c == QLatin1Char('h') && current != h) {
        return h;
    } else if (c == QLatin1Char('h') && current == h) {
        return hh;
    } else if (c == QLatin1Char('l') && current != l) {
        return l;
    } else if (c == QLatin1Char('l') && current == l) {
        return ll;
    } else if (c == QLatin1Char('j')) {
        return j;
    } else if (c == QLatin1Char('z')) {
        return z;
    } else if (c == QLatin1Char('t')) {
        return t;
    } else if (c == QLatin1Char('L')) {
        return L;
    } else {
        return current;
    }
}

template< typename T >
T StringformatFilter::convertUNumber(const QVariant &input, bool *ok) const
{
    T ret = 0;

    const qulonglong u = input.toULongLong(ok);

    if (!*ok) {
        qWarning("%s", "sk_stringformat: failed to convert input value to unsigned integer value");
        return ret;
    }

    if (u < std::numeric_limits<T>::min() || u > std::numeric_limits<T>::max()) {
        qWarning("%s", "sk_stringformat: failed to convert input value to unsigned integer value");
        return ret;
    }

    ret = static_cast<T>(u);

    return ret;
}
