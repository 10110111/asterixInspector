/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "hexedit.h"

#include "global.h"

const int H_MARGIN = 4;


template <typename T>
constexpr inline bool within(const T& value, const T& left, const T& right) { return left <= value && value < right; }


HexEdit::HexEdit(QWidget *parent)
 : QAbstractScrollArea(parent),
   m_data(0),
   m_offsetColumnMode( SIZE32 )
{
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  setFocusPolicy(Qt::StrongFocus);  // accept mouse and tab focus
  QFont myfont = QFontDatabase::systemFont(QFontDatabase::FixedFont); // ( "monospace" )
  myfont.setStyleHint(QFont::TypeWriter);
  myfont.setKerning(false);
  myfont.setFixedPitch(true);
  setFont( myfont );

  setMouseTracking(true);

  switch (m_offsetColumnMode)
  {
    case SIZE32:
      m_sizeHint = fontMetrics().boundingRect( " 01234567: 00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00    0123456789abcdef " ).size();
      break;

    case SIZE64:
      m_sizeHint = fontMetrics().boundingRect( " 0123456789abcdef: 00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00    0123456789abcdef " ).size();
      break;

    default:
      m_sizeHint = fontMetrics().boundingRect( " 00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00    0123456789abcdef " ).size();
      break;
  }

  m_sizeHint.rwidth() += (2 * H_MARGIN) + (2*frameWidth()) + verticalScrollBar()->sizeHint().width();
  m_sizeHint.rheight() = (fontMetrics().lineSpacing() * 16) + (2*frameWidth());  // extend to 16 lines
  setMinimumHeight( fontMetrics().lineSpacing() * 4 + (2*frameWidth()) );
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding );

  m_cursorPosition = 0;
}


HexEdit::~HexEdit()
{
}


void HexEdit::setData(const uchar * data, quint64 size)
{
  m_data = data;
  m_dataSize = size;
  m_lineCount = m_dataSize / 16;
  if (m_dataSize % 16 != 0)
    m_lineCount++;

  verticalScrollBar()->setValue( 0 );
  updateScrollbar();

  setHighlight( 0, 0 );

  m_cursorPosition = 0;
  emit cursorPosition( m_cursorPosition );
}


