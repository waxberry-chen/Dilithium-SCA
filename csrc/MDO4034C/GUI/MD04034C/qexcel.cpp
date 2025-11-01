#include "qexcel.h"
QExcel::QExcel(QObject *parent):QObject(parent)
{
excel = nullptr;
workBooks = nullptr;
workBook = nullptr;
sheets = nullptr;
sheet = nullptr;
}
QExcel::~QExcel()
{
}
/***************************************************************************/
/* 文件操作 */
/**************************************************************************/
void QExcel::open(QString FileName)
{
if(excel==nullptr)
{
excel = new QAxObject();
excel->setControl("Excel.Application"); //连接Excel控件
excel->setProperty("DisplayAlerts", false); //禁止显示警告
workBooks = excel->querySubObject("Workbooks"); //获取工作簿集合
QFile file(FileName);
if(file.exists())
{
workBooks->dynamicCall("Open(const QString&)", FileName); //打开指定文件
workBook = excel->querySubObject("ActiveWorkBook"); //获取活跃工作簿
sheets = workBook->querySubObject("WorkSheets"); //获取工作表集合
sheet = workBook->querySubObject("ActiveSheet"); //获取活跃工作表
}
}
else
{
workBooks = excel->querySubObject("Workbooks"); //获取工作簿集合
QFile file(FileName);
if(file.exists())
{
workBooks->dynamicCall("Open(const QString&)", FileName); //打开指定文件
workBook = excel->querySubObject("ActiveWorkBook"); //获取活跃工作簿
sheets = workBook->querySubObject("WorkSheets"); //获取工作表集合
sheet = workBook->querySubObject("ActiveSheet"); //获取活跃工作表
}
}
}
void QExcel::createFile(QString FileName)
{
if(excel==nullptr)
{
    excel = new QAxObject();
    excel->setControl("Excel.Application"); //连接Excel控件
    excel->setProperty("DisplayAlerts", false); //禁止显示警告
    workBooks = excel->querySubObject("Workbooks"); //获取工作簿集合
    QFile file(FileName);
    if(!file.exists())
    {
        workBooks->dynamicCall("Add"); //新建文件
        workBook=excel->querySubObject("ActiveWorkBook"); //获取活跃工作簿
        workBook->dynamicCall("SaveAs(const QString&)",FileName); //按指定文件名保存文件
        sheets = workBook->querySubObject("WorkSheets"); //获取工作表集合
        sheet = workBook->querySubObject("ActiveSheet"); //获取活跃工作表
    }
}
else{
        workBooks = excel->querySubObject("Workbooks"); //获取工作簿集合
        QFile file(FileName);
        if(!file.exists())
        {
            workBooks->dynamicCall("Add"); //新建文件
            workBook=excel->querySubObject("ActiveWorkBook"); //获取活跃工作簿
            workBook->dynamicCall("SaveAs(const QString&)",FileName); //按指定文件名保存文件
            sheets = workBook->querySubObject("WorkSheets"); //获取工作表集合
            sheet = workBook->querySubObject("ActiveSheet"); //获取活跃工作表
        }
    }
}
void QExcel::save() //保存Excel文件
{
    workBook->dynamicCall("Save()");
    qDebug()<<"save ok";
}
void QExcel::quit() //退出Excel
{
excel->dynamicCall("Quit()");
delete sheet;
delete sheets;
delete workBook;
delete workBooks;
delete excel;
excel = nullptr;
workBooks = nullptr;
workBook = nullptr;
sheets = nullptr;
sheet = nullptr;
qDebug()<<"退出Excel";
}
/**************************************************************************/
/* 工作表操作 */
/*************************************************************************/
void QExcel::selectSheet(const QString& sheetName) //根据名字选择工作表
{
sheet = sheets->querySubObject("Item(const QString&)", sheetName);
}
void QExcel::selectSheet(int sheetIndex) //根据下标索引选择工作表，下标从1开始
{
sheet = sheets->querySubObject("Item(int)", sheetIndex);
}
void QExcel::deleteSheet(const QString& sheetName) //根据名字删除工作表
{
QAxObject * a = sheets->querySubObject("Item(const QString&)", sheetName);
a->dynamicCall("delete");
}
void QExcel::deleteSheet(int sheetIndex) //根据下标索引删除工作表，下标从1开始
{
QAxObject * a = sheets->querySubObject("Item(int)", sheetIndex);
a->dynamicCall("delete");
}
void QExcel::insertSheet(QString sheetName) //新建工作表
{
sheets->querySubObject("Add()");
QAxObject * a = sheets->querySubObject("Item(int)", 1);
a->setProperty("Name", sheetName);
}
int QExcel::getSheetsCount() //获取工作表数量
{
return sheets->property("Count").toInt();
}
QString QExcel::getSheetName() //获取当前活跃工作表的名字
{
return sheet->property("Name").toString();
}
QString QExcel::getSheetName(int sheetIndex) //根据下标索引获取工作表名字
{
QAxObject * a = sheets->querySubObject("Item(int)", sheetIndex);
return a->property("Name").toString();
}
/***************************************************************************/
/* 单元格操作 */
/**************************************************************************/
void QExcel::mergeCells(int topLeftRow, int topLeftColumn, int bottomRightRow, int bottomRightColumn) //根据行列编号合并单元格
{
QString cell;
cell.append(QChar(topLeftColumn - 1 + 'A'));
cell.append(QString::number(topLeftRow));
cell.append(":");
cell.append(QChar(bottomRightColumn - 1 + 'A'));
cell.append(QString::number(bottomRightRow));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("VerticalAlignment", -4108);//xlCenter
range->setProperty("WrapText", true);
range->setProperty("MergeCells", true);
}
void QExcel::mergeCells(const QString& cell) //根据字母编号合并单元格
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("VerticalAlignment", -4108);//xlCenter
range->setProperty("WrapText", true);
range->setProperty("MergeCells", true);
}
void QExcel::setCellString(int row, int column, const QString& value) //根据行列编号设置单元格数据
{
QAxObject *range = sheet->querySubObject("Cells(int,int)", row, column);
range->dynamicCall("SetValue(const QString&)", value);
}



