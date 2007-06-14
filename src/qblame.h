// ----------------------------------------------------------------------------
// Project: qblame
/// @file   qblame.h
/// @author Andy Parkins
//
// Version Control
//    $Author$
//      $Date$
//        $Id$
//
// Legal
//    Copyright 2006  Andy Parkins
//
// ----------------------------------------------------------------------------

// Catch multiple includes
#ifndef QBLAME_H
#define QBLAME_H

// -------------- Includes
// --- C
// --- C++
// --- Qt
#include <QProcess>
// --- OS
// --- Project lib
// --- Project
#include "ui_qblame.h"


// -------------- Namespace
// namespace DVRSpace {
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
//#define		FALSE	0
//#define		TRUE	(!FALSE)
//#define		PI		3.1415926535897932385
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations
class QShowEvent;


// -------------- Class declarations

//
// Class:	TEvent
// Description:
//
class TEvent
{
  public:
	QString Name;
	QString Mail;
	time_t Time;
	QString TimeZone;
};

//
// Class:	TCommitMeta
// Description:
//
class TCommitMeta
{
  public:
	QString Hash;
	TEvent Author;
	TEvent Committer;
	QString Summary;
	QString Filename;
	bool Boundary;
};

//
// Class:	TBlameWindow
// Description:
//
class TBlameWindow : public QWidget, public Ui::QBlame
{
  Q_OBJECT

  public:
	TBlameWindow( const QString &, QWidget * = NULL );
	~TBlameWindow();

  protected slots:
	void readMore();
	void announceStarted();
	void announceFinished( int, QProcess::ExitStatus );

  protected:
	void showEvent( QShowEvent * );

	void preloadFile();
	void parseLine( const QString & );

  protected:
	enum eParseState {
		NEW_BLOCK,
		IN_BLOCK
	} ParseState;
	QString File;

	QProcess *gitBlame;

	QMap<unsigned int,TCommitMeta*> Lines;
	QMap<QString,TCommitMeta*> Commits;
	TCommitMeta *CurrentMeta;
};

// -------------- Function prototypes


// -------------- World globals ("extern"s only)


// -------------- Namespace (ENDS)
//}
// End of conditional compilation
#endif