void HexEdit::paintEvent(QPaintEvent * event)
{
  if (m_data == 0)
  {
    return;
  }

  QPainter painter( viewport() );

  const int cw = fontMetrics().width( '0' );
  const int ch = fontMetrics().height() - fontMetrics().descent() + 1;
  int hexX = H_MARGIN;
  if (m_offsetColumnMode == SIZE32)
    hexX = H_MARGIN + (8 + 2) * cw;
  else if (m_offsetColumnMode == SIZE64)
    hexX = H_MARGIN + (16 + 2) * cw;
  int asciiX = hexX + H_MARGIN + 16*3*cw + 5*cw;

  /*
   * draw fileoffset background rect
   */
  painter.save();
  painter.setBrush( QBrush( QColor( 230, 230, 230 ) ) );
  painter.setPen( Qt::NoPen );
  painter.drawRect( 0, event->rect().top(), hexX-cw, event->rect().bottom() );
  painter.restore();

  const int linesVisible = viewport()->height() / fontMetrics().lineSpacing();
  int firstToBeUpdatedLine = qMax(event->rect().top() / fontMetrics().lineSpacing() - 1, 0);
  int lastToBeUpdatedLine = qMax(event->rect().bottom() / fontMetrics().lineSpacing() + 1, linesVisible);

  const uchar* firstLineAddress = m_data + (verticalScrollBar()->value() * 16);  // address  of first visible line

    // handle line by line
  for ( int i = firstToBeUpdatedLine; i < lastToBeUpdatedLine; i++ )
  {
    const uchar* lineAddress = firstLineAddress + i*16;   // address of first byte in line

    int bytesInLine = 16;    // a complete line has 16 bytes, the very last line has [1..16]
    if (verticalScrollBar()->value() + i + 1 == m_lineCount)
    {
      // this is the last line
      bytesInLine = m_dataSize % 16;
      if (bytesInLine == 0)
        bytesInLine = 16;
    }
    else if (verticalScrollBar()->value() + i + 1 > m_lineCount)
    {
      bytesInLine = 0;
    }

    /*
     * build hex and ascii columns
     */
    QString hexColumn;
    QString asciiColumn;
    for ( int o = 0; o < 16; o++ )
    {
      if (o < bytesInLine)
      {
        quint8 b = *(lineAddress + o);
        QString bs = QString("%1").arg((b), 2, 16, QChar('0'));
        hexColumn += bs;
        if(o != bytesInLine-1)
            hexColumn += ' ';

        QChar c( b );
        if (c.isPrint())
          asciiColumn += b;
        else
          asciiColumn += QChar(0x00b7); // QChar(QChar::ReplacementCharacter);

      }
      else
      {
        hexColumn += "  ";
        asciiColumn += " ";
      }

      if (o % 4 == 3)
        hexColumn += " ";
    }

    int textBaseLineY = ((i+1) * fontMetrics().lineSpacing()) - fontMetrics().leading() - 3;

    /*
     * if this is a highlighted line, draw background
     */
    QRect highlightHex(-1, textBaseLineY+2, 0, -ch);
    QRect highlightAscii(-1, textBaseLineY+2, 0, -ch);

    // check for overlap with highlight region
    if (within(m_highlightStartAdr, lineAddress, lineAddress + 16) ||
        within(m_highlightStartAdr + m_highlightSize, lineAddress, lineAddress + 16) ||
        (m_highlightStartAdr <= lineAddress && m_highlightStartAdr + m_highlightSize >= lineAddress + 16))
    {
      const QPair<int,int> hexpx = calculateHexTextArea(hexColumn, m_highlightStartAdr - lineAddress, m_highlightStartAdr + m_highlightSize - lineAddress);
      highlightHex.setLeft(hexX + hexpx.first);
      highlightHex.setRight(hexX + hexpx.second);

      const QPair<int,int> asciipx = calculateAsciiTextArea(asciiColumn, m_highlightStartAdr - lineAddress, m_highlightStartAdr + m_highlightSize - lineAddress);
      highlightAscii.setLeft(asciiX + asciipx.first);
      highlightAscii.setRight(asciiX + asciipx.second);

      painter.save();
      painter.setPen( Qt::NoPen );
      painter.setBrush( g_highlightColor );
      painter.drawRect( highlightHex );
      painter.drawRect( highlightAscii );
      painter.restore();
    }

    /*
     * if the cursor positioned in this line, draw it
     */
    if (m_cursorPosition / 16 == verticalScrollBar()->value() + i)
    {
      const int cursorLineOffset = m_cursorPosition % 16;
      QPair<int,int> hex_px = calculateHexTextArea(hexColumn, cursorLineOffset, cursorLineOffset+1);
      QPair<int,int> ascii_px = calculateAsciiTextArea(asciiColumn, cursorLineOffset, cursorLineOffset+1);
      painter.save();
      painter.setPen(Qt::darkGray);
      painter.drawRect(hexX + hex_px.first - 1, textBaseLineY - ch + 1, hex_px.second - hex_px.first + 1, ch);  // hex cursor
      painter.drawRect(asciiX + ascii_px.first - 1, textBaseLineY - ch + 1, ascii_px.second - ascii_px.first + 1, ch); // ascii cursor
      painter.restore();
    }


    /*
     * build position column
     */
    QString posColumn = QString("%1").arg(quint64(verticalScrollBar()->value() * 16 + i*16), 16, 16, QChar('0'));
    if (m_offsetColumnMode == SIZE32)
    {
      posColumn = posColumn.right(8);
      posColumn += ": ";
    } else if (m_offsetColumnMode == SIZE64)
    {
      posColumn += ": ";
    } else if (m_offsetColumnMode == NONE)
    {
      posColumn = "";
    }

    painter.drawText(H_MARGIN, textBaseLineY, posColumn);
    painter.drawText(hexX, textBaseLineY, hexColumn.toUpper() ); // draw line address column + hex column
    painter.drawText(asciiX, textBaseLineY, asciiColumn );
  }
}


