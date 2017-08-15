#ifndef UTIL_H
#define UTIL_H

#include <QtGlobal> // for debug macros
#ifdef QT_DEBUG
#include <QDebug>
#endif
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QXmlStreamReader>
#include <memory>
#include <QTableWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QStatusBar>
#include <string.h>

/**
  Utilities functions (global)
  **/
namespace Util{

namespace FileSystem {

QString normalizePath(QString path);

QString cutName(QString path);

QString cutNameWithoutBackSlash(QString path);

QString normalizeAndQuote(QString path);

bool copyDir(const QString &fromPath, QString toPath, const bool isRecursive = false);

bool rmDir(const QString &dirPath);

QStringList getFolderFilesByWildcard(const QString &entryFolder, const QString &wildcard, bool isRecursive = false);

QStringList filterFilesByWildcard(const QStringList &filePaths, const QString &wildcard);

QString fileHash(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);

QString getAppPath();

}

namespace String {

// Not using a function because we need to have this as inline function (in both debug and release mods)
// QString(string).toUtf8().constData() -> since this creates a temporary it needs to be inline
#ifndef QSTR_TO_CSTR
#  define QSTR_TO_CSTR qUtf8Printable
#endif

QString insertApostrophes(const QString &currString);

QString insertQuotes(const QString &currString);

QString fullTrim(QString str);

QStringList substring(QString myString, QString separator, Qt::CaseSensitivity cs = Qt::CaseSensitive);

QString normalizeDecimalSeparator(QString value);

// no problem here with "temporary" cstr
// https://stackoverflow.com/questions/1971183/when-does-c-allocate-deallocate-string-literals
const char* boolToCstr(bool currentBoolean);

}

namespace Dialogs {

void showInfo(const QString &message);

void showRichInfo(const QString &message);

void showWarning(const QString &message);

void showError(const QString &message);

void showRichError(const QString &message);

bool showQuestion(QWidget * parent, QString message, QMessageBox::StandardButton standardButton=QMessageBox::NoButton);

QMessageBox::StandardButton showQuestionWithCancel(QWidget * parent, QString message, QMessageBox::StandardButton standardButton=QMessageBox::NoButton);

QStringList multipleDirSelection(const QString &title);

}

namespace Validation {

bool checkEmptySpaces(QStringList toCheck);
bool checkIfIntegers(QStringList toCheck);
bool checkIfDoubles(QStringList toCheck);
bool isStringInteger(QString myString);
bool isStringDouble(QString myString);

}

namespace TableWidget {

void addRow(QTableWidget *myTable, QStringList &columns);
QModelIndexList getSelectedRows(QTableWidget *myTable);
QModelIndexList getCurrentRows(QTableWidget *myTable);
int getNumberSelectedRows(QTableWidget *myTable);
void clearContents(QTableWidget *myTable, const QString &nothingToClearMessage, const QString &questionToClear);
void clearContentsNoPrompt(QTableWidget *myTable);
void addCheckBox(QTableWidget *myTable, int row, int column, QCheckBox *checkbox = nullptr);
QCheckBox* getCheckBoxFromCell(QTableWidget *myTable, int row, int column);
void swapRows(QTableWidget *myTable, const int indexSourceRow, const int indexDestinationRow, bool selectSwappedRow);
void deleteSelectedRows(QTableWidget *myTable);

}

namespace System {
QRect getScreenResolution();
}

namespace StatusBar {

void showError(QStatusBar * const statusBar, const QString &message);
void showSuccess(QStatusBar * const  statusBar,const QString &message);

}

}
#endif // UTIL_H
