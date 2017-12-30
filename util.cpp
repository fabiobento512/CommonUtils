/**
 * Copyright (C) 2017 - Fábio Bento (random-guy)
 *
 * This library is distributed under the MIT License. See notice at the end
 * of this file.
 *
 */

#include "util.h"

namespace Util{

namespace FileSystem {

QString normalizePath(QString path){
    return path.replace("\\","/");
}

QString cutName(QString path){
    return path.remove(0,path.lastIndexOf('/')).remove('"');
}

QString cutNameWithoutBackSlash(QString path){
    return cutName(path).remove('/');
}

QString normalizeAndQuote(QString path){
    return String::insertQuotes(normalizePath(path));
}

// Created from scratch
bool copyDir(const QString &fromPath, QString toPath, const bool isRecursive){
    QDir fromDir(fromPath);
    QDir toDir(toPath);

    if(!toDir.mkdir(fromDir.dirName())){ // create the folder in the destination
        return false;
    }

    // Update toPath to include the folder from "fromPath"
    toPath = toPath + "/" + fromDir.dirName();
    toDir = QDir(toPath);

    for(const QFileInfo &currFileInfo : fromDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)){

        if(currFileInfo.isFile()){

            QFile destFile(toPath + "/" + currFileInfo.fileName());

            if(!QFile::copy(currFileInfo.absoluteFilePath(),toPath + "/" + currFileInfo.fileName())){
                return false;
            }
        }
        else if(isRecursive && currFileInfo.isDir() && currFileInfo.absoluteFilePath() != fromDir.absolutePath()){

            if(!copyDir(currFileInfo.absoluteFilePath(), toPath, isRecursive)){
                return false;
            }
        }
    }

