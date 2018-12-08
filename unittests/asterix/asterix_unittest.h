#ifndef ASTERIX_UNITTEST_H
#define ASTERIX_UNITTEST_H

#include <QtCore>
#include <QtTest/QtTest>

#include "asterix.h"

#include "bit.h"


class Bitfield : public QByteArray
{
  public:
    Bitfield(const uchar* data, int len) {
      for (int i = 0; i < len; i++)
      {
        for (int j = 7; j >= 0; j--)
        {
          append(BITTEST(data[i], j) ? 1 : 0);
        }
      }
      Q_ASSERT(length()/8 == len);
    }

    int numBytes() const { return length()/8; }

    QByteArray byteData() const {
      QByteArray ret;
      for (int i = 0; i < numBytes(); i++)
      {
        uchar b = 0;
        for (int j = 0; j < 8; j++)
        {
          if (at(i*8+j) > 0)
            BITSET(b, 7-j);
        }
        ret.append(b);
      }
      return ret;
    }

    Bitfield bitfield(int msb, int lsb) const {
      QByteArray a(mid(length()-msb-1, msb-lsb+1));
      return Bitfield(a);
    }

  private:
    Bitfield(const QByteArray& bitfield) : QByteArray(bitfield) {
      if (bitfield.length() == 0 || bitfield.length()%8 > 0)
        prepend(QByteArray(8-(bitfield.length()%8), 0));
    }
};


class Asterix_UnitTest : public QObject
{
  Q_OBJECT
  public:
    Asterix_UnitTest(QObject* parent = 0);

  private slots:
    void correctnessBitmask();
    void correctnessBittest();
    void correctnessBitfield();

};

#endif
