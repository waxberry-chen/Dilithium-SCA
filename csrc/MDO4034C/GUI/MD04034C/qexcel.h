#ifndef QEXCEL_H
#define QEXCEL_H
#include <QString>
#include <QColor>
#include <QVariant>
#include <QAxObject>
#include <QFile>
#include <QStringList>
#include <QDebug>
class QAxObject;
class QExcel : public QObject
{
public:
QExcel(QObject *parent = nullptr);
~QExcel();
public:
QAxObject * getWorkBooks();
QAxObject * getWorkBook();
QAxObject * getWorkSheets();
QAxObject * getWorkSheet();
public:
/***************************************************************************/
/* 文件操作 */
/**************************************************************************/
void open(QString FileName); //打开文件
void createFile(QString FileName); //创建文件
void save(); //保存Excel文件
void quit(); //退出Excel
/**************************************************************************/
/* 工作表操作 */
/*************************************************************************/
void selectSheet(const QString& sheetName); //根据名字选择工作表
void selectSheet(int sheetIndex); //根据下标索引选择工作表，下标从1开始
void deleteSheet(const QString& sheetName); //根据名字删除工作表
void deleteSheet(int sheetIndex); //根据下标索引删除工作表，下标从1开始
void insertSheet(QString sheetName); //新建工作表
int getSheetsCount(); //获取工作表数量
QString getSheetName(); //获取当前活跃工作表的名字
QString getSheetName(int sheetIndex); //根据下标索引获取工作表名字
/***************************************************************************/
/* 单元格操作 */
/**************************************************************************/
void mergeCells(int topLeftRow, int topLeftColumn, int bottomRightRow, int bottomRightColumn); //根据行列编号合并单元格
void mergeCells(const QString& range); //根据字母编号合并单元格，例如(A5:C7)
void setCellString(int row, int column, const QString& value); //根据行列编号设置单元格数据，例如(1,1,"xxx")
void setCellString(const QString& cell, const QString& value); //根据字母编号设置单元格数据，例如("A5","xxx")

QString to26AlphabetString(int data);
void convertToColName(int data, QString &res);
void htz_Write_row(int16_t Buffer[], int column, int Length); //根据行列编号设置单元格数据
void htz_Read_col(int16_t Buffer[], int row, int Length,int coef_index);
void htz_Read_col2(int16_t Buffer[], int col, int Length,int coef_index);

bool setRowData(int row,int colum_start,int column_end,QVariant vData); //批量写入一行数据
QString getRange_fromColumn(int column); //根据列值计算出字母值
QVariant getCellValue(int row, int column); //根据行列编号获取单元格数值
QVariant getCellValue(const QString& cell); //根据字母编号获取单元格数值
void clearCell(int row, int column); //根据行列编号清空单元格
void clearCell(const QString& cell); //根据字母编号清空单元格
void setCellFontBold(int row, int column, bool isBold); //根据行列编号设置单元格字体是否加粗
void setCellFontBold(const QString& cell, bool isBold); //根据字母编号设置单元格字体是否加粗
void setCellFontSize(int row, int column, int size); //根据行列编号设置单元格字体大小
void setCellFontSize(const QString& cell, int size); //根据字母编号设置单元格字体大小
void setCellTextCenter(int row, int column); //根据行列编号设置单元格内容居中
void setCellTextCenter(const QString& cell); //根据字母编号设置单元格内容居中
void setCellTextWrap(int row, int column, bool isWrap); //根据行列编号设置单元格内容是否换行
void setCellTextWrap(const QString& cell, bool isWrap); //根据字母编号设置单元格内容是否换行
void setCellBackground(int row, int column, QColor color); //根据行列编号设置单元格背景色
void setCellBackground(const QString& cell, QColor color); //根据字母编号设置单元格背景色
void setCellFontColor(int row, int column, QColor color); //根据行列编号设置单元格字体颜色
void setCellFontColor(const QString& cell, QColor color); //根据字母编号设置单元格字体颜色
void setCellBorderColor(int row, int column, QColor color); //根据行列编号设置单元格边框颜色
void setCellBorderColor(const QString& cell, QColor color); //根据字母编号设置单元格边框颜色
void getUsedRange(int *topLeftRow, int *topLeftColumn, int *bottomRightRow, int *bottomRightColumn); //取得工作表已使用范围
void setColumnWidth(int column, int width); //设置列宽
void setRowHeight(int row, int height); //设置行高
void setAutoFitRow(int row); //设置自适应行高
void mergeSerialSameCellsInAColumn(int column, int topRow); //合并一列中相同连续的单元格
int getUsedRowsCount(); //获取总行数
int getUsedColumnCount(); //获取总列数
private:
QAxObject * excel; //Excel指针
QAxObject * workBooks; //工作簿集合
QAxObject * workBook; //工作簿
QAxObject * sheets; //工作表集合
QAxObject * sheet; //工作表
};
#endif
