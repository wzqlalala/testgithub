#include "MainWindow.h"
#include "ui_MainWindow.h"

//Qt
#include <QFileDialog>
#include <QFile>
#include <QThreadPool>
#include <QMimeData>
#include <QScrollBar>
#include <QClipboard>
#include <QTextStream>
#include <qDebug>

#include "crc32.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "keccak.h"
#include "sha3.h"
#include "hash.h"
#include "TaskThread.h"

#include <iostream>
#include <fstream>
#include "FileThread.h"
#include "BasicState.h"

//#include "crc32parallel.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	_ui = new Ui::MainWindow();
	_ui->setupUi(this);

	threadCount = 0;

	//init_table();

	this->setAcceptDrops(true); //必须设置，不然无法接收拖放事件
	QScrollBar *a = _ui->tableWidget->horizontalScrollBar(); //获取到tablewidget的滚动条
	a->setSingleStep(5); //设置单步，值越小，下滑越慢

	//_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置表格不可修改
	_ui->tableWidget->setColumnCount(4);
	QStringList horizontal,vertical;
	horizontal << "filename"<<"CRC32" << "MD5" << "SHA1";
	_ui->tableWidget->setHorizontalHeaderLabels(horizontal);
	_ui->tableWidget->verticalHeader()->hide();//将默认序号隐藏

	_ui->tableWidget->setWordWrap(true);//设置自动换行

	//表头自适应
	_ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	//点击表时不对表头行光亮（获取焦点）
	_ui->tableWidget->horizontalHeader()->setHighlightSections(false);

	//设置表头字体加粗
	QFont font = _ui->tableWidget->horizontalHeader()->font();
	font.setBold(true);
	_ui->tableWidget->horizontalHeader()->setFont(font);

	_ui->tableWidget->installEventFilter(this);

	connect(_ui->checkBox, &QCheckBox::clicked, this, &MainWindow::slot_checkBox);
	connect(_ui->checkBox_2, &QCheckBox::clicked, this, &MainWindow::slot_checkBox);
	connect(_ui->checkBox_3, &QCheckBox::clicked, this, &MainWindow::slot_checkBox);
	connect(_ui->checkBox_4, &QCheckBox::clicked, this, &MainWindow::slot_checkBox);
	connect(_ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::slot_clear);
	connect(_ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::slot_exportcsv);
	connect(_ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::slot_copy);
	connect(_ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::slot_browser);
	connect(_ui->pushButton, &QPushButton::clicked, this, &MainWindow::slot_calculate);
	connect(_ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &MainWindow::slot_doubleClickedTable);
	connect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
	//connect(_ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), _ui->tableWidget, SLOT(resizeRowsToContents()));


	//lamda表达式
	//connect(m_pBtn, static_cast<void (MyButton::*)(bool)>(&MyButton::sigClicked), this, [=](bool check) {
	//	//do something
	//});
}

MainWindow::~MainWindow()
{
	if (_ui != nullptr)
	{
		delete _ui;
		_ui = nullptr;
	}
}

void MainWindow::slot_checkBox()
{
	QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
	if(checkBox == nullptr)
	{
		return;
	}
	bool isChecked = checkBox->isChecked();
	if(checkBox == _ui->checkBox)
	{
		
	}
	else if (checkBox == _ui->checkBox_2)
	{

	}
	else if (checkBox == _ui->checkBox_3)
	{

	}
	else if (checkBox == _ui->checkBox_4)
	{
		Parallel = isChecked;
	}
}