    return true;
}

//Copied from here: http://stackoverflow.com/questions/2536524/copy-directory-using-qt (ty roop)
bool rmDir(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return true;
    for(const QFileInfo &info : dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isDir()) {
            if (!rmDir(info.filePath()))
                return false;
        } else {
            if (!dir.remove(info.fileName()))
                return false;
        }
    }
    QDir parentDir(QFileInfo(dirPath).path());
    return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

// Gets all files from a folder filtered by a given wildcard
QStringList getFolderFilesByWildcard(const QString &entryFolder, const QString &wildcard, bool isRecursive){

    QStringList filesFound; // result files with absolute path

    QDirIterator it(entryFolder, QDir::Files, (isRecursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags));

    while (it.hasNext()){
        filesFound << it.next();
    }

    return filterFilesByWildcard(filesFound, wildcard);
}

// Supports wildcards, and subdirectories with wildcard e.g.:
// *.xml
// /myXmls/*.xml
//
// online helper: https://regex101.com/
QStringList filterFilesByWildcard(const QStringList &filePaths, const QString &wildcard){
    QStringList resultFiles;
    QString formattedWildcard;

    if(wildcard.trimmed().isEmpty()){
        return resultFiles;
    }

    formattedWildcard=normalizePath(wildcard); // Convert slashes to work in both mac and windows

    // escape the string so '.' or '(' chars get correctly escaped
    formattedWildcard = QRegularExpression::escape(formattedWildcard);

    // replace * by the corresponding regex
    formattedWildcard.replace("\\*",".*");

    // replace ? by the corresponding regex
    formattedWildcard.replace("\\?",".");

    // if it doesn't start with any regex wildcard or a subdirectory slash, add a slash to beginning (so the file/folder matches at least the root folder)
    // We use \\/ instead of / because it was escaped
    if(!formattedWildcard.startsWith("\\/") && !formattedWildcard.startsWith(".*") && !formattedWildcard.startsWith(".")){
        formattedWildcard = "\\/" + formattedWildcard;
    }

    // if it is a subdirectory add * to match
    if(formattedWildcard.startsWith("\\/")){
        formattedWildcard = ".*" + formattedWildcard;
    }

    formattedWildcard = "^" + formattedWildcard + "$"; // we want a full match (http://stackoverflow.com/a/5752852)

    QRegularExpression regex(formattedWildcard);

    for(const QString &currentFile : filePaths){

        if(regex.match(currentFile).hasMatch()){
            resultFiles << currentFile;
        }

    }

    return resultFiles;
}

// Returns empty QString on failure.
// Based from here: http://www.qtcentre.org/archive/index.php/t-35674.html (thanks wysota!)
QString fileHash(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm)
{

    QCryptographicHash crypto(hashAlgorithm);
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    while(!file.atEnd()){
        crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result();

    return QString(crypto.result().toHex());

}


/**
  Gets application directory. In mac os gets the .app directory
  **/
QString getAppPath(){
#ifdef Q_OS_MAC
    QDir dir = QDir(QCoreApplication::applicationDirPath());
    if(dir.absolutePath().contains(".app")){ // include bundle, but we don't want it
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
    }
    return dir.absolutePath();
#else
    return  QDir::currentPath();
#endif
}

bool backupFile(const QString &file, QString newFilename){
	
	if(newFilename.isEmpty()){
		newFilename = file;
	}
	
    return QFile::copy(file, newFilename+".bak");
}

}

namespace String {

QString insertApostrophes(const QString &currString){
    return "'"+currString+"'";
}

QString insertQuotes(const QString &currString){
    return "\""+currString+"\"";
}

QString fullTrim(QString str) {

    str = str.simplified(); //convert all invisible chars in normal whitespaces
    str.replace( " ", "" );

    return str;
}

QStringList substring(QString myString, QString separator, Qt::CaseSensitivity cs){
    QStringList result = QStringList();
    int currIdx=0, nextIdx=0;

    while(true){
        nextIdx=myString.indexOf(separator,currIdx,cs);
        result << myString.mid(currIdx,nextIdx-currIdx);
        if(nextIdx==-1) break;
        currIdx=nextIdx+1;
    }

    return result;
}

QString normalizeDecimalSeparator(QString value){
    return value.replace(',','.');
}

//Searches for the QString "toSearch" in the "myString" variable backward
//Returns the index of the first match or -1 if not found
int indexOfBackward(QString myString, QString toSearch, int from){
    int myStringSize=myString.size();
    int toSearchSize=toSearch.size();

    if(from==-1){
        from=myStringSize;
    }

    int i=from;

    while(i>=0){
        for(int j=toSearchSize-1; j>=0; j--){
            i--;
            if(myString.at(i)!=toSearch.at(j)){
                break;
            }
            if(j==0){
                return i;
            }
        }
    }

    return -1;
}

// no problem here with "temporary" cstr
// https://stackoverflow.com/questions/1971183/when-does-c-allocate-deallocate-string-literals
const char* boolToCstr(bool currentBoolean){
    return currentBoolean ? "true" : "false";
}

}

#ifdef QT_GUI_LIB
namespace Dialogs {

void showInfo(const QString &message){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(message);
    msgBox.exec();
}

void showRichInfo(const QString &message){
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(message);
    msgBox.exec();
}

void showWarning(const QString &message){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(message);
    msgBox.exec();
}

void showError(const QString &message){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    msgBox.exec();
}

void showRichError(const QString &message){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    msgBox.exec();
}

bool showQuestion(QWidget * parent, QString message, QMessageBox::StandardButton standardButton){
    return QMessageBox::question (parent, "Are you sure?", message, QMessageBox::Yes | QMessageBox::No, standardButton)==QMessageBox::Yes;
}

QMessageBox::StandardButton showQuestionWithCancel(QWidget * parent, QString message, QMessageBox::StandardButton standardButton){
    return QMessageBox::question (parent, "Are you sure?", message, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, standardButton);
}

QStringList multipleDirSelection(const QString &title){
    QFileDialog w;

    // We need to use non native dialog, because native doesn't support multiple folder selection
    w.setOption(QFileDialog::DontUseNativeDialog,true);

    w.setFileMode(QFileDialog::DirectoryOnly);

    w.setWindowTitle(title);

    QTreeView *t = w.findChild<QTreeView*>();

    if (t) {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    if(w.exec()){ //if accepted
        return w.selectedFiles();
    }
    return QStringList(); //return empty
}

}
#endif

namespace Validation {

// Check if any string in the list is empty
bool checkEmptySpaces(QStringList toCheck){
    for (const QString &current : toCheck){
        if(current.trimmed().isEmpty()){
            return true; //There are empty spaces
        }
    }
    return false;
}

bool checkIfIntegers(QStringList toCheck){
    for (const QString &current : toCheck){
        if(!isStringInteger(current)){
            return true; // Some aren't valid integers
        }
    }
    return false;
}

bool checkIfDoubles(QStringList toCheck){
    for (const QString &current : toCheck){
        if(!isStringDouble(current)){
            return true; // Some aren't valid doubles
        }
    }
    return false;
}

bool isStringInteger(QString myString){
    bool isNumber;

    myString.toInt(&isNumber); //convert to int and see if it succeeds

    return isNumber;
}

bool isStringDouble(QString myString){
    bool isDouble;

    myString.toDouble(&isDouble); //convert to double and see if it succeeds

    return isDouble;
}

}

namespace System {

#ifdef QT_GUI_LIB
// From here: http://stackoverflow.com/questions/17893328/qt-getting-the-screen-resolution-without-the-extended-monitor ty Chris
QRect getScreenResolution(){
    QDesktopWidget widget;
    return widget.availableGeometry(widget.primaryScreen()); // or screenGeometry(), depending on your needs
}
#endif

}

#ifdef QT_GUI_LIB
namespace TableWidget {


void addRow(QTableWidget *myTable, QStringList &columns){
    //Get actual number rows
    int twSize=myTable->rowCount();

    //increase the rows for the new item
    myTable->setRowCount(twSize+1);

    //Add to table and list to
    for(int i=0; i<columns.size(); i++){
        QTableWidgetItem *newColumn = new QTableWidgetItem(columns[i]);
        myTable->setItem(twSize,i,newColumn);
        // Add a tooltip with with the cell content
        myTable->item(twSize,i)->setToolTip(myTable->item(twSize,i)->text());
    }

}


QModelIndexList getSelectedRows(QTableWidget *myTable){
    return myTable->selectionModel()->selectedRows();
}

QModelIndexList getCurrentRows(QTableWidget *myTable){

    QModelIndexList oldSelection = getSelectedRows(myTable);

    myTable->selectAll();

    QModelIndexList allRows = getSelectedRows(myTable);

    myTable->selectionModel()->clearSelection();

    // Restore old selection
    for(const QModelIndex &currentIndex : oldSelection){
        myTable->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::SelectionFlag::Rows);
    }

