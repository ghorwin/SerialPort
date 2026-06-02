/*!

SerialPort - A wrapper library around QSerialPort with a worker thread handling the port communication.


MIT License

Copyright (c) 2026-now  Andreas Nicolai <andreas.nicolai@gmx.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef SP_SerialPortWorker_H
#define SP_SerialPortWorker_H

#include <QObject>
#include <QSerialPort>

#include "SP_global.h"

namespace SP {

/*! Worker class that runs in the serial communication worker thread.
	This default implementation is pretty dump but encapsulates native QSerialPort communication.

	If you need more advanced functionality, for example fast message response, then re-implement this
	class and define the slot onReadyRead() yourself (which will then be called instead).
*/
class SP_EXPORT SerialPortWorker : public QObject {
	Q_OBJECT

public:
	~SerialPortWorker();

public slots:
	// DO NOT CALL THESE FUNCTIONS DIRECTLY (from GUI thread), but only communicate with the worker thread
	// via queued signal-slot connections.

	/*! Open port. */
	void openPort(const QString & portName, int baudRate, int dataBits, int parity, int stopBits);
	/*! Close the serial connection. */
	void closePort();
	/*! Write data to open port. */
	void writeData(const QByteArray & binaryDataBlock);

signals:
	/*! Emitted, when a chunk of data has been received from serial port. */
	void dataReceived(QByteArray data);
	/*! Emitted when an error occured. */
	void errorOccurred(QString errorMsg);
	/*! Emitted, when serial port becomes connected or disconnected. */
	void connectionStatusChanged(bool connected);

private slots:
	/*! Startup slot, called as first function just before worker thread is about to enter its event loop. */
	void onStartup();
	/*! Connected to QSerialPort::readyRead() */
	void onReadyRead();
	/*! Connected to QSerialPort::errorOccurred() */
	void onErrorOccurred(QSerialPort::SerialPortError error);

private:
	/*! The serial port we wrap (owned).
		Created in onStartup().
	*/
	QSerialPort				*m_serialPort = nullptr;
};


} // namespace SP

#endif // SP_SerialPortWorker_H
