#include "ChangeLogDialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

ChangeLogDialog::ChangeLogDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setupUI();
    loadChangeLogs();
}

ChangeLogDialog::~ChangeLogDialog() = default;

void ChangeLogDialog::setupUI() {
    setWindowTitle("修改紀錄 / 版本歷史");
    resize(900, 600);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 說明標籤
    QLabel* infoLabel = new QLabel("顯示最近 100 筆事件修改紀錄");
    infoLabel->setStyleSheet("color: #666; padding: 5px;");
    mainLayout->addWidget(infoLabel);
    
    // 建立表格
    m_logTable = new QTableWidget(this);
    m_logTable->setColumnCount(6);
    m_logTable->setHorizontalHeaderLabels({
        "時間", "類型", "操作", "實體ID", "變更欄位", "詳細資訊"
    });
    
    // 設定表格屬性
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->horizontalHeader()->setStretchLastSection(true);
    m_logTable->verticalHeader()->setVisible(false);
    
    // 設定欄寬
    m_logTable->setColumnWidth(0, 150);  // 時間
    m_logTable->setColumnWidth(1, 80);   // 類型
    m_logTable->setColumnWidth(2, 80);   // 操作
    m_logTable->setColumnWidth(3, 150);  // 實體ID
    m_logTable->setColumnWidth(4, 150);  // 變更欄位
    
    connect(m_logTable, &QTableWidget::cellClicked,
            this, &ChangeLogDialog::onLogSelected);
    
    mainLayout->addWidget(m_logTable);
    
    // 按鈕區
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_refreshBtn = new QPushButton("重新整理");
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &ChangeLogDialog::onRefreshClicked);
    buttonLayout->addWidget(m_refreshBtn);
    
    m_closeBtn = new QPushButton("關閉");
    connect(m_closeBtn, &QPushButton::clicked,
            this, &QDialog::accept);
    buttonLayout->addWidget(m_closeBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void ChangeLogDialog::loadChangeLogs() {
    m_logTable->setRowCount(0);
    
    QList<ChangeLog> logs = m_dbManager->loadChangeLogs(100);
    
    for (const ChangeLog& log : logs) {
        int row = m_logTable->rowCount();
        m_logTable->insertRow(row);
        
        // 時間
        QTableWidgetItem* timeItem = new QTableWidgetItem(
            log.timestamp.toString("yyyy-MM-dd hh:mm:ss")
        );
        m_logTable->setItem(row, 0, timeItem);
        
        // 類型
        QTableWidgetItem* typeItem = new QTableWidgetItem(log.entityType);
        m_logTable->setItem(row, 1, typeItem);
        
        // 操作
        QString actionText;
        if (log.action == "created") {
            actionText = "新增";
        } else if (log.action == "updated") {
            actionText = "更新";
        } else if (log.action == "deleted") {
            actionText = "刪除";
        } else {
            actionText = log.action;
        }
        QTableWidgetItem* actionItem = new QTableWidgetItem(actionText);
        
        // 根據操作類型設定顏色
        if (log.action == "created") {
            actionItem->setForeground(QColor("#4CAF50"));  // 綠色
        } else if (log.action == "updated") {
            actionItem->setForeground(QColor("#2196F3"));  // 藍色
        } else if (log.action == "deleted") {
            actionItem->setForeground(QColor("#F44336"));  // 紅色
        }
        
        m_logTable->setItem(row, 2, actionItem);
        
        // 實體ID
        QTableWidgetItem* idItem = new QTableWidgetItem(
            log.entityId.length() > 20 ? log.entityId.left(17) + "..." : log.entityId
        );
        m_logTable->setItem(row, 3, idItem);
        
        // 變更欄位
        QTableWidgetItem* fieldsItem = new QTableWidgetItem(log.changedFields);
        m_logTable->setItem(row, 4, fieldsItem);
        
        // 詳細資訊 (簡化顯示)
        QString detailText;
        if (!log.newValues.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(log.newValues.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("title")) {
                    detailText = obj["title"].toString();
                }
            }
        }
        QTableWidgetItem* detailItem = new QTableWidgetItem(detailText);
        m_logTable->setItem(row, 5, detailItem);
        
        // 儲存完整的 log 資料到第一個欄位
        timeItem->setData(Qt::UserRole, QVariant::fromValue(log));
    }
}

void ChangeLogDialog::onRefreshClicked() {
    loadChangeLogs();
}

void ChangeLogDialog::onLogSelected(int row, int column) {
    Q_UNUSED(column);
    
    QTableWidgetItem* item = m_logTable->item(row, 0);
    if (!item) return;
    
    ChangeLog log = item->data(Qt::UserRole).value<ChangeLog>();
    showLogDetails(log);
}

void ChangeLogDialog::showLogDetails(const ChangeLog& log) {
    QString details;
    
    details += QString("操作類型: %1\n").arg(
        log.action == "created" ? "新增" : 
        log.action == "updated" ? "更新" : 
        log.action == "deleted" ? "刪除" : log.action
    );
    details += QString("實體類型: %1\n").arg(log.entityType);
    details += QString("實體ID: %1\n").arg(log.entityId);
    details += QString("時間: %1\n").arg(log.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    details += QString("變更欄位: %1\n\n").arg(log.changedFields);
    
    if (!log.oldValues.isEmpty()) {
        details += "變更前的值:\n";
        QJsonDocument doc = QJsonDocument::fromJson(log.oldValues.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                details += QString("  %1: %2\n").arg(it.key()).arg(it.value().toString());
            }
        }
        details += "\n";
    }
    
    if (!log.newValues.isEmpty()) {
        details += "變更後的值:\n";
        QJsonDocument doc = QJsonDocument::fromJson(log.newValues.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                details += QString("  %1: %2\n").arg(it.key()).arg(it.value().toString());
            }
        }
    }
    
    QMessageBox::information(this, "修改紀錄詳情", details);
}
