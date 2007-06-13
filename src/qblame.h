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


// -------------- Class declarations

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
	void preloadFile( const QString & );

  protected:
	QProcess *gitBlame;
};

// -------------- Function prototypes


// -------------- World globals ("extern"s only)


// -------------- Namespace (ENDS)
//}
// End of conditional compilation
#endif

