// ----------------------------------------------------------------------------
// Project: qblame
/// @file   qblame.cc
/// @author Andy Parkins
//
// Version Control
//    $Author$
//      $Date$
//        $Id: 6b01d834ceb3f774b497dfcb100a986a1d2f9806 $
//
// Legal
//    Copyright 2006  Andy Parkins
//
// ----------------------------------------------------------------------------

// Module include
#include "qblame.h"

// -------------- Includes
// --- C
// --- C++
#include <iostream>
// --- Qt
#include <QApplication>
// --- OS
#include <signal.h>
// --- Project libs
// --- Project


// -------------- Namespace
// namespace DVRSpace {

// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class member definitions

//
// Function:	TBlameWindow :: TBlameWindow
// Description:
//
TBlameWindow::TBlameWindow( const QString &File, QWidget *p ) :
	QWidget( p )
{
	setupUi( this );

	gitBlame = new QProcess( this );
	connect( gitBlame, SIGNAL( readyRead() ),
			this, SLOT( readMore() ) );
	connect( gitBlame, SIGNAL( started() ),
			this, SLOT( announceStarted() ) );
	connect( gitBlame, SIGNAL( finished( int, QProcess::ExitStatus ) ),
			this, SLOT( announceFinished( int, QProcess::ExitStatus ) ) );

	preloadFile( File );

	gitBlame->start( "git-blame", QStringList()
			<< "--incremental"
			<< File
			);
}

//
// Function:	TBlameWindow :: ~TBlameWindow
// Description:
//
TBlameWindow::~TBlameWindow()
{
	delete gitBlame;
}

//
// Function:	TBlameWindow :: preloadFile
// Description:
//
void TBlameWindow::preloadFile( const QString &File )
{
}

//
// Function:	TBlameWindow :: readMore
// Description:
//
void TBlameWindow::readMore()
{
	QByteArray ba;

	do {
		ba = gitBlame->readLine();
		listBlame->addItem( ba );

		cerr << "RX: " << ba.data();
	} while( ba.size() > 0 );

}

//
// Function:	TBlameWindow :: announceStarted
// Description:
//
void TBlameWindow::announceStarted()
{
	cerr << "PROCESS: Started " << gitBlame->pid() << endl;
}

//
// Function:	TBlameWindow :: announceFinished
// Description:
//
void TBlameWindow::announceFinished( int ExitCode, QProcess::ExitStatus )
{
	cerr << "PROCESS: Finished with exit code " << ExitCode << endl;
}


// -------------- Function Declarations
void signalHandler( int );


// -------------- Function Definitions
void signalHandler( int sig )
{
	if( !qApp )
		return;

	cerr << "SIGNAL: " << strsignal( sig ) << " signal is triggering quit" << endl;
	qApp->quit();
}


// -------------- Namespace (END)
//}

//using namespace DVRSpace;

// -------------- Main

//
// Function:	main
// Description:	.
//
int main( int argc, char *argv[] )
{
	// Create fundamental Qt object
	QApplication *app = NULL;
    struct sigaction sa;
	int retval = 0;

	cerr << "*** Program started" << endl;
//	cerr << "INIT: Revision control system ID of this build is " << VCSID_CONST << endl;
	cerr << "INIT: PID is " << getpid() << endl;

	cerr << "INIT: Setting locale \"" << setlocale( LC_ALL, "" ) << "\"" << endl;

	cerr << "INIT: Setting interrupt handler" << endl;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalHandler;
	sa.sa_flags = SA_RESETHAND;		// One shot
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	try {
		// Create
		app = new QApplication( argc, argv );

		// Blame window
		TBlameWindow *bw = new TBlameWindow( argv[1], NULL );
		bw->show();

		// Now hand control to Qt
		cerr << "INIT: Initialisation complete, passing control to Qt event loop" << endl;
		retval = app->exec();
	} catch( exception &e ) {
		cerr << "EXCEPTION: " << typeid(e).name() << " " << e.what() << endl;
	} catch( ... ) {
		cerr << "EXCEPTION: Caught unknown exception" << endl;
	}

	cerr << "DEINIT: Returned from Qt event loop, shutting down" << endl;
	delete app;
	cerr << "*** Program ended" << endl;

	return retval;
}


