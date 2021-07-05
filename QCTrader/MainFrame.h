#pragma once

#include <QMainFrame>
#include "ui_MainFrame.h"

class MainFrame : public QMainFrame
{
	Q_OBJECT

public:
	MainFrame(QWidget *parent = Q_NULLPTR);
	~MainFrame();

private:
	Ui::MainFrame ui;
};
