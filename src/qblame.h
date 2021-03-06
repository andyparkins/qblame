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
#include <QAbstractItemModel>
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
	TCommitMeta() : Boundary(false) {}

	QString Hash;
	TEvent Author;
	TEvent Committer;
	QString Summary;
	QString Filename;
	bool Boundary;
	QList<TCommitMeta *> Parent;
};

//
// Class:	TBlameLine
// Description:
//
class TBlameLine
{
  public:
	unsigned int SourceLine;
	TCommitMeta *Commit;
	QString data;
};

//
// Class:	TBlameModel
// Description:
//
class TBlameModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
	enum {
		RowFile,
		RowHistory,
		ROOT_TYPE_COUNT
	};
	enum {
		FileRoot,
		HistoryRoot,
		FileIndex,
		HistoryIndex,
		NODE_TYPE_INVALID
	};

  public:
	TBlameModel( const QString &, QWidget * = NULL );
	~TBlameModel();

	void init();

	// --- QAbstractItemModel overrides
	QVariant data( const QModelIndex &index, int role ) const;
	QVariant dataFileRoot( const QModelIndex &index, int role ) const;
	QVariant dataHistoryRoot( const QModelIndex &index, int role ) const;
	QVariant dataFileIndex( const QModelIndex &index, int role ) const;
	QVariant dataHistoryIndex( const QModelIndex &index, int role ) const;
//	Qt::ItemFlags flags( const QModelIndex &index ) const;
//	QVariant headerData( int section, Qt::Orientation orientation,
//			int role = Qt::DisplayRole ) const;
	QModelIndex index( int row, int column,
			const QModelIndex &parent = QModelIndex() ) const;
	QModelIndex parent( const QModelIndex &index ) const;
	int rowCount( const QModelIndex &parent = QModelIndex() ) const;
	int columnCount( const QModelIndex &parent = QModelIndex() ) const;

  protected slots:
	void readMore();
	void announceStarted();
	void announceFinished( int, QProcess::ExitStatus );

  protected:
	void preloadFile();
	void parseLine( const QString & );

  protected:
	enum eParseState {
		NEW_BLOCK,
		IN_BLOCK
	} ParseState;
	enum eFileColumnMap {
		ColumnHash,
		ColumnAuthor,
		ColumnSummary,
		ColumnLine,
		ColumnData,
		FILE_COLUMN_COUNT
	};
	enum eHistoryColumnMap {
		HColumnID,
		HColumnHash,
		HColumnAuthor,
		HColumnSummary,
		HISTORY_COLUMN_COUNT
	};
	QString File;

	QProcess *gitBlame;

	QList<TBlameLine> Lines;
	QMap<QString,TCommitMeta*> Commits;
	TCommitMeta *CurrentMeta;
	TCommitMeta *OldMeta;
	QList<TCommitMeta *> History;
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

  protected:
	void showEvent( QShowEvent * );

	TBlameModel *Model;
};

// -------------- Function prototypes


// -------------- World globals ("extern"s only)


// -------------- Namespace (ENDS)
//}
// End of conditional compilation
#endif

