#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "storage/DatabaseManager.h"

// 修改紀錄檢視對話框
class ChangeLogDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit ChangeLogDialog(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~ChangeLogDialog() override;
    
private slots:
    void onRefreshClicked();
    void onLogSelected(int row, int column);
    
private:
    void setupUI();
    void loadChangeLogs();
    void showLogDetails(const ChangeLog& log);
    
    DatabaseManager* m_dbManager;
    QTableWidget* m_logTable;
    QPushButton* m_refreshBtn;
    QPushButton* m_closeBtn;
};