QSize HexEdit::sizeHint() const
{
  return m_sizeHint;
}


void HexEdit::resizeEvent(QResizeEvent* event)
{
  QAbstractScrollArea::resizeEvent(event);
  updateScrollbar();
}


/* configures vscrollbar so that its value reflects the
 * first to be displayed line
 *
 */
void HexEdit::updateScrollbar()
{
  verticalScrollBar()->setRange(0, m_lineCount - visibleLines());
  viewport()->update();
}


void HexEdit::scrollContentsBy(int dx, int dy)
{
  int pixeldy = dy * fontMetrics().lineSpacing();;
  if (qAbs(pixeldy) < viewport()->height())
  {
    viewport()->scroll( 0, pixeldy );
  }
  else
  {
    viewport()->update();
  }
}


void HexEdit::keyPressEvent(QKeyEvent * event)
{
  switch (event->key())
  {
    case Qt::Key_Left:
      if (m_cursorPosition > 0)
      {
        m_cursorPosition--;
        ensureCursorVisible();
        emit cursorPosition( m_cursorPosition );
        viewport()->update();
      }
      break;

    case Qt::Key_Right:
      if (m_cursorPosition < m_dataSize-1)
      {
        m_cursorPosition++;
        ensureCursorVisible();
        emit cursorPosition( m_cursorPosition );
        viewport()->update();
      }
      break;

    case Qt::Key_Down:
      if (m_cursorPosition < m_dataSize - 16)
      {
        m_cursorPosition += 16;
        ensureCursorVisible();
        emit cursorPosition( m_cursorPosition );
        viewport()->update();
      }
      break;

    case Qt::Key_Up:
      if (m_cursorPosition >= 16)
      {
        m_cursorPosition -= 16;
        ensureCursorVisible();
        emit cursorPosition( m_cursorPosition );
        viewport()->update();
      }
      break;

    case Qt::Key_PageDown:
      if (m_cursorPosition / 16 + visibleLines() < m_dataSize / 16)
      {
        m_cursorPosition += visibleLines() * 16;
        verticalScrollBar()->setValue(m_cursorPosition / 16);
        emit cursorPosition( m_cursorPosition );
      }
      break;

    case Qt::Key_PageUp:
      if (m_cursorPosition / 16 - visibleLines() >= 0)
      {
        m_cursorPosition -= visibleLines() * 16;
        ensureCursorVisible();
        emit cursorPosition( m_cursorPosition );
        viewport()->update();
      }
      break;

    default:
      QAbstractScrollArea::keyPressEvent( event );
      break;
  }
}

void HexEdit::ensureCursorVisible()
{
  if (m_cursorPosition / 16 < verticalScrollBar()->value())
  {
    // cursor above viewport
    verticalScrollBar()->setValue( m_cursorPosition / 16 );
  }
  else if ((m_cursorPosition / 16) + 1 >= verticalScrollBar()->value() + visibleLines())
  {
    // cursor below viewport
    verticalScrollBar()->setValue( m_cursorPosition / 16 - visibleLines() + 1);
  }
}


/**
 * @brief HexEdit::calculateHexTextArea
 * Calculate start and end pos in pixels in hex string
 * @param hexStr
 * @param start start chr index
 * @param end end chr index (exclusive)
 * @return
 */