void MainWindow::slot_exportcsv()
{

	QString fileName = QFileDialog::getSaveFileName(this, tr("Excel file"), "", tr("Files (*.csv)"));
	if (fileName.isNull())
	{
		return;
	}
	//打开.csv文件
	QFile file(fileName);
	file.setFileName(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		//qDebug() << "open file fail";
		return;
	}
	QTextStream out(&file);             //创建一个文本流，向保存文件中写入文本

	int column = _ui->tableWidget->columnCount();
	int row = _ui->tableWidget->rowCount();

	//写表头
	for (int i = 0; i < column; i++)
	{
		QTableWidgetItem *item = _ui->tableWidget->horizontalHeaderItem(i);
		if (item == nullptr)
		{
			continue;
		}
		out << QString("%1\t").arg(item->text());
		if (i == (column - 1))
		{
			out << "\n";
		}
		else
		{
			out << ",";
		}
	}
	for(int i = 0;i<row;++i)
	{
		for (int j = 0;j<column;++j)
		{
			QTableWidgetItem *item = _ui->tableWidget->item(i, j);
			if (item == nullptr)
			{
				continue;
			}
			out << QString("%1\t").arg(item->text());
			if (j == (column - 1))
			{
				out << "\n";
			}
			else
			{
				out << ",";
			}
		}
	}

	file.close();

}

void MainWindow::slot_copy()
{
	QString copytext;
	int column = _ui->tableWidget->columnCount();
	int row = _ui->tableWidget->rowCount();

	//写表头
	for (int i = 0; i < column; i++)
	{
		QTableWidgetItem *item = _ui->tableWidget->horizontalHeaderItem(i);
		if (item == nullptr)
		{
			continue;
		}
		copytext.append(item->text());
		if (i == (column - 1))
		{
			copytext.append("\n");
		}
		else
		{
			copytext.append(",");
		}
	}
	for (int i = 0; i < row; ++i)
	{
		for (int j = 0; j < column; ++j)
		{
			QTableWidgetItem *item = _ui->tableWidget->item(i, j);
			if (item == nullptr)
			{
				continue;
			}
			copytext.append(QString("\"%1\"").arg(item->text()));
			if (j == (column - 1))
			{
				copytext.append("\n");
			}
			else
			{
				copytext.append(",");
			}
		}
	}

	QApplication::clipboard()->setText(copytext);
}

void MainWindow::slot_browser()
{
	QStringList filenames = QFileDialog::getOpenFileNames();
	fileNames.append(filenames);

	okFileNames();
}

void MainWindow::slot_clear()
{
	int column = _ui->tableWidget->columnCount();
	int row = _ui->tableWidget->rowCount();
	for (int i = 0; i < row; ++i)
	{
		for (int j = 0; j < column; ++j)
		{
			QTableWidgetItem *item = _ui->tableWidget->item(i, j);
			if (item == nullptr)
			{
				continue;
			}
			delete item;
		}
	}
	_ui->tableWidget->clearContents();
	_ui->tableWidget->setRowCount(0);
	_ui->progressBar->setValue(0);

	_fileName_tableItem.clear();
	fileNames.clear();

}

void MainWindow::slot_calculate()
{
	if (_ui->tabWidget->currentIndex() == 0)//字符串
	{
		QString t = _ui->plainTextEdit->toPlainText();
		const std::string &text = t.toStdString();//乱码嘞
		QThreadPool threadPool;
		QList<HashType> _hashTypes;
		if (_ui->checkBox->isChecked())
		{
			_hashTypes.append(HashType::CRC32);
		}
		if (_ui->checkBox_2->isChecked())
		{
			_hashTypes.append(HashType::MD5);
		}
		if (_ui->checkBox_3->isChecked())
		{
			_hashTypes.append(HashType::SHA1);
		}
		for (HashType hashType : _hashTypes)
		{
			Hash *hash;
			switch (hashType)
			{
			case HashType::CRC32:hash = new CRC32; break;
			case HashType::MD5:hash = new MD5; break;
			case HashType::SHA1:hash = new SHA1; break;
			default:hash = new CRC32; break;
			}
			//CRC32 c;
			//c(text);
			//(*hash)(text);
			_ui->plainTextEdit->appendPlainText(hash->getHashTypeStr() + ":" + QString::fromStdString((*hash)(text)));
		}

		// 计算CRC32值
		//const char *data = "Hello, world!";
		//const size_t len = strlen(data);
		//const uint32_t crc = crc32_parallel(data, len);
		//_ui->plainTextEdit->appendPlainText("CRC32:" + QString("%1").arg(crc));

	}
	else//文件
	{
		time.start();
		if (fileNames.empty())
		{
			return;
		}
		threadCount = 0;
		disconnect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
		for (QString filename : fileNames)
		{
			//开始校验
			QList<HashType> hashTypes;
			if (_ui->checkBox->isChecked())
			{
				hashTypes.append(HashType::CRC32);
				threadCount++;
			}
			if (_ui->checkBox_2->isChecked())
			{
				hashTypes.append(HashType::MD5);
				threadCount++;
			}
			if (_ui->checkBox_3->isChecked())
			{
				hashTypes.append(HashType::SHA1);
				threadCount++;
			}
			FileThread *thread = new FileThread(filename, hashTypes);
			connect(thread, &FileThread::finishedCalculate, this, &MainWindow::slot_finishedCalculate);
			QThreadPool::globalInstance()->start(thread);
		}
		_ui->progressBar->setRange(0, threadCount);
		_ui->progressBar->setValue(0);
		fileNames.clear();
	}
}

