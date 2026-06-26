#include "QAssistantClientCompat.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QProcess>
#include <QUrl>

QAssistantClient::QAssistantClient(const QString &path, QObject *parent)
	: QObject(parent),
	d_path(path),
	d_process(new QProcess(this))
{
	connect(d_process, SIGNAL(finished(int, QProcess::ExitStatus)),
			this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(d_process, SIGNAL(error(QProcess::ProcessError)),
			this, SLOT(processError(QProcess::ProcessError)));
}

QAssistantClient::~QAssistantClient()
{
	closeAssistant();
}

bool QAssistantClient::isOpen() const
{
	return d_process->state() != QProcess::NotRunning;
}

void QAssistantClient::setArguments(const QStringList &args)
{
	d_arguments = args;
}

void QAssistantClient::openAssistant()
{
	startAssistant(QString());
}

void QAssistantClient::closeAssistant()
{
	if (!isOpen())
		return;

	d_process->terminate();
	if (!d_process->waitForFinished(2000))
		d_process->kill();
}

void QAssistantClient::showPage(const QString &page)
{
	if (isOpen() && sendRemoteCommand(page))
		return;

	startAssistant(page);
}

void QAssistantClient::processFinished(int, QProcess::ExitStatus)
{
	emit assistantClosed();
}

void QAssistantClient::processError(QProcess::ProcessError)
{
	emit error(tr("Failed to start Qt Assistant."));
}

QString QAssistantClient::assistantPath() const
{
	if (!d_path.isEmpty()) {
		QFileInfo fi(d_path);
		if (fi.isDir()) {
#ifdef Q_OS_WIN
			const QString direct = fi.absoluteFilePath() + QDir::separator() + QLatin1String("assistant_adp.exe");
			if (QFileInfo(direct).isFile())
				return direct;
			return fi.absoluteFilePath() + QDir::separator() + QLatin1String("assistant.exe");
#else
			const QString direct = fi.absoluteFilePath() + QDir::separator() + QLatin1String("assistant_adp");
			if (QFileInfo(direct).isFile())
				return direct;
			return fi.absoluteFilePath() + QDir::separator() + QLatin1String("assistant");
#endif
		}
		return d_path;
	}

#ifdef Q_OS_WIN
	const QString adp = candidatePath(QLatin1String("assistant_adp.exe"));
	if (!adp.isEmpty())
		return adp;

	const QString assistant = candidatePath(QLatin1String("assistant.exe"));
	if (!assistant.isEmpty())
		return assistant;

	return QLatin1String("assistant_adp.exe");
#else
	const QString adp = candidatePath(QLatin1String("assistant_adp"));
	if (!adp.isEmpty())
		return adp;

	const QString assistant = candidatePath(QLatin1String("assistant"));
	if (!assistant.isEmpty())
		return assistant;

	return QLatin1String("assistant_adp");
#endif
}

QString QAssistantClient::candidatePath(const QString &binary) const
{
	QString path = QCoreApplication::applicationDirPath() + QDir::separator() + binary;
	if (QFileInfo(path).isFile())
		return path;

	path = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator() + binary;
	if (QFileInfo(path).isFile())
		return path;

	return QString();
}

bool QAssistantClient::startAssistant(const QString &page)
{
	const QString path = assistantPath();
	const bool adpAssistant = isAdpAssistant(path);
	QStringList args;

	if (adpAssistant) {
		if (!page.isEmpty())
			args << QLatin1String("-file") << page;
		args << d_arguments;
	} else {
		args << QLatin1String("-enableRemoteControl") << d_arguments;
	}

	if (isOpen())
		closeAssistant();

	d_process->start(path, args);
	if (!d_process->waitForStarted()) {
		emit error(tr("Failed to start Qt Assistant."));
		return false;
	}

	emit assistantOpened();
	if (!adpAssistant && !page.isEmpty())
		sendRemoteCommand(page);
	return true;
}

bool QAssistantClient::isAdpAssistant(const QString &path) const
{
	return QFileInfo(path).baseName().compare(QLatin1String("assistant_adp"), Qt::CaseInsensitive) == 0;
}

bool QAssistantClient::sendRemoteCommand(const QString &page)
{
	const QString path = assistantPath();
	if (isAdpAssistant(path) || !d_process->isWritable())
		return false;

	QString source = page;
	if (QFileInfo(page).isFile())
		source = QUrl::fromLocalFile(QFileInfo(page).absoluteFilePath()).toString();

	QByteArray command = QByteArray("SetSource ") + source.toUtf8() + '\0';
	return d_process->write(command) >= 0;
}
