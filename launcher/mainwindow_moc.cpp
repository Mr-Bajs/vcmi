/*
 * mainwindow_moc.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "mainwindow_moc.h"
#include "ui_mainwindow_moc.h"

#include <QProcess>
#include <QDir>

#include "../lib/CConfigHandler.h"
#include "../lib/VCMIDirs.h"
#include "../lib/filesystem/Filesystem.h"
#include "../lib/logging/CBasicLogConfigurator.h"

#include "updatedialog_moc.h"
#include "main.h"

int __argc;
char ** __argv;

void MainWindow::load()
{
	// Set current working dir to executable folder.
	// This is important on Mac for relative paths to work inside DMG.
	QDir::setCurrent(QApplication::applicationDirPath());

#ifndef VCMI_IOS
	console = new CConsoleHandler();
#endif
	CBasicLogConfigurator logConfig(VCMIDirs::get().userLogsPath() / "VCMI_Launcher_log.txt", console);
	logConfig.configureDefault();

	CResourceHandler::initialize();
	CResourceHandler::load("config/filesystem.json");

#ifdef Q_OS_IOS
	QDir::addSearchPath("icons", pathToQString(VCMIDirs::get().binaryPath() / "icons"));
#else
	for(auto & string : VCMIDirs::get().dataPaths())
		QDir::addSearchPath("icons", pathToQString(string / "launcher" / "icons"));
	QDir::addSearchPath("icons", pathToQString(VCMIDirs::get().userDataPath() / "launcher" / "icons"));
#endif

	settings.init();
}

MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	load(); // load FS before UI

	ui->setupUi(this);

	//load window settings
	QSettings s(Ui::teamName, Ui::appName);

	auto size = s.value("MainWindow/Size").toSize();
	if(size.isValid())
	{
		resize(size);
	}
	auto position = s.value("MainWindow/Position").toPoint();
	if(!position.isNull())
	{
		move(position);
	}

	//set default margins

	auto width = ui->startGameTitle->fontMetrics().boundingRect(ui->startGameTitle->text()).width();
	if(ui->startGameButton->iconSize().width() < width)
	{
		ui->startGameButton->setIconSize(QSize(width, width));
	}
	auto tab_icon_size = ui->tabSelectList->iconSize();
	if(tab_icon_size.width() < width)
	{
		ui->tabSelectList->setIconSize(QSize(width, width + tab_icon_size.height() - tab_icon_size.width()));
		ui->tabSelectList->setGridSize(QSize(width, width));
		// 4 is a dirty hack to make it look right
		ui->tabSelectList->setMaximumWidth(width + 4);
	}
	ui->tabListWidget->setCurrentIndex(0);

	ui->settingsView->isExtraResolutionsModEnabled = ui->stackedWidgetPage2->isExtraResolutionsModEnabled();
	ui->settingsView->setDisplayList();
	connect(ui->stackedWidgetPage2, &CModListView::extraResolutionsEnabledChanged,
		ui->settingsView, &CSettingsView::fillValidResolutions);

	connect(ui->tabSelectList, &QListWidget::currentRowChanged, [this](int i) {
#ifdef Q_OS_IOS
		if(auto widget = qApp->focusWidget())
			widget->clearFocus();
#endif
		ui->tabListWidget->setCurrentIndex(i);
	});

	if(settings["launcher"]["updateOnStartup"].Bool())
		UpdateDialog::showUpdateDialog(false);
}

MainWindow::~MainWindow()
{
	//save window settings
	QSettings s(Ui::teamName, Ui::appName);
	s.setValue("MainWindow/Size", size());
	s.setValue("MainWindow/Position", pos());

	delete ui;
}

void MainWindow::startGame(const QStringList & args)
{
	__argc = args.size() + 1; //first argument is omitted
	__argv = new char*[__argc];
	__argv[0] = "vcmiclient";
	for(int i = 1; i < __argc; ++i)
	{
		const char * s = args[i - 1].toLocal8Bit().constData();
		__argv[i] = new char[strlen(s)];
		strcpy(__argv[i], s);
	}
#ifdef Q_OS_IOS
	logGlobal->warn("Starting game with the arguments: %s", args.join(" ").toStdString());
	qApp->quit();
#else
	startExecutable(pathToQString(VCMIDirs::get().clientPath()), args);
#endif
}

void MainWindow::on_startGameButton_clicked()
{
	startGame({});
}

#ifndef Q_OS_IOS
void MainWindow::startExecutable(QString name, const QStringList & args)
{
	QProcess process;

	// Start the executable
	if(process.startDetached(name, args))
	{
		close(); // exit launcher
	}
	else
	{
		QMessageBox::critical(this,
		                      "Error starting executable",
		                      "Failed to start " + name + "\n"
		                      "Reason: " + process.errorString(),
		                      QMessageBox::Ok,
		                      QMessageBox::Ok);
	}
}
#endif