QString QExcel::to26AlphabetString(int data)
{
    QChar ch = data + 0x40;//A对应0x41
    return QString(ch);
}



void QExcel::convertToColName(int data, QString &res)
{
    Q_ASSERT(data>0 && data<65535);
    int tempData = data / 26;
    if(tempData > 0)
    {
        int mode = data % 26;
         convertToColName(mode,res);
         convertToColName(tempData,res);
   }
     else
     {
        res=(to26AlphabetString(data)+res);
     }
 }

void QExcel::htz_Write_row(int16_t Buffer[], int row, int Length) //根据行列编号设置单元格数据
{
    QString  range_str;
    range_str = "A";
    range_str += QString::number(row);
    range_str += ":";
    range_str += getRange_fromColumn(Length);
    range_str += QString::number(row);

    QAxObject *range;
    range = new QAxObject();
    range = sheet->querySubObject("Range(const QString&)",range_str);
    if(range == NULL) {
        range_str = range_str + range_str;
    }
    QVariant var;
    QVariantList  list;
    QVariant succ;
    for(int i=0;i<Length;i++)
    {
        list.append(Buffer[i]);
    }
    var = list;

    //setRowData(3,1,5,var);
    //succ = range->setProperty("SetValue(QVariant)", var);
    range->dynamicCall("SetValue(const QVariant&)",QVariant(var));
    delete range;
    //range->dynamicCall("Value", var);
    //range->dynamicCall("SetValue(QVariant)", var);
}

void QExcel::htz_Read_col(int16_t Buffer[], int col, int Length,int coef_index) //根据行列编号设置单元格数据
{
    QString  range_str;

    QString Length_str = QString::number(4*Length);
    range_str = getRange_fromColumn(col);
    range_str += "1";
    range_str += ":";
    range_str += getRange_fromColumn(col);
    range_str += Length_str;

    double fuck[1000];
    QAxObject *range;
    //range = new QAxObject;
    range = sheet->querySubObject("Range(const QString&)",range_str);
    if(range == NULL) {
        range_str = range_str + range_str;
    }
    QVariant excel_data = range->dynamicCall("Value()");//range->property("Value");

    QVariantList  list = excel_data.toList();
    QVariantList  list2;
    for(int i=0;i<Length;i++)
    {
        list2 = list[4*i + coef_index].toList();
        fuck[i] = list2[0].toDouble();
        Buffer[i] = fuck[i];

    }
    delete range;
}

void QExcel::htz_Read_col2(int16_t Buffer[], int col, int Length,int coef_index) //根据行列编号设置单元格数据
{
    QString  range_str;

    QString Length_str = QString::number(Length + 10);
    range_str = getRange_fromColumn(col);
    range_str += "11";
    range_str += ":";
    range_str += getRange_fromColumn(col);
    range_str += Length_str;

    double fuck[3000];
    QAxObject *range;
    //range = new QAxObject;
    range = sheet->querySubObject("Range(const QString&)",range_str);
    if(range == NULL) {
        range_str = range_str + range_str;
    }
    QVariant excel_data = range->dynamicCall("Value()");//range->property("Value");

    QVariantList  list = excel_data.toList();
    QVariantList  list2;
    for(int i=0;i<Length;i++)
    {
        list2 = list[i].toList();
        fuck[i] = list2[0].toDouble();
        Buffer[i] = fuck[i];

    }
    delete range;
}

