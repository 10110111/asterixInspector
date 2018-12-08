/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2014 Volker Poplawski (volker@openbios.org)
 */
#ifndef BIT_H
#define BIT_H

// generate mask for lowest/lsb 'n' bits. E.g. BITMASK(6) => 0x3f
#ifndef BITMASK
#define BITMASK(n) ((1<<(n))-1)
#endif

// test if the 'n'th lowest/lsb is set
#ifndef BITTEST
#define BITTEST(v,n) ((bool)((v) & (1 << (n))))
#endif

// set the 'n'th lowest/lsb bit
#ifndef BITSET
#define BITSET(v,n) (v |= (1 << (n)))
#endif


// get u8/u16/u32 at octed offset 'ofs' without alignment&endian constraints
inline quint8   getU8(const uchar* p, int ofs) { return *reinterpret_cast<const quint8*>(p + ofs); }
inline quint16 getU16(const uchar* p, int ofs) { return qFromBigEndian<quint16>(p + ofs); }
inline quint32 getU32(const uchar* p, int ofs) { return qFromBigEndian<quint32>(p + ofs); }


/** Right shift QByteArray by 0..7 bits.
 **/
inline 
void shiftRight(QByteArray& a, int n)
{
  Q_ASSERT(n < 8);

  if (a.isEmpty())
    return;

  for (int i = a.length()-1; i > 0; i--)
  {
    a[i] = (a[i-1] << (8-n)) | ((uchar)a[i] >> n);
  }
  a[0] = (uchar)a[0] >> n;
}


/** Extract arbitrary bitfield (i.e. memory section on bit precision).
 ** Extract from memory at p with size n with inclusive bit indexes msb and lsb counting from 0 from end of memory.
 ** Resulting Bytearray is right-justified and 0-padded on left side.
 **/
inline 
QByteArray bitfield(const uchar* p, int n, int msb, int lsb)
{
  Q_ASSERT(p);
  Q_ASSERT(n > 0);
  Q_ASSERT((msb >= lsb));

  int lByte = n-1 - lsb/8;   // offset of least sig byte
  int mByte = n-1 - msb/8;   // offset of most sig byte

  QByteArray ret((const char*)(p + mByte), lByte-mByte+1);    // copy memory to buffer
  ret[0] = ret[0] & BITMASK(msb%8+1);                         // mask out left side
  shiftRight(ret, lsb%8);                                     // shift right
  ret = ret.right((msb-lsb)/8+1);                             // drop leftover bytes

  return ret;
}


/** Converts arbitrary data/bitfield to quint64.
 **/
inline
quint64 bitfieldToUInt(QByteArray bitfield)
{
  bitfield.prepend( QByteArray(8 - bitfield.size(), 0 )); // extend 'bitfield' to 64bit with prepended zeros
  return qFromBigEndian<quint64>((const uchar*)bitfield.constData());
}


/** Converts arbitrary data/bitfield to qint64, cares for sign.
 **/
inline
qint64 bitfieldToInt(QByteArray bitfield, int msb)
{
  if (BITTEST(bitfield[bitfield.size()-1-msb/8], msb%8))
  {
    // msb is 1: sign extension
    bitfield.prepend( QByteArray(8 - bitfield.size(),  0xff)); // extend 'bitfield' to 64bit with prepended 0xff
    bitfield[7 - (msb/8)] = (0xff & ~BITMASK(msb % 8)) | bitfield[7 - (msb/8)];
    return qFromBigEndian<qint64>((const uchar*)bitfield.constData());
  }
  else
  {
    return bitfieldToUInt(bitfield);
  }
}

#endif // BIT_H