    return allRows;
}

int getNumberSelectedRows(QTableWidget *myTable){
    return getSelectedRows(myTable).size();
}

void clearContents(QTableWidget *myTable, const QString &nothingToClearMessage, const QString &questionToClear){

    if(myTable->rowCount()==0){
        Dialogs::showInfo(nothingToClearMessage);
        return;
    }

    if(Dialogs::showQuestion(myTable, questionToClear)){
        clearContentsNoPrompt(myTable);
    }

}

void clearContentsNoPrompt(QTableWidget *myTable){
    myTable->clearContents();
    myTable->setRowCount(0);
}

// Adapted from here:
// http://stackoverflow.com/questions/29176317/qtablewidget-checkbox-get-state-and-location
void addCheckBox(QTableWidget *myTable, int row, int column, QCheckBox *checkbox){
    if(checkbox == nullptr){
        checkbox = new QCheckBox();
    }

    QWidget *auxLayoutWidget = new QWidget(myTable);

    QHBoxLayout* checkBoxLayout = new QHBoxLayout();
    checkBoxLayout->setContentsMargins(0,0,0,0);
    checkBoxLayout->addWidget(checkbox);
    checkBoxLayout->setAlignment(Qt::AlignCenter);
    checkBoxLayout->setSpacing(0);
    auxLayoutWidget->setLayout(checkBoxLayout);

    myTable->setCellWidget(row, column, auxLayoutWidget);
}

// Adapted from here:
// http://stackoverflow.com/questions/29176317/qtablewidget-checkbox-get-state-and-location
QCheckBox* getCheckBoxFromCell(QTableWidget *myTable, int row, int column){
    return dynamic_cast<QCheckBox*>(myTable->cellWidget(row, column)->findChild<QCheckBox *>());
}

// Adapted from here:
// http://www.qtcentre.org/threads/3386-QTableWidget-move-row
// Thanks jpn
void swapRows(QTableWidget *myTable, const int indexSourceRow, const int indexDestinationRow, bool selectSwappedRow)
{
    // takes and returns the whole row
    auto takeRow = [&myTable](int row) -> QList<QTableWidgetItem*>
    {
        QList<QTableWidgetItem*> rowItems;
        for (int col = 0; col < myTable->columnCount(); ++col)
        {
            rowItems << myTable->takeItem(row, col);
        }
        return rowItems;
    };

    // sets the whole row
    auto setRow = [&myTable](int row, const QList<QTableWidgetItem*>& rowItems)
    {
        for (int col = 0; col < myTable->columnCount(); ++col)
        {
            myTable->setItem(row, col, rowItems.at(col));
        }
    };

    // take whole rows
    QList<QTableWidgetItem*> sourceItems = takeRow(indexSourceRow);
    QList<QTableWidgetItem*> destItems = takeRow(indexDestinationRow);

    // set back in reverse order
    setRow(indexSourceRow, destItems);
    setRow(indexDestinationRow, sourceItems);

    if(selectSwappedRow){
        myTable->selectRow(indexDestinationRow);
    }
}

void deleteSelectedRows(QTableWidget *myTable){
    int size = myTable->selectionModel()->selectedRows().size();

    for(int i=0; i<size; i++){
        myTable->removeRow(myTable->selectionModel()->selectedRows().at(size-i-1).row());
    }
}

}
#endif

#ifdef QT_GUI_LIB
namespace StatusBar {

void showError(QStatusBar * const statusBar, const QString &message){

    QPalette myPalete = QPalette();
    myPalete.setColor( QPalette::WindowText, QColor(255,0,0));
    statusBar->setPalette( myPalete );
    statusBar->showMessage(message,10000); //display by 10 seconds

}

void showSuccess(QStatusBar * const statusBar,const QString &message){

    QPalette myPalete = QPalette();
    myPalete.setColor( QPalette::WindowText, QColor(0,150,0));
    statusBar->setPalette( myPalete );
    statusBar->showMessage(message,10000); //display by 10 seconds

}

}
#endif

}


/**
 * Copyright (c) 2017 - Fábio Bento (random-guy)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