void QExcel::setCellString(const QString& cell, const QString& value) //根据字母编号设置单元格数据
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->dynamicCall("SetValue(const QString&)", value);
}
bool QExcel::setRowData(int row,int colum_start,int column_end,QVariant vData) //批量写入一行数据
{
bool op = false;
QString start,end;
start=getRange_fromColumn(colum_start);
end=getRange_fromColumn(column_end);
QVariant qstrRange = start+QString::number(row,10)+":"+end+QString::number(row,10);
QAxObject *range = sheet->querySubObject("Range(const QString&)", qstrRange);
if ( range )
{
range->dynamicCall("SetValue(const QVariant&)",QVariant(vData)); //修改单元格的数据
op = true;
}
else
{
op = false;
}
delete range;
return op;
}
QString QExcel::getRange_fromColumn(int column) //根据列值计算出字母值
{
if(column <= 0) //列值必须大于等于1
return "";
QString ABC="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
QString result="";
QVector<int> pos;
if(column>=1 && column<=26)
{
result += ABC[column-1];
return result;
}
else
{
int high = column;
int low;
int last_high;
while(high>0)
{
last_high=high;
high = high / 26;
low = last_high % 26;
if(low==0 && high!=0)
{
low=26;
high=high-1;
}
pos.push_front(low);
}
for(int i=0; i<pos.size(); i++)
{
result += ABC[pos[i]-1];
}
}
return result;
}
QVariant QExcel::getCellValue(int row, int column) //根据行列编号获取单元格数值
{
QAxObject *range = sheet->querySubObject("Cells(int,int)", row, column);
return range->property("Value");
}
QVariant QExcel::getCellValue(const QString& cell) //根据字母编号获取单元格数值
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
return range->property("Value");
}
void QExcel::clearCell(int row, int column) //根据行列编号清空单元格
{
QString cell;
cell.append(QChar(column - 1 + 'A'));
cell.append(QString::number(row));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->dynamicCall("ClearContents()");
}
void QExcel::clearCell(const QString& cell) //根据字母编号清空单元格
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->dynamicCall("ClearContents()");
}
void QExcel::setCellFontBold(int row, int column, bool isBold) //根据行列编号设置单元格字体是否加粗
{
QString cell;
cell.append(QChar(column - 1 + 'A'));
cell.append(QString::number(row));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range = range->querySubObject("Font");
range->setProperty("Bold", isBold);
}
void QExcel::setCellFontBold(const QString &cell, bool isBold) //根据字母编号设置单元格字体是否加粗
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range = range->querySubObject("Font");
range->setProperty("Bold", isBold);
}
void QExcel::setCellFontSize(int row, int column, int size) //根据行列编号设置单元格字体大小
{
QString cell;
cell.append(QChar(column - 1 + 'A'));
cell.append(QString::number(row));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range = range->querySubObject("Font");
range->setProperty("Size", size);
}
void QExcel::setCellFontSize(const QString &cell, int size) //根据字母编号设置单元格字体大小
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range = range->querySubObject("Font");
range->setProperty("Size", size);
}
void QExcel::setCellTextCenter(int row, int column) //根据行列编号设置单元格内容居中
{
QString cell;
cell.append(QChar(column - 1 + 'A'));
cell.append(QString::number(row));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("HorizontalAlignment", -4108);//xlCenter
}
void QExcel::setCellTextCenter(const QString &cell) //根据字母编号设置单元格内容居中
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("HorizontalAlignment", -4108);//xlCenter
}
void QExcel::setCellTextWrap(int row, int column, bool isWrap) //根据行列编号设置单元格内容是否换行
{
QString cell;
cell.append(QChar(column - 1 + 'A'));
cell.append(QString::number(row));
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("WrapText", isWrap);
}
void QExcel::setCellTextWrap(const QString &cell, bool isWrap) //根据字母编号设置单元格内容是否换行
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
range->setProperty("WrapText", isWrap);
}
void QExcel::setCellBackground(int row, int column, QColor color) //根据行列编号设置单元格背景颜色
{
QAxObject *range = sheet->querySubObject("Cells(int,int)", row, column);
QAxObject *interior =range->querySubObject("Interior");
interior->setProperty("Color", color);
}
void QExcel::setCellBackground(const QString& cell, QColor color) //根据字母编号设置单元格背景颜色
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
QAxObject *interior =range->querySubObject("Interior");
interior->setProperty("Color", color);
}
void QExcel::setCellFontColor(int row, int column, QColor color) //根据行列编号设置单元格字体颜色
{
QAxObject *range = sheet->querySubObject("Cells(int,int)", row, column);
QAxObject *font =range->querySubObject("Font");
font->setProperty("Color", color);
}
void QExcel::setCellFontColor(const QString& cell, QColor color) //根据字母编号设置单元格字体颜色
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
QAxObject *font =range->querySubObject("Font");
font->setProperty("Color", color);
}
void QExcel::setCellBorderColor(int row, int column, QColor color) //根据行列编号设置单元格边框颜色
{
QAxObject *range = sheet->querySubObject("Cells(int,int)", row, column);
QAxObject* border = range->querySubObject("Borders");
border->setProperty("Color", color);
}
void QExcel::setCellBorderColor(const QString& cell, QColor color) //根据字母编号设置单元格边框颜色
{
QAxObject *range = sheet->querySubObject("Range(const QString&)", cell);
QAxObject* border = range->querySubObject("Borders");
border->setProperty("Color", color);
}
void QExcel::getUsedRange(int *topLeftRow, int *topLeftColumn, int *bottomRightRow, int *bottomRightColumn) //取得工作表已使用范围
{
QAxObject *usedRange = sheet->querySubObject("UsedRange");
*topLeftRow = usedRange->property("Row").toInt();
*topLeftColumn = usedRange->property("Column").toInt();
QAxObject *rows = usedRange->querySubObject("Rows");
*bottomRightRow = *topLeftRow + rows->property("Count").toInt() - 1;
QAxObject *columns = usedRange->querySubObject("Columns");
*bottomRightColumn = *topLeftColumn + columns->property("Count").toInt() - 1;
}
void QExcel::setColumnWidth(int column, int width) //设置列宽
{
QString columnName;
columnName.append(QChar(column - 1 + 'A'));
columnName.append(":");
columnName.append(QChar(column - 1 + 'A'));
QAxObject * col = sheet->querySubObject("Columns(const QString&)", columnName);
col->setProperty("ColumnWidth", width);
}
void QExcel::setRowHeight(int row, int height) //设置行高
{
QString rowsName;
rowsName.append(QString::number(row));
rowsName.append(":");
rowsName.append(QString::number(row));
QAxObject * r = sheet->querySubObject("Rows(const QString &)", rowsName);
r->setProperty("RowHeight", height);
}
void QExcel::setAutoFitRow(int row) //设置自适应行高
{
QString rowsName;
rowsName.append(QString::number(row));
rowsName.append(":");
rowsName.append(QString::number(row));
QAxObject * rows = sheet->querySubObject("Rows(const QString &)", rowsName);
rows->dynamicCall("AutoFit()");
}
void QExcel::mergeSerialSameCellsInAColumn(int column, int topRow) //合并一列中相同连续的单元格
{
int a,b,c,rowsCount;
getUsedRange(&a, &b, &rowsCount, &c);
int aMergeStart = topRow, aMergeEnd = topRow + 1;
QString value;
while(aMergeEnd <= rowsCount)
{
value = getCellValue(aMergeStart, column).toString();
while(value == getCellValue(aMergeEnd, column).toString())
{
clearCell(aMergeEnd, column);
aMergeEnd++;
}
aMergeEnd--;
mergeCells(aMergeStart, column, aMergeEnd, column);
aMergeStart = aMergeEnd + 1;
aMergeEnd = aMergeStart + 1;
}
}
int QExcel::getUsedRowsCount() //获取总行数
{
QAxObject *usedRange = sheet->querySubObject("UsedRange");
int topRow = usedRange->property("Row").toInt();
QAxObject *rows = usedRange->querySubObject("Rows");
int bottomRow = topRow + rows->property("Count").toInt() - 1;
return bottomRow;
}
int QExcel::getUsedColumnCount() //获取总列数
{
QAxObject *usedRange = sheet->querySubObject("UsedRange");
int leftColumn = usedRange->property("Column").toInt();
QAxObject *columns = usedRange->querySubObject("Columns");
int rightColumn = leftColumn + columns->property("Count").toInt() - 1;
return rightColumn;
}
QAxObject *QExcel::getWorkBooks()
{
return workBooks;
}
QAxObject *QExcel::getWorkBook()
{
return workBook;
}
QAxObject *QExcel::getWorkSheets()
{
return sheets;
}
QAxObject *QExcel::getWorkSheet()
{
return sheet;
}
