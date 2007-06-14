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
#include <QShowEvent>
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
TBlameWindow::TBlameWindow( const QString &file, QWidget *p ) :
	QWidget( p ),
	ParseState( NEW_BLOCK ),
	File( file )
{
	setupUi( this );

	gitBlame = new QProcess( this );
	connect( gitBlame, SIGNAL( readyRead() ),
			this, SLOT( readMore() ) );
	connect( gitBlame, SIGNAL( started() ),
			this, SLOT( announceStarted() ) );
	connect( gitBlame, SIGNAL( finished( int, QProcess::ExitStatus ) ),
			this, SLOT( announceFinished( int, QProcess::ExitStatus ) ) );

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
// Function:	TBlameWindow :: showEvent
// Description:
//
void TBlameWindow::showEvent( QShowEvent *event )
{
	if( event->spontaneous() )
		return;

	preloadFile();

	gitBlame->start( "git-blame", QStringList()
			<< "--incremental"
			<< File
			);
}

//
// Function:	TBlameWindow :: preloadFile
// Description:
//
void TBlameWindow::preloadFile()
{
}

//
// Function:	TBlameWindow :: readMore
// Description:
//
void TBlameWindow::readMore()
{
	QByteArray ba;
	QString x;

	do {
		ba = gitBlame->readLine();
		x = ba;

		// There is always a newline at the end, so remove it
		x.chop(1);
		parseLine(x);
	} while( ba.size() > 0 );

}

//
// Function:	TBlameWindow :: parseLine
// Description:
//
void TBlameWindow::parseLine( const QString &line )
{
	QStringList Field = line.split( ' ' );

	unsigned int SourceLine, ResultLine, NumLines;
	QString Hash;
	QString Key, Value;

	switch( ParseState ) {
		case NEW_BLOCK:
			// Incomplete (probably at end of file)
			if( Field.size() < 4 )
				break;

			// <40-byte hex sha1> <sourceline> <resultline> <num_lines>
			Hash = Field[0];
			SourceLine = Field[1].toUInt();
			ResultLine = Field[2].toUInt();
			NumLines = Field[3].toUInt();

			if( !Commits.contains( Hash ) ) {
				CurrentMeta = new TCommitMeta;
				CurrentMeta->Hash = Field[0];
				// Create a new record for this commit
				Commits[Hash] = CurrentMeta;
			} else {
				CurrentMeta = Commits[Hash];
			}

			// Populate the line map for the target lines to point at
			// the incoming meta.  Note, it's perfectly acceptable to
			// overwrite a previous value because this is an incremental
			// blame and gets more and more accurate
			for( unsigned int i = 0; i < NumLines; i++ ) {
				Lines[ResultLine + i] = CurrentMeta;
			}

			cerr << "New block: " << qPrintable(CurrentMeta->Hash) << endl;

			ParseState = IN_BLOCK;
			break;

		case IN_BLOCK:
			// Incomplete (premature end of file - probably an error)
			if( Field.size() == 0 )
				break;

			Key = Field.takeFirst();
			Value = Field.join(" ");

			if( Key == "author" )
				CurrentMeta->Author.Name = Value;
			else if( Key == "author-mail" )
				CurrentMeta->Author.Mail = Value;
			else if( Key == "author-time" )
				CurrentMeta->Author.Time = Value.toInt();
			else if( Key == "author-tz" )
				CurrentMeta->Author.TimeZone = Value;
			else if( Key == "committer" )
				CurrentMeta->Committer.Name = Value;
			else if( Key == "committer-mail" )
				CurrentMeta->Committer.Mail = Value;
			else if( Key == "committer-time" )
				CurrentMeta->Committer.Time = Value.toInt();
			else if( Key == "committer-tz" )
				CurrentMeta->Committer.TimeZone = Value;
			else if( Key == "summary" )
				CurrentMeta->Summary = Value;
			else if( Key == "boundary" )
				CurrentMeta->Boundary = true;
			else if( Key == "filename" ) {
				CurrentMeta->Filename = Value;
				CurrentMeta = NULL;
				ParseState = NEW_BLOCK;
			} else {
				cerr << "Unknown key in blame block: "
					<< qPrintable(Key) << " = " 
					<< qPrintable(Value) << endl;
			}
			break;
	}

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

	foreach( TCommitMeta *Meta, Lines ) {
		cerr << qPrintable( Meta->Hash ) << " " << qPrintable( Meta->Summary ) << endl;
	}
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


