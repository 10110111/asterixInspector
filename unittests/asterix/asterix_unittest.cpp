
#include "asterix_unittest.h"



Asterix_UnitTest::Asterix_UnitTest(QObject *parent) :
  QObject(parent)
{
}


void Asterix_UnitTest::correctnessBitmask()
{
  QVERIFY( BITMASK(0) == 0x00 );
  QVERIFY( BITMASK(1) == 0x01 );
  QVERIFY( BITMASK(2) == 0x03 );
  QVERIFY( BITMASK(3) == 0x07 );
  QVERIFY( BITMASK(6) == 0x3f );
  QVERIFY( BITMASK(8) == 0xff );
  QVERIFY( BITMASK(9) == 0x01ff );
  QVERIFY( BITMASK(16) == 0xffff );
}


void Asterix_UnitTest::correctnessBittest()
{
  for (int i = 0; i < 15; i++)
  {
    QVERIFY( BITTEST(0, i) == false );
  }
 
  for (int i = 0; i < 15; i++)
  {
    QVERIFY( BITTEST(0xffff, i) == true );
  }

  QVERIFY( BITTEST(0xcaffee, 4) == false );
  QVERIFY( BITTEST(0xcaffee, 5) == true );
  
}


void Asterix_UnitTest::correctnessBitfield()
{
  const uchar data[] = { 0x12, 0x34, 0x57, 0x79, 0xf2 };

  Bitfield bf(data, sizeof(data));

  for (int msb = sizeof(data)*8-1; msb >= 0; msb--)
  {
    for (int lsb = msb; lsb >= 0; lsb--)
    {
      QByteArray actual = bitfield(data, sizeof(data), msb, lsb);
      QByteArray expected = bf.bitfield(msb, lsb).byteData();
      QVERIFY2(actual == expected, qPrintable(QString("msb: %1  lsb:%2").arg(msb).arg(lsb)));
    }
  }


  QByteArray data2;
  for (int i = 0; i < 256; i++)
    data2.append((uchar)i);
    
  for (int i = 255; i >= 0; i--)
    data2.append((uchar)i);

  Bitfield bf2((uchar*)data2.data(), data2.length());

  for (int msb = data2.length()*8-1; msb >= 0; msb--)
  {
    for (int lsb = msb; lsb >= 0; lsb--)
    {
      QByteArray actual = bitfield((uchar*)data2.data(), data2.length(), msb, lsb);
      QByteArray expected = bf2.bitfield(msb, lsb).byteData();
      QVERIFY2(actual == expected, qPrintable(QString("msb: %1  lsb:%2").arg(msb).arg(lsb)));
    }
  }
}


QTEST_MAIN(Asterix_UnitTest)