void MainWindow::slot_finishedCalculate(Hash *hash)
{
	FileThread *thread = qobject_cast<FileThread*>(sender());

	if(thread!=nullptr)
	{
		threadCount--;
		int row = _fileName_tableItem[thread->getFileName()]->row();
		QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(hash->getHash()));
		//item->setFlags(Qt::ItemIsEditable);
		_ui->tableWidget->setItem(row, int(hash->getHashType()) + 1, item);
		_ui->tableWidget->resizeRowToContents(row);
		_ui->progressBar->setValue(_ui->progressBar->maximum() - threadCount);
		delete hash;
	}

	if (threadCount == 0)
	{
		connect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
		qDebug() << time.elapsed();
		//_ui->tableWidget->resizeColumnsToContents();//根据内容调整表格
		//_ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//拉长最后一列
		//_ui->tableWidget->resizeRowsToContents();
	}
}

void MainWindow::slot_doubleClickedTable(QTableWidgetItem* item)
{
	if (item == nullptr)
	{
		return;
	}
	_currentString = item->text();
}

void MainWindow::slot_tableItemChanged(QTableWidgetItem* item)
{
	if (item == nullptr)
	{
		return;
	}
	disconnect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
	item->setText(_currentString);
	connect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) //拖动文件到窗口释放文件，触发
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls())
	{
		QList<QUrl> urls = mimeData->urls();
		for (QUrl url : urls)
		{
			QString fileName = url.toLocalFile();
			fileNames.append(fileName);
		}

		okFileNames();
	}
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
	//if (obj == _ui->tableWidget)
	//{
	//	if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
	//	{
	//		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event); //使用Qt提供的QKeyEvent接口

	//		if (keyEvent->modifiers() == Qt::ControlModifier)
	//		{ // 是否按下Ctrl键
	//			if (keyEvent->key() == Qt::Key_C)              // 是否按下C键
	//			{
	//				QTableWidgetItem *item = _ui->tableWidget->currentItem();
	//				if (item != nullptr)
	//				{
	//					QClipboard *clip = QApplication::clipboard();
	//					clip->setText(item->text());
	//				}
	//			}
	//		}
	//		return true;
	//	}
	//	return QMainWindow::eventFilter(obj, event);
	//}
	//else 
	//{
		return QMainWindow::eventFilter(obj, event);
	//}
}

void MainWindow::okFileNames()
{
	disconnect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
	int startRowCount = _ui->tableWidget->rowCount();
	_ui->tableWidget->setRowCount(_ui->tableWidget->rowCount() + fileNames.size());
	_fileName_tableItem.clear();
	for (QString filename : fileNames)
	{
		//设置表格
		QTableWidgetItem *item = new QTableWidgetItem(filename);
		_ui->tableWidget->setItem(startRowCount, 0, item);
		_fileName_tableItem.insert(filename, item);
		startRowCount++;
	}
	connect(_ui->tableWidget, &QTableWidget::itemChanged, this, &MainWindow::slot_tableItemChanged);
}
