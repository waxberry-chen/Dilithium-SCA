<span style="white-space:pre">	</span>QString strContent;
	QAxObject *range;
	QList<QVariant> mLstData;
	QList<QString> mLstCurData;
	int nColumns = 0;
	QString strMergeCell;
	int nIndex = 0, nTotal = 0;		// 记录数据个数
	QStringList strListParaName;
	QMap<int,PidInfoStruct> mapPidInfo;
 
	//建立Excel对象
	HRESULT r = OleInitialize(0);<span style="white-space:pre">		</span>//由于我把导出数据的操作放在了一个新线程中，所以在调用activeX之前要初始化com组件
	if (r != S_OK && r != S_FALSE) {
		qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
	}
	QAxObject *excel 