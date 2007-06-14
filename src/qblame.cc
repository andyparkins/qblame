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
#include <QFile>
#include <QHeaderView>
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
	QWidget( p )
{
	setupUi( this );

	Model = new TBlameModel( file, this );

	listBlame->setRootIsDecorated(false);
//	listBlame->header()->hide();
}

//
// Function:	TBlameWindow :: ~TBlameWindow
// Description:
//
TBlameWindow::~TBlameWindow()
{
	delete Model;
}

//
// Function:	TBlameWindow :: showEvent
// Description:
//
void TBlameWindow::showEvent( QShowEvent *event )
{
	if( event->spontaneous() )
		return;

	Model->init();
	listBlame->setModel( Model );
}


// --------------------------------------------

//
// Function:	TBlameModel :: TBlameModel
// Description:
//
TBlameModel::TBlameModel( const QString &file, QWidget *p ) :
	QAbstractItemModel( p ),
	ParseState( NEW_BLOCK ),
	File( file )
{
	gitBlame = new QProcess( this );
	connect( gitBlame, SIGNAL( readyRead() ),
			this, SLOT( readMore() ) );
	connect( gitBlame, SIGNAL( started() ),
			this, SLOT( announceStarted() ) );
	connect( gitBlame, SIGNAL( finished( int, QProcess::ExitStatus ) ),
			this, SLOT( announceFinished( int, QProcess::ExitStatus ) ) );

}

//
// Function:	TBlameModel :: ~TBlameModel
// Description:
//
TBlameModel::~TBlameModel()
{
	delete gitBlame;
}

//
// Function:	TBlameModel :: showEvent
// Description:
//
void TBlameModel::init()
{
	preloadFile();

	gitBlame->start( "git-blame", QStringList()
			<< "--incremental"
			<< File
			);
}

//
// Function:	TBlameModel :: preloadFile
// Description:
//
void TBlameModel::preloadFile()
{
	// File handle
	QFile fh( File );
	fh.open( QIODevice::ReadOnly );

	QByteArray ba;

	while( !fh.atEnd() ) {
		ba = fh.readLine();

		Lines.append( TBlameLine() );
		Lines.last().data = ba;
		Lines.last().Commit = NULL;

		// Remove the newline
		Lines.last().data.chop(1);
	}

	cerr << "LOAD: Read " << Lines.size() << " lines" << endl;
}

//
// Function:	TBlameModel :: readMore
// Description:
//
void TBlameModel::readMore()
{
	QByteArray ba;
	QString x;

	do {
		if( !gitBlame->canReadLine() )
			break;

		ba = gitBlame->readLine();
		x = ba;

		// There is always a newline at the end, so remove it
		x.chop(1);
		parseLine(x);
	} while( ba.size() > 0 );

}

//
// Function:	TBlameModel :: parseLine
// Description:
//
void TBlameModel::parseLine( const QString &line )
{
	QStringList Field = line.split( ' ' );

	unsigned int SourceLine, ResultLine, NumLines;
	QString Hash;
	QString Key, Value;
	QModelIndex begin;
	QModelIndex end;

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
				Lines[ResultLine + i - 1].Commit = CurrentMeta;
				Lines[ResultLine + i - 1].SourceLine = SourceLine + i;
			}

			// Tell the world that the hash has changed
			begin = createIndex(ResultLine-1,ColumnHash);
			end = createIndex(ResultLine+NumLines-1,COLUMN_COUNT-1);
			emit dataChanged( begin, end );

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
				cerr << "Unknown key in blame block "
					<< qPrintable(CurrentMeta->Hash)
					<< ": "
					<< qPrintable(Key) << " = " 
					<< qPrintable(Value) << endl;
			}
			break;
	}

}

//
// Function:	TBlameModel :: announceStarted
// Description:
//
void TBlameModel::announceStarted()
{
	cerr << "PROCESS: Started " << gitBlame->pid() << endl;
}

//
// Function:	TBlameModel :: announceFinished
// Description:
//
void TBlameModel::announceFinished( int ExitCode, QProcess::ExitStatus )
{
	cerr << "PROCESS: Finished with exit code " << ExitCode << endl;

//	foreach( const TBlameLine &L, Lines ) {
//		cerr << qPrintable( L.Commit->Hash ) << " " << qPrintable( L.Commit->Summary ) << endl;
//	}
}

//
// Function:	TBlameModel :: announceFinished
// Description:
//
QVariant TBlameModel::data( const QModelIndex &index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	const TBlameLine *L = &Lines[index.row()];

	switch( role ) {
		// --- General Purpose Roles
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
			switch( index.column() ) {
				case ColumnHash:
					return L->Commit ? L->Commit->Hash : QVariant();
				case ColumnAuthor:
					return L->Commit ? L->Commit->Author.Name : QVariant();
				case ColumnSummary:
					return L->Commit ? L->Commit->Summary : QVariant();
				case ColumnLine:
					return index.row()+1;
				case ColumnData:
					return L->data;
			}

			break;
		case Qt::DecorationRole:
		case Qt::EditRole:
		case Qt::StatusTipRole:
		case Qt::WhatsThisRole:
		case Qt::SizeHintRole:
			break;

		// --- Appearance Roles
		case Qt::FontRole:
			switch( index.column() ) {
				case ColumnData: {
					QFont f;
					f.setFamily("Monospace");
					return f;
				}
				default:
					break;
			}
		case Qt::TextAlignmentRole:
		case Qt::BackgroundRole:
			break;
		case Qt::ForegroundRole:
			break;
		case Qt::CheckStateRole:
			break;

		// --- Everything else
		default:
			break;
	}
	return QVariant();
}

//
// Function:	TBlameModel :: index
// Description:
//
QModelIndex TBlameModel::index( int row, int column, const QModelIndex &parent ) const
{
	if( parent != QModelIndex() )
		return QModelIndex();

	return createIndex( row, column );
}

//
// Function:	TBlameModel :: parent
// Description:
//
QModelIndex TBlameModel::parent( const QModelIndex &index ) const
{
	// There is only one parent
	return QModelIndex();
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