QPair<int,int> HexEdit::calculateHexTextArea(const QString &hexStr, int start, int end) const
{
  start = qBound(0, start, 16);
  end = qMin(end, 16);
  int start_char_ofs = (start/4) * (8+3+2) + (start%4) * 3;
  int end_char_ofs = (end/4) * (8+3+2) + (end%4) * 3 - 1;

  if (end % 4 == 0 && end > 0)  // end falls on last byte of 4byte column
  {
    end_char_ofs -= 1;          // remove trailing space
  }

  int startlen = fontMetrics().width(hexStr.left(start_char_ofs));
  int endlen = fontMetrics().width(hexStr.left(end_char_ofs));

  return {startlen, endlen};
}


QPair<int, int> HexEdit::calculateAsciiTextArea(const QString &asciiStr, int start, int end) const
{
  int startlen = fontMetrics().width(asciiStr.left(qBound(0, start, 16)));
  int endlen = fontMetrics().width(asciiStr.left(qMin(end, 16)));

  return {startlen, endlen};
}


/* number of lines visible
 * based on widget height and data size
 */
int HexEdit::visibleLines() const
{
  return viewport()->height() / fontMetrics().lineSpacing();
}


void HexEdit::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    setCursorPosition(m_data + mousePosToOffset(event->pos()));
}


void HexEdit::contextMenuEvent(QContextMenuEvent *e)
{
  QMenu menu;
  QAction* a = menu.addAction("Rescan from offset 0x" + QString::number(m_cursorPosition, 16));
  if (menu.exec(e->globalPos()) == a)
  {
    emit rescanRequested(m_cursorPosition);
  }
}


void HexEdit::setHighlight(const uchar * start, qint64 size, bool ensureVisible)
{
  m_highlightStartAdr = start;
  m_highlightSize = size;

  if (ensureVisible)
    ensureHighlightVisible();

  viewport()->update();
}


void HexEdit::setCursorPosition(const uchar *pos)
{
  if (m_cursorPosition != pos - m_data)
  {
    m_cursorPosition = pos - m_data;
    viewport()->update();
    emit cursorPosition( m_cursorPosition );
  }
}


void HexEdit::ensureHighlightVisible()
{
  if (m_highlightStartAdr == 0)
    return;

  int highlightStartLine = (m_highlightStartAdr - m_data) / 16;
  if (highlightStartLine < verticalScrollBar()->value()
      ||
      highlightStartLine >= (verticalScrollBar()->value() + visibleLines()))
  {
    verticalScrollBar()->setValue( (m_highlightStartAdr - m_data) / 16 );
  }
}


qint64 HexEdit::mousePosToOffset(const QPoint &pos) const
{
  qint64 firstLineOfs = (verticalScrollBar()->value() * 16);  // offset  of first visible line
  int cw = fontMetrics().width( '0' );

  int yofs = 16 * ((pos.y()-1) / fontMetrics().lineSpacing());

  int posMargin;  // size of posStr column in px

  if (m_offsetColumnMode == SIZE32)
  {
    posMargin = H_MARGIN + cw * 10;
  } else if (m_offsetColumnMode == SIZE64)
  {
    posMargin = H_MARGIN + cw * 18;
  } else if (m_offsetColumnMode == NONE)
  {
    posMargin = H_MARGIN;
  }

  int asciiX = posMargin - H_MARGIN + (4*(8+3) + 3*2 + 4)*cw;

  int xofs;
  if (pos.x() < asciiX)
  {
    const int x = pos.x() - posMargin + cw;
    const int blockWidth = (8+3+2)*cw;
    const int byteWidth = 3*cw;
    const int blockNumber = x / blockWidth;
    const int byteInBlock = qBound(0, (x % blockWidth - cw/2) / byteWidth, 3);
    xofs = blockNumber * 4 + byteInBlock;
  }
  else
    xofs = (pos.x() - asciiX) / cw;

  return qBound((qint64)0, firstLineOfs + yofs + qBound(0, xofs, 15), m_dataSize);
}

